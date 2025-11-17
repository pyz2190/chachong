/*
 * ������AI������ȫͨ�ð汾 - ����������ƽ̨��
 * ʵ�ֻ���Alpha-Beta��֦�������㷨
 * ���ߣ�����ԭ�����ʵ��
 * ���ڣ�2025-11-17
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

 /* ==================== �������� ==================== */

#define BOARD_SIZE 12
#define EMPTY 0
#define BLACK 1
#define WHITE 2

// �������֣�ԭ����ƣ�
#define SCORE_FIVE       100000
#define SCORE_LIVE_FOUR   20000
#define SCORE_RUSH_FOUR    5000
#define SCORE_LIVE_THREE   3000
#define SCORE_SLEEP_THREE   800
#define SCORE_LIVE_TWO      200
#define SCORE_SLEEP_TWO      50
#define SCORE_SINGLE         10

// ��������
#define MAX_CANDIDATES 15
#define MIN_SCORE_THRESHOLD 5  // ���������ֵ�����ڴ�ֵ�ĺ�ѡλ�ò�����
#define INF 999999999

// ʱ����ƣ����룩
#define TURN_TIME_LIMIT 1900
#define TOTAL_TIME_LIMIT 88000

/* ==================== ���ݽṹ���� ==================== */

// ���̽ṹ
typedef struct {
    int grid[BOARD_SIZE][BOARD_SIZE];
    int move_count;
} Board;

// �߷��ṹ
typedef struct {
    int row;
    int col;
    int priority;
} Move;

// ʱ�������
typedef struct {
    long long total_time_used;
    long long current_start;
    int turn_time_limit;
    int total_time_limit;
} TimeManager;

// ����������
typedef struct {
    Board* board;
    TimeManager* timer;
    int my_side;
    int enemy_side;
} SearchContext;

/* ==================== ȫ�ֱ��� ==================== */

Board game_board;
TimeManager time_manager;
int my_color = 0;
int enemy_color = 0;

// �ĸ�������ˮƽ����ֱ�������Խ��ߣ�
const int dir_x[4] = { 1, 0, 1, 1 };
const int dir_y[4] = { 0, 1, 1, -1 };

/* ==================== ʱ��������� ==================== */

// ��ȡ��ǰʱ�䣨���룩- ʹ�ñ�׼C���ͨ��ʵ��
long long get_time_ms() {
    // ʹ�ñ�׼C���clock()����
    // ע�⣺clock()���ص���CPUʱ�䣬����ǽ��ʱ�䣬���������Ӧ���㹻��
    return (long long)(clock() * 1000.0 / CLOCKS_PER_SEC);
}

// ��ʼ��ʱ�������
void init_timer(TimeManager* tm) {
    tm->total_time_used = 0;
    tm->current_start = 0;
    tm->turn_time_limit = TURN_TIME_LIMIT;
    tm->total_time_limit = TOTAL_TIME_LIMIT;
}

// ��ʼһ���غ�
void start_turn(TimeManager* tm) {
    tm->current_start = get_time_ms();
}

// ����һ���غ�
void end_turn(TimeManager* tm) {
    long long elapsed = get_time_ms() - tm->current_start;
    tm->total_time_used += elapsed;
}

// ����Ƿ�ʱ
int is_timeout(TimeManager* tm) {
    long long current = get_time_ms();
    long long turn_elapsed = current - tm->current_start;

    if (turn_elapsed >= tm->turn_time_limit) {
        return 1;
    }

    if (tm->total_time_used + turn_elapsed >= tm->total_time_limit) {
        return 1;
    }

    return 0;
}

/* ==================== ���̹������� ==================== */

// ��ʼ������
void init_board(Board* board) {
    int i, j;
    for (i = 0; i < BOARD_SIZE; i++) {
        for (j = 0; j < BOARD_SIZE; j++) {
            board->grid[i][j] = EMPTY;
        }
    }
    board->move_count = 0;
}

