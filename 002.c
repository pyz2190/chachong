#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

/*
 * =================================================================
 * 高强度五子棋 AI (C 语言融合版)
 * =================================================================
 *

 * 1. 迭代加深 (ID) + Alpha-Beta 搜索
 * 2. Zobrist 置换表 (TT)，含 EXACT/ALPHA/BETA 标志
 * 3. Killer Moves (杀手走法)
 * 4. History Heuristic (历史启发)
 * 5. 2000ms C 语言时间管理 (clock())
 * 6. 兼容的 START/PLACE/TURN 接口
 *
 * =================================================================
 */


// ========== 基础定义 =============
#define SIZE 12
#define EMPTY 0
#define BLACK 1
#define WHITE 2

// 最大允许的理论深度（迭代加深上限）
#define MAX_PLY 14
// 单次决策时间限制（毫秒）
#define TIME_LIMIT_MS 1980 // 留 20ms 缓冲
// 邻域半径（生成候选）
#define NEIGHBOR_RADIUS 2
// 走法生成宽度限制
#define MOVE_GEN_WIDTH 40

int board[SIZE][SIZE];
int my_color = 0;
int enemy_color = 0;

// 8 方向 
int dx[8] = {1, 0, 1, 1, -1, 0, -1, -1};
int dy[8] = {0, 1, 1, -1, 0, -1, -1, 1};
// 4 主方向 (用于新评估)
int d4x[4] = {1, 0, 1, 1};
int d4y[4] = {0, 1, 1, -1};



#define SCORE_WIN 100000000
#define SCORE_FIVE 5000000     // 五连 (评估时)
#define SCORE_LIVE_FOUR 43200  // 活四
#define SCORE_RUSH_FOUR 7200   // 冲四 (Block Four)
#define SCORE_LIVE_THREE 7200  // 活三
#define SCORE_SLEEP_THREE 1200 // 眠三 (Block Three)
#define SCORE_LIVE_TWO 1200    // 活二
#define SCORE_SLEEP_TWO 200    // 眠二
#define SCORE_LIVE_ONE 20      // 眠一/单子



typedef unsigned long long uint64;
uint64 zobrist_table[SIZE][SIZE][3];
uint64 current_hash = 0ULL;

#define HASH_SIZE (1 << 20) // 1M 条目

enum { HASH_EXACT, HASH_ALPHA, HASH_BETA };

typedef struct {
    uint64 hash;
    int depth;
    int score;
    int flag;
    int best_x, best_y; // PV move
} TTEntry;

TTEntry transposition_table[HASH_SIZE];

void init_zobrist() {
    srand((unsigned)time(NULL));
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            for (int k = 0; k < 3; k++) {
                uint64 high = (uint64)rand();
                uint64 low = (uint64)rand();
                zobrist_table[i][j][k] = (high << 32) ^ low ^ (((uint64)rand()) << 16);
            }
}

void clear_tt() {
    memset(transposition_table, 0, sizeof(transposition_table));
}

static inline int tt_index(uint64 h) { return (int)(h & (HASH_SIZE - 1)); }

void store_tt(uint64 hash, int depth, int score, int flag, int bx, int by) {
    int idx = tt_index(hash);
    // 替换策略：更深的
    if (transposition_table[idx].hash == 0 || transposition_table[idx].depth <= depth) {
        transposition_table[idx].hash = hash;
        transposition_table[idx].depth = depth;
        transposition_table[idx].score = score;
        transposition_table[idx].flag = flag;
        transposition_table[idx].best_x = bx;
        transposition_table[idx].best_y = by;
    }
}

int probe_tt(uint64 hash, int depth, int alpha, int beta, int *out_score, int *out_bx, int *out_by) {
    int idx = tt_index(hash);
    TTEntry *e = &transposition_table[idx];
    if (e->hash == hash) {
        // 即使深度不够，也提取 PV 走法用于排序
        if (out_bx) *out_bx = e->best_x;
        if (out_by) *out_by = e->best_y;

        if (e->depth >= depth) {
            if (e->flag == HASH_EXACT) {
                *out_score = e->score; return 1;
            }
            if (e->flag == HASH_ALPHA && e->score <= alpha) {
                *out_score = alpha; return 1;
            }
            if (e->flag == HASH_BETA && e->score >= beta) {
                *out_score = beta; return 1;
            }
        }
    }
    return 0;
}


static inline int in_bounds(int x, int y) { return x >= 0 && x < SIZE && y >= 0 && y < SIZE; }

static inline int opp_color(int color) {
    return (color == BLACK) ? WHITE : BLACK;
}

void place_stone(int x, int y, int color) {
    if (!in_bounds(x, y)) return;
    int old = board[x][y];
    current_hash ^= zobrist_table[x][y][old];
    board[x][y] = color;
    current_hash ^= zobrist_table[x][y][color];
}

void remove_stone(int x, int y) {
    if (!in_bounds(x, y)) return;
    int old = board[x][y];
    current_hash ^= zobrist_table[x][y][old];
    board[x][y] = EMPTY;
    current_hash ^= zobrist_table[x][y][EMPTY];
}


int check_win_at(int x, int y, int color) {
    if (!in_bounds(x, y) || board[x][y] != color) return 0;
    for (int d = 0; d < 4; d++) {
        int cnt = 1;
        // 正向
        for (int i = 1; i < 5; i++) {
            int nx = x + d4x[d] * i, ny = y + d4y[d] * i;
            if (in_bounds(nx, ny) && board[nx][ny] == color) cnt++;
            else break;
        }
        // 反向
        for (int i = 1; i < 5; i++) {
            int nx = x - d4x[d] * i, ny = y - d4y[d] * i;
            if (in_bounds(nx, ny) && board[nx][ny] == color) cnt++;
            else break;
        }
        if (cnt >= 5) return 1;
    }
    return 0;
}


int check_win_move(int x, int y, int color) {
    if (!in_bounds(x, y) || board[x][y] != EMPTY) return 0;
    place_stone(x, y, color);
    int w = check_win_at(x, y, color);
    remove_stone(x, y);
    return w;
}
int line_buffer[SIZE]; // 临时行缓冲区
// ========== (新) 评估系统 (C 语言重构) =============
// ========== 优化后 的 模式评分与评估 ==========
static inline int get_pattern_score(int count, int blocks, int gap) {
    if (count >= 5) {
        return SCORE_FIVE;
    }

    if (gap == 0) { // --- 连续模式 ---
        switch (count) {
        case 4:
            if (blocks == 0) return SCORE_LIVE_FOUR; // 活四
            if (blocks == 1) return SCORE_RUSH_FOUR; // 冲四
            break;
        case 3:
            if (blocks == 0) return SCORE_LIVE_THREE; // 活三
            if (blocks == 1) return SCORE_SLEEP_THREE; // 眠三
            break;
        case 2:
            if (blocks == 0) return SCORE_LIVE_TWO; // 活二
            if (blocks == 1) return SCORE_SLEEP_TWO; // 眠二
            break;
        case 1:
            if (blocks == 0) return SCORE_LIVE_ONE; // 活一
            break;
        }
    } else { // --- 跳空模式 (如 O_OO) ---
        switch (count) {
        case 4: // O_OOO, OO_OO
            // 按冲四算
            if (blocks == 0) return SCORE_RUSH_FOUR;
            if (blocks == 1) return SCORE_RUSH_FOUR; 
            break;
        case 3: // O_OO, OO_O
            // 按眠三算
            if (blocks == 0) return SCORE_SLEEP_THREE;
            if (blocks == 1) return SCORE_SLEEP_THREE / 2;
            break;
        case 2: // O_O
            if (blocks == 0) return SCORE_LIVE_TWO / 2; // 弱活二
            break;
        }
    }
    return 0;
}