// �߽���
int in_bounds(int row, int col) {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

// ����Ƿ�Ϊ��λ
int is_empty(Board* board, int row, int col) {
    if (!in_bounds(row, col)) return 0;
    return board->grid[row][col] == EMPTY;
}

// ����
void place_stone(Board* board, int row, int col, int color) {
    if (in_bounds(row, col) && board->grid[row][col] == EMPTY) {
        board->grid[row][col] = color;
        board->move_count++;
    }
}

// ����
void undo_stone(Board* board, int row, int col) {
    if (in_bounds(row, col) && board->grid[row][col] != EMPTY) {
        board->grid[row][col] = EMPTY;
        board->move_count--;
    }
}

// 辅助函数：沿某个方向计数连续的棋子
static int count_direction(Board* board, int row, int col, int dx, int dy, int color) {
    int count = 0;
    int i;
    for (i = 1; i < 5; i++) {
        int nr = row + dx * i;
        int nc = col + dy * i;
        if (in_bounds(nr, nc) && board->grid[nr][nc] == color) {
            count++;
        } else {
            break;
        }
    }
    return count;
}

// ����ĳ�㿪ʼ�Ƿ��γ�����
int check_five(Board* board, int row, int col, int color) {
    int d;

    if (!in_bounds(row, col) || board->grid[row][col] != color) {
        return 0;
    }

    // ����ĸ�����
    for (d = 0; d < 4; d++) {
        int count = 1;  // 自己这个点
        count += count_direction(board, row, col, dir_x[d], dir_y[d], color);    // 正向
        count += count_direction(board, row, col, -dir_x[d], -dir_y[d], color);  // 反向

        if (count >= 5) return 1;
    }

    return 0;
}

/* ==================== �������� ==================== */

// �ж����͵÷�
int get_pattern_score(int count, int left_open, int right_open) {
    int open_ends = left_open + right_open;

    if (count >= 5) {
        return SCORE_FIVE;
    }

    if (count == 4) {
        if (open_ends == 2) return SCORE_LIVE_FOUR;
        if (open_ends == 1) return SCORE_RUSH_FOUR;
        return 0;
    }

    if (count == 3) {
        if (open_ends == 2) return SCORE_LIVE_THREE;
        if (open_ends == 1) return SCORE_SLEEP_THREE;
        return 0;
    }

    if (count == 2) {
        if (open_ends == 2) return SCORE_LIVE_TWO;
        if (open_ends == 1) return SCORE_SLEEP_TWO;
        return 0;
    }

    return SCORE_SINGLE;
}

// ���ٵ������������ں�ѡ����
int evaluate_point(Board* board, int row, int col, int color) {
    int total = 0;
    int d, i, count, left_open, right_open;
    int nr, nc;

    // �ĸ�����
    for (d = 0; d < 4; d++) {
        count = 1;  // ������ǰ��
        left_open = 0;
        right_open = 0;

        // ������ͳ��
        for (i = 1; i < 6; i++) {
            nr = row + dir_x[d] * i;
            nc = col + dir_y[d] * i;
            if (!in_bounds(nr, nc)) break;
            if (board->grid[nr][nc] == color) {
                count++;
            }
            else if (board->grid[nr][nc] == EMPTY) {
                right_open = 1;
                break;
            }
            else {
                break;
            }
        }

        // ������ͳ��
        for (i = 1; i < 6; i++) {
            nr = row - dir_x[d] * i;
            nc = col - dir_y[d] * i;
            if (!in_bounds(nr, nc)) break;
            if (board->grid[nr][nc] == color) {
                count++;
            }
            else if (board->grid[nr][nc] == EMPTY) {
                left_open = 1;
                break;
            }
            else {
                break;
            }
        }

        total += get_pattern_score(count, left_open, right_open);
    }

    return total;
}

// ȫ�־�������
int evaluate_board(Board* board, int my_side, int enemy_side) {
    int my_score = 0;
    int enemy_score = 0;
    int r, c;

    for (r = 0; r < BOARD_SIZE; r++) {
        for (c = 0; c < BOARD_SIZE; c++) {
            if (board->grid[r][c] == my_side) {
                // ��ʱ��������������
                int old = board->grid[r][c];
                board->grid[r][c] = EMPTY;
                my_score += evaluate_point(board, r, c, my_side);
                board->grid[r][c] = old;
            }
            else if (board->grid[r][c] == enemy_side) {
                int old = board->grid[r][c];
                board->grid[r][c] = EMPTY;
                enemy_score += evaluate_point(board, r, c, enemy_side);
                board->grid[r][c] = old;
            }
        }
    }

    // �ǶԳ�Ȩ�أ����������ڷ���
    return my_score - (enemy_score * 85 / 100);
}

/* ==================== �������� ==================== */

// �ȽϺ���������qsort��
int compare_moves(const void* a, const void* b) {
    Move* ma = (Move*)a;
    Move* mb = (Move*)b;
    return mb->priority - ma->priority;  // ����
}

// ���ɺ�ѡ�߷�����������ɸѡ��
int generate_candidates(SearchContext* ctx, Move* candidates, int for_side) {
    Board* board = ctx->board;
    int count = 0;
    int r, c;
    int attack, defense, total_value;
    int enemy = (for_side == BLACK) ? WHITE : BLACK;

    // ���㶯̬��ֵ��������ֽ׶ε���ɸѡ��׼
    int move_count = board->move_count;
    int threshold = MIN_SCORE_THRESHOLD;

    // ���ֽ׶Σ������ɵ�ɸѡ�����������ѡλ�ã�
    if (move_count < 12) {
        threshold = 0;  // ���ֿ�������λ��
    }
    else if (move_count < 30) {
        threshold = MIN_SCORE_THRESHOLD;
    }
    else {
        // �к�֣������ֵ���۽��߼�ֵλ��
        threshold = MIN_SCORE_THRESHOLD * 2;
    }

    for (r = 0; r < BOARD_SIZE; r++) {
        for (c = 0; c < BOARD_SIZE; c++) {
            if (board->grid[r][c] != EMPTY) continue;

            // �����õ�Ĺ����ͷ��ؼ�ֵ
            attack = evaluate_point(board, r, c, for_side);
            defense = evaluate_point(board, r, c, enemy);

            // �����ۺϼ�ֵ������Ȩ�� 11:9��
            total_value = attack * 11 + defense * 9;

            // �������ֵ�����ɸѡ��ֻ�����м�ֵ��λ��
            if (total_value < threshold) continue;

            candidates[count].row = r;
            candidates[count].col = c;
            candidates[count].priority = total_value;
            count++;

            if (count >= 256) break;  // ��ֹ���
        }
        if (count >= 256) break;
    }

    // �������ȼ��Ӹߵ���
    qsort(candidates, count, sizeof(Move), compare_moves);

    // �ضϵ�MAX_CANDIDATES
    if (count > MAX_CANDIDATES) {
        count = MAX_CANDIDATES;
    }

    return count;
}

// ���һ����ʤ
int find_winning_move(SearchContext* ctx, int side, Move* result) {
    Board* board = ctx->board;
    int r, c;

    for (r = 0; r < BOARD_SIZE; r++) {
        for (c = 0; c < BOARD_SIZE; c++) {
            if (board->grid[r][c] != EMPTY) continue;

            // ��������
            place_stone(board, r, c, side);
            int win = check_five(board, r, c, side);
            undo_stone(board, r, c);

            if (win) {
                result->row = r;
                result->col = c;
                return 1;
            }
        }
    }

    return 0;
}

// ǰ������
int alpha_beta(SearchContext* ctx, int depth, int alpha, int beta, int is_max);

// ��������Ӧ�������
int get_search_depth(Board* board) {
    int move_count = board->move_count;

    if (move_count < 10) {
        return 4;  // ���֣�ǳ��
    }
    else if (move_count < 80) {
        return 6;  // �о֣���׼���
    }
    else {
        return 8;  // �о֣�����
    }
}

// Alpha-Beta����
int alpha_beta(SearchContext* ctx, int depth, int alpha, int beta, int is_max) {
    // ��ʱ���
    if (is_timeout(ctx->timer)) {
        return evaluate_board(ctx->board, ctx->my_side, ctx->enemy_side);
    }

    // ����Ҷ�ӽڵ�
    if (depth == 0) {
        return evaluate_board(ctx->board, ctx->my_side, ctx->enemy_side);
    }

    Move candidates[256];
    int num_moves;
    int current_side = is_max ? ctx->my_side : ctx->enemy_side;
    int i, score;

    // ���ɺ�ѡ
    num_moves = generate_candidates(ctx, candidates, current_side);

    if (num_moves == 0) {
        return evaluate_board(ctx->board, ctx->my_side, ctx->enemy_side);
    }

    if (is_max) {
        // ��������
        int max_eval = -INF;

        for (i = 0; i < num_moves; i++) {
            place_stone(ctx->board, candidates[i].row, candidates[i].col, ctx->my_side);

            // �������ʤ��
            if (check_five(ctx->board, candidates[i].row, candidates[i].col, ctx->my_side)) {
                score = SCORE_FIVE - depth;  // Խ��ʤ��Խ��
            }
            else {
                score = alpha_beta(ctx, depth - 1, alpha, beta, 0);
            }

            undo_stone(ctx->board, candidates[i].row, candidates[i].col);

            if (score > max_eval) max_eval = score;
            if (score > alpha) alpha = score;
            if (alpha >= beta) break;  // Beta��֦

            if (is_timeout(ctx->timer)) break;
        }

        return max_eval;
    }
    else {
        // ��������
        int min_eval = INF;

        for (i = 0; i < num_moves; i++) {
            place_stone(ctx->board, candidates[i].row, candidates[i].col, ctx->enemy_side);

            // ����������ʤ��
            if (check_five(ctx->board, candidates[i].row, candidates[i].col, ctx->enemy_side)) {
                score = -SCORE_FIVE + depth;  // ����Խ��ʤ��Խ��
            }
            else {
                score = alpha_beta(ctx, depth - 1, alpha, beta, 1);
            }

            undo_stone(ctx->board, candidates[i].row, candidates[i].col);

            if (score < min_eval) min_eval = score;
            if (score < beta) beta = score;
            if (alpha >= beta) break;  // Alpha��֦

            if (is_timeout(ctx->timer)) break;
        }

        return min_eval;
    }
}

// ��������߷�
void search_best_move(SearchContext* ctx, Move* best_move) {
    // ����1�����һ����ʤ
    if (find_winning_move(ctx, ctx->my_side, best_move)) {
        return;
    }

    // ����2�����������
    if (find_winning_move(ctx, ctx->enemy_side, best_move)) {
        return;
    }

    // ����3����������
    Move candidates[256];
    int num_moves = generate_candidates(ctx, candidates, ctx->my_side);

    if (num_moves == 0) {
        // ���ף��������ĸ���
        best_move->row = BOARD_SIZE / 2;
        best_move->col = BOARD_SIZE / 2;
        return;
    }

    // ����ѡ��һ����ѡ
    *best_move = candidates[0];

    int depth = get_search_depth(ctx->board);
    int best_score = -INF;
    int alpha = -INF;
    int beta = INF;
    int i, score;

    for (i = 0; i < num_moves; i++) {
        place_stone(ctx->board, candidates[i].row, candidates[i].col, ctx->my_side);

        if (check_five(ctx->board, candidates[i].row, candidates[i].col, ctx->my_side)) {
            score = SCORE_FIVE;
        }
        else {
            score = alpha_beta(ctx, depth - 1, alpha, beta, 0);
        }

        undo_stone(ctx->board, candidates[i].row, candidates[i].col);

        if (score > best_score) {
            best_score = score;
            *best_move = candidates[i];
        }

        if (score > alpha) alpha = score;

        if (is_timeout(ctx->timer)) {
            break;
        }
    }
}

/* ==================== �����ƺ�IO ==================== */

// ���ó�ʼ����
void place_initial_stones(Board* board) {
    place_stone(board, 5, 5, WHITE);
    place_stone(board, 6, 6, WHITE);
    place_stone(board, 5, 6, BLACK);
    place_stone(board, 6, 5, BLACK);
}

// ����STARTָ��
void handle_start(const char* cmd) {
    int field;
    sscanf(cmd, "START %d", &field);

    my_color = field;
    enemy_color = (field == 1) ? 2 : 1;

    init_board(&game_board);
    place_initial_stones(&game_board);
    init_timer(&time_manager);

    printf("OK\n");
    fflush(stdout);
}

// ����PLACEָ��
void handle_place(const char* cmd) {
    int x, y;
    sscanf(cmd, "PLACE %d %d", &x, &y);
    place_stone(&game_board, x, y, enemy_color);
}

// ����TURNָ��
void handle_turn() {
    start_turn(&time_manager);

    SearchContext ctx;
    ctx.board = &game_board;
    ctx.timer = &time_manager;
    ctx.my_side = my_color;
    ctx.enemy_side = enemy_color;

    Move best_move;
    search_best_move(&ctx, &best_move);

    place_stone(&game_board, best_move.row, best_move.col, my_color);

    printf("%d %d\n", best_move.row, best_move.col);
    fflush(stdout);

    end_turn(&time_manager);
}

// ����ENDָ��
void handle_end(const char* cmd) {
    // ��Ϸ����������Ҫ���⴦��
}

/* ==================== ������ ==================== */

int main() {
    char command[128];

    init_board(&game_board);
    init_timer(&time_manager);

    while (fgets(command, sizeof(command), stdin)) {
        command[strcspn(command, "\n")] = 0;

        if (strncmp(command, "START", 5) == 0) {
            handle_start(command);
        }
        else if (strncmp(command, "PLACE", 5) == 0) {
            handle_place(command);
        }
        else if (strncmp(command, "TURN", 4) == 0) {
            handle_turn();
        }
        else if (strncmp(command, "END", 3) == 0) {
            handle_end(command);
            break;
        }
    }

    return 0;
}