// 核心：扫描一条线
static inline int scan_line(int len, int who) {
    int sum = 0;
    int enemy = opp_color(who);
    int i = 0;
    
    while (i < len) {
        if (line_buffer[i] != who) {
            i++;
            continue;
        }

        // 找到一个 'who' 的棋子，开始统计
        int count = 0;
        int blocks = 0;
        int gap = 0;
        int j = i;

        // 检查左侧
        if (i == 0 || line_buffer[i - 1] == enemy) {
            blocks++;
        }

        // 向右扫描
        while (j < len) {
            int v = line_buffer[j];
            if (v == who) {
                count++;
            } else if (v == EMPTY) {
                // 尝试跳过一个空位
                if (gap == 0 && (j + 1) < len && line_buffer[j + 1] == who) {
                    gap = 1;
                    j++; // 跳过这个空位
                    count++;
                } else {
                    break; // 遇到第二个空位
                }
            } else if (v == enemy) {
                // blocks++; // (bug fix) 不应在这里加，应在循环外
                break;
            }
            j++;
        }
        
        // 检查右侧
        if (j == len || line_buffer[j] == enemy) {
            blocks++;
        }
        
        if (count > 0) {
            sum += get_pattern_score(count, blocks, gap);
        }
        
        i = j + 1; // 从中断处继续
    }
    return sum;
}
// 辅助：根据 count 和 open_ends 得到基础单方向分
static inline int pattern_base_score(int count, int open_ends) {
    if (count >= 5) return SCORE_FIVE;
    if (count == 4) {
        if (open_ends == 2) return SCORE_LIVE_FOUR;
        if (open_ends == 1) return SCORE_RUSH_FOUR;
    }
    if (count == 3) {
        if (open_ends == 2) return SCORE_LIVE_THREE;
        if (open_ends == 1) return SCORE_SLEEP_THREE;
    }
    if (count == 2) {
        if (open_ends == 2) return SCORE_LIVE_TWO;
        if (open_ends == 1) return SCORE_SLEEP_TWO;
    }
    if (count == 1) {
        if (open_ends == 2) return SCORE_LIVE_ONE;
    }
    return 0;
}

// 在某一方向上扫描返回 count 和 open_ends（不使用跳空合并，单方向先检测连续块与单跳）
// 注意：这个函数假设 (r,c) 还未下子，是在考虑“如果在(r,c) 下子”的情况下评估该方向
static inline void scan_direction_for_point(int r, int c, int dr, int dc, int who,
                                           int *out_count, int *out_open_ends, int *out_has_gap) {
    int enemy = opp_color(who);
    int count = 1; // 计入(r,c)
    int open_ends = 0;
    int has_gap = 0;

    // 向正方向扫描连续（允许单跳一次来视作成段的情况）
    int i = r + dr, j = c + dc;
    int gap_used = 0;
    while (in_bounds(i, j)) {
        int v = board[i][j];
        if (v == who) {
            count++; i += dr; j += dc; continue;
        } else if (v == EMPTY) {
            // 检查是否可以跳一个空位接上同色（单跳）
            int ii = i + dr, jj = j + dc;
            if (!gap_used && in_bounds(ii, jj) && board[ii][jj] == who) {
                gap_used = 1;
                has_gap = 1;
                count++; // 把跳过的子也算上
                i = ii + dr; j = jj + dc;
                continue;
            } else {
                // 这个方向的正端为开放
                open_ends++;
                break;
            }
        } else { // enemy
            break;
        }
    }
    // 若出界即为堵塞，不算开放端
    if (!in_bounds(i, j)) {
        // 不增加 open_ends
    }

    // 向反方向扫描
    i = r - dr; j = c - dc;
    gap_used = gap_used; // 保留正向使用信息（如果正向已用了跳空，不再允许反向再用）
    while (in_bounds(i, j)) {
        int v = board[i][j];
        if (v == who) {
            count++; i -= dr; j -= dc; continue;
        } else if (v == EMPTY) {
            int ii = i - dr, jj = j - dc;
            if (!gap_used && in_bounds(ii, jj) && board[ii][jj] == who) {
                gap_used = 1;
                has_gap = 1;
                count++;
                i = ii - dr; j = jj - dc;
                continue;
            } else {
                open_ends++;
                break;
            }
        } else { // enemy
            break;
        }
    }
    // 出界时不加 open_ends

    *out_count = count;
    *out_open_ends = open_ends;
    *out_has_gap = has_gap;
}


// 改良版单点启发评估：不仅按每个方向评分，还统计“威胁组合”（double live-three, multi rush-four 等）
int eval_heuristic_move(int r, int c, int who) {
    if (!in_bounds(r,c) || board[r][c] != EMPTY) return 0;

    int dir_scores[4];
    int counts[4], opens[4], gaps[4];
    int total = 0;

    int live_four_cnt = 0, rush_four_cnt = 0, live_three_cnt = 0, sleep_three_cnt = 0;
    int live_two_cnt = 0;

    for (int k = 0; k < 4; ++k) {
        int dr = d4x[k], dc = d4y[k];
        int cnt, op, gap;
        scan_direction_for_point(r, c, dr, dc, who, &cnt, &op, &gap);
        counts[k] = cnt; opens[k] = op; gaps[k] = gap;

        int dir_score = pattern_base_score(cnt, op);
        dir_scores[k] = dir_score;
        total += dir_score;

        // 分类统计 (用于组合检测)
        if (cnt >= 5) { return SCORE_FIVE; } // 立即返回五连
        if (cnt == 4) {
            if (op == 2) live_four_cnt++;
            else if (op == 1) rush_four_cnt++;
        } else if (cnt == 3) {
            if (op == 2) live_three_cnt++;
            else if (op == 1) sleep_three_cnt++;
        } else if (cnt == 2) {
            if (op == 2) live_two_cnt++;
        }
    }

    // 组合加分逻辑（关键）：
    // - 双活三 -> 极强（通常对手无法同时阻止） -> 给比单活四更高的优先级（但仍低于直接5）
    // - 任意 >=2 个 冲四（rush_four_cnt >= 2） 或 rush_four + live_three -> 胜负威胁
    // - 活四出现 -> 接近必胜
    int combo_bonus = 0;
    if (live_four_cnt > 0) {
        // 活四几乎必胜，给大额加成（但不超过直接五连）
        combo_bonus += SCORE_LIVE_FOUR * 4;
    }
    if (rush_four_cnt >= 2) {
        // 两个冲四能形成双威胁
        combo_bonus += SCORE_RUSH_FOUR * 6;
    } else if (rush_four_cnt == 1 && live_three_cnt >= 1) {
        // 冲四 + 活三 也是致命威胁
        combo_bonus += SCORE_RUSH_FOUR * 4 + SCORE_LIVE_THREE * 2;
    }
    if (live_three_cnt >= 2) {
        // 双活三非常强（比单活四略弱或可相提并论），给很大加分
        combo_bonus += SCORE_LIVE_THREE * 5;
    }
    if (live_three_cnt == 1 && rush_four_cnt == 0 && live_four_cnt == 0) {
        // 单活三仍然重要，适度加分（已经在 dir_score 中）
        combo_bonus += SCORE_LIVE_THREE / 2;
    }
    if (live_two_cnt >= 2) {
        // 多个活二增加进攻潜力
        combo_bonus += SCORE_LIVE_TWO;
    }

    // 微调：如果该点同时对手也能形成类似威胁，则把它设为“关键防守/抢先”走
    // 这个判断留给 generate_moves（在生成时会计算攻防并组合），这里返回的是面向who的分数
    total += combo_bonus;

    // 小的中心偏好（可选，促使在中间更活跃开局）
    int center_bonus = (SIZE/2 - abs(r - SIZE/2)) + (SIZE/2 - abs(c - SIZE/2));
    total += center_bonus; // 很小的影响

    return total;
}


// ========== 改良版 全盘评估 ==========
// 扫描整盘时我们仍然按方向识别模式，但引入“威胁放大”与“对手急迫威胁检测”
// 返回相对于 my_color 的评估值（越大对我越有利）

// 复用之前的 scan_line，但用更稳健的识别（这里保留原 scan_line 思路，但我们将增加组合检测）
// 为简洁起见，我们把 line_scan 保持为原有 scan_line（你已有实现），但在 eval_full_board_scan 中
// 我们进一步统计活三/冲四等全局计数并添加放大系数。

int eval_full_board_scan(int color_to_move) {
    int my_score = 0, opp_score = 0;
    int enemy = opp_color(color_to_move);
    int len = 0;

    int my_live_three = 0, my_rush_four = 0;
    int opp_live_three = 0, opp_rush_four = 0;

    // 1. 行
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) line_buffer[c] = board[r][c];
        my_score += scan_line(SIZE, color_to_move);
        opp_score += scan_line(SIZE, enemy);
    }
    // 2. 列
    for (int c = 0; c < SIZE; c++) {
        for (int r = 0; r < SIZE; r++) line_buffer[r] = board[r][c];
        my_score += scan_line(SIZE, color_to_move);
        opp_score += scan_line(SIZE, enemy);
    }
    // 3. 主对角
    for (int s = -(SIZE - 5); s <= (SIZE - 5); s++) {
        len = 0;
        for (int r = 0; r < SIZE; r++) {
            int c = r - s;
            if (in_bounds(r, c)) line_buffer[len++] = board[r][c];
        }
        if (len >= 5) {
            my_score += scan_line(len, color_to_move);
            opp_score += scan_line(len, enemy);
        }
    }
    // 4. 次对角
    for (int s = 4; s <= 2 * (SIZE - 1) - 4; s++) {
        len = 0;
        for (int r = 0; r < SIZE; r++) {
            int c = s - r;
            if (in_bounds(r, c)) line_buffer[len++] = board[r][c];
        }
        if (len >= 5) {
            my_score += scan_line(len, color_to_move);
            opp_score += scan_line(len, enemy);
        }
    }

    // 警报：如果对手已经有非常大的攻击威胁（例如多个冲四/双活三），放大对手分数
    // 简单启发：如果 opp_score 在短时间内远大于 my_score（>1.5 倍），放大惩罚
    int base_eval = my_score * 10 - opp_score * 9;

    // 威胁放大：检测对手/我方是否有接近必胜的威胁（使用简单阈值）
    if (opp_score > my_score * 2) {
        base_eval -= (opp_score - my_score) * 2; // 加大惩罚，促使防守
    }
    if (my_score > opp_score * 2) {
        base_eval += (my_score - opp_score) * 2;
    }

    // 轻微中心控制奖励，鼓励在中间制造活力（避免走边角死活）
    int center_control = 0;
    for (int x = 0; x < SIZE; x++) for (int y = 0; y < SIZE; y++) {
        if (board[x][y] == color_to_move) {
            center_control += (SIZE/2 - abs(x - SIZE/2)) + (SIZE/2 - abs(y - SIZE/2));
        } else if (board[x][y] == enemy) {
            center_control -= (SIZE/2 - abs(x - SIZE/2)) + (SIZE/2 - abs(y - SIZE/2));
        }
    }
    base_eval += center_control / 2;

    return base_eval;
}

// ========== 走法生成与排序 (融合) =============

typedef struct { int x, y, score; } Move;


int history_score[SIZE][SIZE];
int killer[MAX_PLY][2][2]; // [ply][id][x/y]

int compare_moves(const void *a, const void *b) {
    return ((Move *)b)->score - ((Move *)a)->score;
}


int has_neighbor(int x, int y, int radius) {
    for (int i = -radius; i <= radius; i++) {
        for (int j = -radius; j <= radius; j++) {
            if (i == 0 && j == 0) continue;
            int nx = x + i, ny = y + j;
            if (in_bounds(nx, ny) && board[nx][ny] != EMPTY) {
                return 1;
            }
        }
    }
    return 0;
}

/*
 * (融合) 走法生成
 */
int generate_moves(Move moves[], int color, int ply, int pvx, int pvy) {
    int count = 0;
    int enemy = opp_color(color);
    int considered[SIZE][SIZE];
    memset(considered, 0, sizeof(considered));

    // 1. 检查必杀/必须防守
    // (优化：在 find_best_move 根节点做一次即可，这里只为排序)
    for (int x = 0; x < SIZE; x++)
        for (int y = 0; y < SIZE; y++) {
            if (check_win_move(x, y, color)) {
                moves[count].x = x; moves[count].y = y;
                moves[count].score = SCORE_WIN + 1000;
                count++; return count; // 立即返回
            }
        }
    for (int x = 0; x < SIZE; x++)
        for (int y = 0; y < SIZE; y++) {
            if (check_win_move(x, y, enemy)) {
                moves[count].x = x; moves[count].y = y;
                moves[count].score = SCORE_WIN + 500;
                count++; return count; // 立即返回
            }
        }


    // 2.  TT 提供的 PV Move
    if (pvx >= 0 && pvy >= 0 && board[pvx][pvy] == EMPTY) {
        moves[count].x = pvx;
        moves[count].y = pvy;
        moves[count].score = 20000000; // PV 优先
        considered[pvx][pvy] = 1;
        count++;
    }

    // 3. (融合) 收集邻域内的点
    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            if (board[x][y] != EMPTY || considered[x][y]) continue;

            // (新) 只在有棋子的地方 (半径 NEIGHBOR_RADIUS) 下棋
            if (!has_neighbor(x, y, NEIGHBOR_RADIUS)) continue;

            // (新) 使用 eval_heuristic_move 进行攻防评估
            int att = eval_heuristic_move(x, y, color);
            int def = eval_heuristic_move(x, y, enemy);
           
            int score = att * 10 + def * 9;

            // 加上动态启发分
            score += history_score[x][y];
            if (killer[ply][0][0] == x && killer[ply][0][1] == y) score += 200000;
            if (killer[ply][1][0] == x && killer[ply][1][1] == y) score += 150000;

            moves[count].x = x;
            moves[count].y = y;
            moves[count].score = score;
            considered[x][y] = 1;
            count++;
        }
    }
    
    // (新) 兜底：如果邻域找不到（比如开局），则在中宫
    if (count == 0 && board[SIZE/2][SIZE/2] == EMPTY) {
         moves[count].x = SIZE/2; moves[count].y = SIZE/2;
         moves[count].score = 1;
         considered[SIZE/2][SIZE/2] = 1;
         count++;
    }

    // 排序
    qsort(moves, count, sizeof(Move), compare_moves);

    // 限制宽度
    if (count > MOVE_GEN_WIDTH) count = MOVE_GEN_WIDTH;
    
    return count;
}


static clock_t start_time;
static int time_up = 0;

static inline int elapsed_ms() {
    return (int)((clock() - start_time) * 1000 / CLOCKS_PER_SEC);
}

static inline int time_exceeded() {
    if (time_up) return 1;
    if (elapsed_ms() >= TIME_LIMIT_MS) {
        time_up = 1;
        return 1;
    }
    return 0;
}


int nodes_searched = 0;

int alpha_beta(int depth, int alpha, int beta, int color, int maximizing, int ply) {
    if (time_exceeded()) return 0; // 时间到
    nodes_searched++;

    // 1. 置换表探测
    int tt_score, bx = -1, by = -1;
    if (probe_tt(current_hash, depth, alpha, beta, &tt_score, &bx, &by)) {
        return tt_score;
    }

    // 2. 到达叶节点，调用新评估函数
    if (depth == 0) {
        // 评估值总是相对于 my_color
        return eval_full_board_scan(my_color);
    }

    // 3. 生成走法 (已融合排序)
    Move moves[SIZE * SIZE];
    int pvx = (bx >= 0 && by >= 0 && board[bx][by] == EMPTY) ? bx : -1;
    int pvy = (bx >= 0 && by >= 0 && board[bx][by] == EMPTY) ? by : -1;
    int move_count = generate_moves(moves, color, ply, pvx, pvy);
    
    if (move_count == 0) { // 没棋可走 (平局)
        return 0;
    }

    int best_score = (maximizing) ? -SCORE_WIN * 2 : SCORE_WIN * 2;
    int best_x = -1, best_y = -1;
    int hash_flag = HASH_ALPHA; // 默认为 Alpha (fail-low)

    // (优化) 如果只有一个走法 (强制)，则深度 + 1
    // if (move_count == 1 && depth < MAX_PLY - 2) depth++;

    for (int i = 0; i < move_count; i++) {
        int x = moves[i].x, y = moves[i].y;
        place_stone(x, y, color);

        int score;
        if (check_win_at(x, y, color)) {
            // 越快赢越好
            score = (color == my_color) ? (SCORE_WIN - ply) : (-SCORE_WIN + ply);
        } else {
            score = alpha_beta(depth - 1, alpha, beta, opp_color(color), !maximizing, ply + 1);
        }
        
        remove_stone(x, y);

        if (time_exceeded()) return 0; // 中途退出

        if (maximizing) {
            if (score > best_score) {
                best_score = score;
                best_x = x; best_y = y;
            }
            if (score > alpha) {
                alpha = score;
                hash_flag = HASH_EXACT; // 找到一个 PV
            }
        } else { // minimizing
            if (score < best_score) {
                best_score = score;
                best_x = x; best_y = y;
            }
            if (score < beta) {
                beta = score;
                hash_flag = HASH_EXACT; // 找到一个 PV
            }
        }

        if (alpha >= beta) {
            if (moves[i].score < 10000000) { // 排除 PV Move 和必杀
                if (killer[ply][0][0] != x || killer[ply][0][1] != y) {
                    killer[ply][1][0] = killer[ply][0][0];
                    killer[ply][1][1] = killer[ply][0][1];
                    killer[ply][0][0] = x;
                    killer[ply][0][1] = y;
                }
                history_score[x][y] += depth * depth;
            }
            hash_flag = HASH_BETA; // 发生了 Beta 截断
            break; // 剪枝
        }
    }

    // 存储到 TT
    if (!time_up) {
        store_tt(current_hash, depth, best_score, hash_flag, best_x, best_y);
    }
    return best_score;
}



void find_best_move(int *out_x, int *out_y) {
    // 寻找我方一步胜利
    for (int x = 0; x < SIZE; x++)
        for (int y = 0; y < SIZE; y++) {
            if (check_win_move(x, y, my_color)) {
                *out_x = x; *out_y = y; return;
            }
        }
    // 寻找对方一步胜利 (必须防守)
    for (int x = 0; x < SIZE; x++)
        for (int y = 0; y < SIZE; y++) {
            if (check_win_move(x, y, enemy_color)) {
                *out_x = x; *out_y = y; return;
            }
        }

    start_time = clock();
    time_up = 0;
    nodes_searched = 0;
    // (优化) 不清空 killer, 但清空 history
    memset(history_score, 0, sizeof(history_score));
    // (优化) killer 逐层下降
    for(int i = MAX_PLY-1; i > 0; i--) {
        killer[i][0][0] = killer[i-1][0][0]; killer[i][0][1] = killer[i-1][0][1];
        killer[i][1][0] = killer[i-1][1][0]; killer[i][1][1] = killer[i-1][1][1];
    }
    killer[0][0][0] = -1; killer[0][1][0] = -1;

    int best_x = -1, best_y = -1;
    int best_score = -SCORE_WIN * 10;

    for (int depth = 1; depth <= MAX_PLY; depth++) {
        if (time_exceeded()) break;
        
        // 搜索
        int score = alpha_beta(depth, -SCORE_WIN * 10, SCORE_WIN * 10, my_color, 1, 0);

        if (time_up) break; // 本轮搜索超时，不采用结果


        int idx = tt_index(current_hash);
        if (transposition_table[idx].hash == current_hash && transposition_table[idx].best_x >= 0) {
            best_x = transposition_table[idx].best_x;
            best_y = transposition_table[idx].best_y;
            best_score = transposition_table[idx].score;
        }

        // 找到必胜局，提前退出
        if (best_score >= (SCORE_WIN - MAX_PLY)) break;
        // 找到必败局，也提前退出
        if (best_score <= (-SCORE_WIN + MAX_PLY)) break; 
    }

    if (best_x == -1) {
        // 如果 TT 没找到，从 generate_moves 找一个
        Move moves[SIZE * SIZE];
        int mc = generate_moves(moves, my_color, 0, -1, -1);
        if (mc > 0) {
            best_x = moves[0].x;
            best_y = moves[0].y;
        } else {
            // 终极兜底：找第一个空位
            for (int x = 0; x < SIZE; x++) for (int y = 0; y < SIZE; y++)
                if (board[x][y] == EMPTY) { *out_x = x; *out_y = y; return; }
        }
    }

    *out_x = best_x;
    *out_y = best_y;
}


// 初始化 (保留原开局)
void init_board() {
    clear_tt();
    init_zobrist();
    current_hash = 0ULL;
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++) {
            board[i][j] = EMPTY;
            current_hash ^= zobrist_table[i][j][EMPTY];
        }
    place_stone(5, 5, WHITE);
    place_stone(6, 6, WHITE);
    place_stone(5, 6, BLACK);
    place_stone(6, 5, BLACK);
    memset(history_score, 0, sizeof(history_score));
    memset(killer, 0, sizeof(killer));
}

int main() {
    char cmd[128];
    init_board(); 

    while (fgets(cmd, sizeof(cmd), stdin)) {
        if (strncmp(cmd, "START", 5) == 0) {
            sscanf(cmd, "START %d", &my_color);
            enemy_color = (my_color == BLACK) ? WHITE : BLACK;
            // 重新开始
            init_board();
            printf("OK\n");
            fflush(stdout);
        } else if (strncmp(cmd, "PLACE", 5) == 0) {
            int x, y;
            sscanf(cmd, "PLACE %d %d", &x, &y);
            place_stone(x, y, enemy_color);
        } else if (strncmp(cmd, "TURN", 4) == 0) {
            int bx = -1, by = -1;
            find_best_move(&bx, &by);
            if (bx < 0 || by < 0) {
                // 兜底：找到第一个空位
                for (int x = 0; x < SIZE; x++) for (int y = 0; y < SIZE; y++)
                    if (board[x][y] == EMPTY) { bx = x; by = y; break; }
                if (bx<0) { bx = 0; by = 0; } // 满了？
            }
            place_stone(bx, by, my_color);
            printf("%d %d\n", bx, by);
            fflush(stdout);
        } else if (strncmp(cmd, "END", 3) == 0) {
            break;
        }
    }
    return 0;
}