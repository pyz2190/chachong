#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <vector>

// 基本常量
constexpr int BOARD_SIZE = 12;
using i64 = long long;

namespace Config {
// 最大搜索深度（迭代加深的上限）
static const int FIXED_DEPTH = 7;
// 邻域半径（生成候选）
static const int NEIGHBOR_RADIUS = 2;
// 搜索中的胜负极值
static const int WIN_SCORE = 100000000;
// 候选截断：仅保留分数前 N 个候选
static const int MOVE_CAP = 10;
// TURN 指令软超时（毫秒），尽量不超 2s
static const int SOFT_TIME_MS = 1980;
// 置换表槽位掩码（2^16）
static const uint32_t TT_MASK = 0xFFFFu;
// 点估值权重（进攻/防守简单融合）
static const int ATTACK_WEIGHT = 10;
static const int DEFENSE_WEIGHT = 9;
} // namespace Config

// 棋子颜色定义
enum class Color {
    empty = 0,
    black = 1,
    white = 2
};

// 对手颜色
static inline Color opp(Color c) {
    if (c == Color::black) {
        return Color::white;
    } else if (c == Color::white) {
        return Color::black;
    } else {
        return Color::empty;
    }
}

// 一步棋
struct Move {
    int r;
    int c;
    Color who;
    int hint; // 启发式分数（用于排序）
    Move() : r(0), c(0), who(Color::empty), hint(0) {
    }
    Move(int rr, int cc, Color w) : r(rr), c(cc), who(w), hint(0) {
    }
};

// 棋盘类
class Board {
  private:
    static constexpr int SIZE = BOARD_SIZE;
    std::array<Color, SIZE * SIZE> grid;
    std::vector<Move> history;
    int stoneCnt;

  public:
    // 四个主方向（水平/垂直/两条对角）
    const int d4x[4] = {1, 0, 1, 1};
    const int d4y[4] = {0, 1, 1, -1};

    Board() : grid(), history(), stoneCnt(0) {
        reset();
    }

    // 重置为空棋盘
    void reset() {
        std::fill(grid.begin(), grid.end(), Color::empty);
        history.clear();
        stoneCnt = 0;
    }

    // 索引换算
    static inline int idx(int r, int c) {
        return r * SIZE + c;
    }

    // 边界判断
    static inline bool inBoard(int r, int c) {
        return r >= 0 && r < SIZE && c >= 0 && c < SIZE;
    }

    // 读取/写入
    inline Color at(int r, int c) const {
        return grid[idx(r, c)];
    }

    inline void setCell(int r, int c, Color v) {
        Color old = grid[idx(r, c)];
        if (old == v) {
            return;
        }
        if (old == Color::empty && v != Color::empty) {
            stoneCnt += 1;
        } else if (old != Color::empty && v == Color::empty) {
            stoneCnt -= 1;
        }
        grid[idx(r, c)] = v;
    }

    inline bool isEmpty(int r, int c) const {
        if (!inBoard(r, c)) {
            return false;
        }
        return at(r, c) == Color::empty;
    }

    // 落子
    bool place(const Move &m) {
        if (!inBoard(m.r, m.c) || !isEmpty(m.r, m.c)) {
            return false;
        }
        setCell(m.r, m.c, m.who);
        history.push_back(m);
        return true;
    }

    // 悔手（搜索需要）
    bool undo() {
        if (history.empty()) {
            return false;
        }
        Move m = history.back();
        history.pop_back();
        setCell(m.r, m.c, Color::empty);
        return true;
    }

    // 是否空盘
    bool emptyBoard() const {
        return stoneCnt == 0;
    }

    // 邻域检测（半径 iD 内存在任意棋子）
    bool hasNeighbor(int r, int c, int iD) const {
        for (int dr = -iD; dr <= iD; ++dr) {
            for (int dc = -iD; dc <= iD; ++dc) {
                if (dr == 0 && dc == 0) {
                    continue;
                }
                int nr = r + dr;
                int nc = c + dc;
                if (inBoard(nr, nc) && at(nr, nc) != Color::empty) {
                    return true;
                }
            }
        }
        return false;
    }

    // 从某点判五连
    bool isFiveFrom(int r, int c, Color col) const {
        for (int k = 0; k < 4; ++k) {
            int dr = d4x[k];
            int dc = d4y[k];
            int cnt = 1;
            int rr = r + dr, cc = c + dc;
            while (inBoard(rr, cc) && at(rr, cc) == col) {
                cnt += 1;
                rr += dr;
                cc += dc;
            }
            rr = r - dr;
            cc = c - dc;
            while (inBoard(rr, cc) && at(rr, cc) == col) {
                cnt += 1;
                rr -= dr;
                cc -= dc;
            }
            if (cnt >= 5) {
                return true;
            }
        }
        return false;
    }

    // 上一手是否赢棋
    Color winnerByLast() const {
        if (history.empty()) {
            return Color::empty;
        }
        const Move &m = history.back();
        if (isFiveFrom(m.r, m.c, m.who)) {
            return m.who;
        }
        return Color::empty;
    }

    // 生成候选（空位+邻域过滤），可用于两方
    std::vector<Move> generateCandidates(Color toMove, int neighborRadius) const {
        std::vector<Move> out;
        if (emptyBoard()) {
            int cen = SIZE / 2;
            out.emplace_back(cen, cen, toMove);
            return out;
        }
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                if (isEmpty(r, c) && hasNeighbor(r, c, neighborRadius)) {
                    out.emplace_back(r, c, toMove);
                }
            }
        }
        if (out.empty()) {
            for (int r = 0; r < SIZE; ++r) {
                for (int c = 0; c < SIZE; ++c) {
                    if (isEmpty(r, c)) {
                        out.emplace_back(r, c, toMove);
                    }
                }
            }
        }
        return out;
    }
};

// Zobrist 哈希
struct Zobrist {
    uint64_t key[BOARD_SIZE][BOARD_SIZE][3]; // 使用 0:empty 1:black 2:white
    uint64_t cur;
    std::mt19937_64 gen;
    Zobrist() : key(), cur(0), gen(std::random_device{}()) {
        std::uniform_int_distribution<uint64_t> dist;
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                for (int k = 0; k < 3; ++k) {
                    key[i][j][k] = dist(gen);
                }
            }
        }
        cur = 0;
    }
    inline void apply(int r, int c, Color prev, Color now) {
        cur ^= key[r][c][static_cast<int>(prev)];
        cur ^= key[r][c][static_cast<int>(now)];
    }
};

// 置换表条目
struct TTEntry {
    uint64_t hash;
    int depth;
    int value;
};

// 评分工具：模式评分
namespace ScoreRule {
static const int FIVE = 50000;      // 五连
static const int OPEN_FOUR = 4320;  // 活四
static const int BLOCK_FOUR = 720;  // 冲四/强三
static const int OPEN_THREE = 720;  // 活三/强威胁
static const int BLOCK_THREE = 120; // 眠三/次级威胁
static const int OPEN_TWO = 120;    // 活二
static const int BLOCK_TWO = 20;    // 眠二
static const int SINGLE = 20;       // 散子/单子
} // namespace ScoreRule

// 模式评分函数（count 连子数；block 端点被堵的数量[0..2]；emptyGap 表示中间空位位置处理，-1 表示无空位）
static inline int scorePattern(int count, int block, int emptyGap) {
    if (count >= 5) {
        return ScoreRule::FIVE;
    }
    if (emptyGap == -1) {
        if (block == 0) {
            if (count == 4) {
                return ScoreRule::OPEN_FOUR;
            } else if (count == 3) {
                return ScoreRule::OPEN_THREE;
            } else if (count == 2) {
                return ScoreRule::OPEN_TWO;
            } else {
                return ScoreRule::SINGLE;
            }
        } else if (block == 1) {
            if (count == 4) {
                return ScoreRule::BLOCK_FOUR;
            } else if (count == 3) {
                return ScoreRule::BLOCK_THREE;
            } else if (count == 2) {
                return ScoreRule::BLOCK_TWO;
            } else {
                return ScoreRule::SINGLE;
            }
        } else {
            if (count >= 4) {
                return ScoreRule::BLOCK_FOUR / 2;
            } else {
                return 0;
            }
        }
    } else {
        // 存在一个空位穿插的情况（如 OO_O 或 O_OO），按经验略降分
        if (count == 4) {
            if (block == 0) {
                return ScoreRule::BLOCK_FOUR; // 空一格的四基本等同冲四
            } else {
                return ScoreRule::BLOCK_FOUR / 2;
            }
        } else if (count == 3) {
            if (block == 0) {
                return ScoreRule::BLOCK_THREE + ScoreRule::OPEN_TWO; // 介于活三与眠三之间
            } else {
                return ScoreRule::BLOCK_THREE;
            }
        } else if (count == 2) {
            if (block == 0) {
                return ScoreRule::OPEN_TWO / 2;
            } else {
                return ScoreRule::SINGLE;
            }
        } else {
            return ScoreRule::SINGLE;
        }
    }
}

// 单点落子启发式评分（仅查看四个方向，成本低，用于排序与即时判断）
static int evaluatePlacementQuick(const Board &bd, int r, int c, Color who) {
    int total = 0;
    // 四方向扫描，统计连续子数、端点封堵数和是否存在单空
    for (int k = 0; k < 4; ++k) {
        int dr = bd.d4x[k];
        int dc = bd.d4y[k];
        int cnt = 1; // 包含自身
        int block = 0;
        int emptyGap = -1;

        // 正向
        int i = r + dr, j = c + dc;
        while (Board::inBoard(i, j)) {
            Color v = bd.at(i, j);
            if (v == who) {
                cnt += 1;
                i += dr;
                j += dc;
                continue;
            } else if (v == Color::empty) {
                // 允许一个空位的“跳连”
                int ii = i + dr, jj = j + dc;
                if (Board::inBoard(ii, jj) && bd.at(ii, jj) == who && emptyGap == -1) {
                    emptyGap = cnt; // 记录空位出现的位置（粗略）
                    cnt += 1;
                    i = ii + dr;
                    j = jj + dc;
                    continue;
                }
                break;
            } else {
                block += 1;
                break;
            }
        }

        // 反向
        i = r - dr;
        j = c - dc;
        while (Board::inBoard(i, j)) {
            Color v = bd.at(i, j);
            if (v == who) {
                cnt += 1;
                i -= dr;
                j -= dc;
                continue;
            } else if (v == Color::empty) {
                int ii = i - dr, jj = j - dc;
                if (Board::inBoard(ii, jj) && bd.at(ii, jj) == who && emptyGap == -1) {
                    emptyGap = cnt;
                    cnt += 1;
                    i = ii - dr;
                    j = jj - dc;
                    continue;
                }
                break;
            } else {
                block += 1;
                break;
            }
        }

        total += scorePattern(cnt, block, emptyGap);
    }
    return total;
}

// 整体局面评分（对行/列/两斜线进行线性扫描），返回对 toMove 方的相对优势
static int evaluateBoardFull(const Board &bd, Color toMove) {
    auto lineScoreFor = [&](Color who) -> int {
        int sum = 0;
        std::function<void(const std::function<Color(int)> &, int)> scanLine =
            [&](const std::function<Color(int)> &getter, int len) {
                int i = 0;
                while (i < len) {
                    if (getter(i) != who) {
                        i += 1;
                        continue;
                    }
                    int j = i;
                    int cnt = 0;
                    int block = 0;
                    int emptyGap = -1;

                    // 左侧是否被堵
                    if (i - 1 < 0 || getter(i - 1) == opp(who)) {
                        block += 1;
                    }

                    // 顺着统计 who 与可能的空一格跳连
                    bool usedGap = false;
                    while (j < len && getter(j) == who) {
                        cnt += 1;
                        j += 1;
                    }
                    // 尝试“空一格”后继续连
                    if (j < len && getter(j) == Color::empty) {
                        int t = j + 1;
                        while (t < len && getter(t) == who) {
                            if (!usedGap) {
                                emptyGap = cnt;
                                usedGap = true;
                            }
                            cnt += 1;
                            t += 1;
                        }
                        j = t;
                    }

                    // 右侧是否被堵
                    if (j >= len || getter(j) == opp(who)) {
                        block += 1;
                    }

                    sum += scorePattern(cnt, block, emptyGap);
                    i = j + 1;
                }
            };

        // 行
        for (int r = 0; r < BOARD_SIZE; ++r) {
            scanLine(std::function<Color(int)>([&](int x) { return bd.at(r, x); }), BOARD_SIZE);
        }
        // 列
        for (int c = 0; c < BOARD_SIZE; ++c) {
            scanLine(std::function<Color(int)>([&](int x) { return bd.at(x, c); }), BOARD_SIZE);
        }
        // 主对角线（从左下到右上长度>=5的线）
        for (int s = -(BOARD_SIZE - 5); s <= (BOARD_SIZE - 5); ++s) {
            // 以 (r, c) 满足 r - c = s
            std::vector<Color> tmp;
            tmp.reserve(BOARD_SIZE);
            for (int r = 0; r < BOARD_SIZE; ++r) {
                int c = r - s;
                if (c >= 0 && c < BOARD_SIZE) {
                    tmp.push_back(bd.at(r, c));
                }
            }
            if (static_cast<int>(tmp.size()) >= 5) {
                scanLine(std::function<Color(int)>([&](int x) { return tmp[x]; }), static_cast<int>(tmp.size()));
            }
        }
        // 次对角线（左上到右下）
        for (int s = 4; s <= 2 * BOARD_SIZE - 5; ++s) {
            std::vector<Color> tmp;
            tmp.reserve(BOARD_SIZE);
            for (int r = 0; r < BOARD_SIZE; ++r) {
                int c = s - r;
                if (c >= 0 && c < BOARD_SIZE) {
                    tmp.push_back(bd.at(r, c));
                }
            }
            if (static_cast<int>(tmp.size()) >= 5) {
                scanLine(std::function<Color(int)>([&](int x) { return tmp[x]; }), static_cast<int>(tmp.size()));
            }
        }
        return sum;
    };

    int self = lineScoreFor(toMove);
    int rival = lineScoreFor(opp(toMove));
    return self - rival;
}

// 大脑：管理棋盘、搜索、IO
class Brain {
  private:
    Board bd;
    Zobrist zob;
    std::vector<TTEntry> tt;
    Color me;
    Color enemy;
    int steps;
    std::chrono::steady_clock::time_point tStart;
    // 预留一点缓冲，临近超时时提前返回当前最优
    inline bool nearTimeout(int reserveMs = 50) const {
        auto now = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - tStart).count();
        if (Config::SOFT_TIME_MS <= reserveMs) {
            return ms >= Config::SOFT_TIME_MS; // 兜底
        } else {
            return ms >= (Config::SOFT_TIME_MS - reserveMs);
        }
    }

  public:
    Brain() : bd(), zob(), tt(), me(Color::black), enemy(Color::white), steps(0), tStart() {
        tt.resize(Config::TT_MASK + 1);
        for (auto &e : tt) {
            e.hash = 0ull;
            e.depth = -1;
            e.value = 0;
        }
    }

    // 初始化：根据我方颜色设置默认四子，并回复 OK
    void onStart(int myFlag) {
        if (myFlag == 1) {
            me = Color::black;
            enemy = Color::white;
        } else {
            me = Color::white;
            enemy = Color::black;
        }
        bd.reset();
        zob = Zobrist();
        steps = 0;

        // 开局默认放置四子
        // 白: (5,5) (6,6)  黑: (5,6) (6,5)
        placeStone(5, 5, Color::white);
        placeStone(6, 6, Color::white);
        placeStone(5, 6, Color::black);
        placeStone(6, 5, Color::black);
        steps = 4;

        std::cout << "OK\n";
        std::cout.flush();
    }

    // 对手落子
    void onPlace(int x, int y) {
        placeStone(x, y, enemy);
        steps += 1;
    }

    // 我方回合
    void onTurn() {
        tStart = std::chrono::steady_clock::now();

        int bestR = -1, bestC = -1;
        int bestScore = std::numeric_limits<int>::min();

        // 先进行即时战术：一手取胜或必堵
        if (tryImmediate(bestR, bestC, bestScore)) {
            outputMove(bestR, bestC);
            return;
        }

        // 迭代加深搜索（尽量用满时间）
        std::vector<Move> rootMoves = makeCandidates(me);
        if (rootMoves.empty()) {
            // 兜底：落在中心
            bestR = BOARD_SIZE / 2;
            bestC = BOARD_SIZE / 2;
            outputMove(bestR, bestC);
            return;
        }

        // 预排序
        sortCandidates(rootMoves, me);

        for (int depth = 1; depth <= Config::FIXED_DEPTH; ++depth) {
            int localBestR = -1, localBestC = -1;
            int alpha = std::numeric_limits<int>::min() / 2;
            int beta = std::numeric_limits<int>::max() / 2;
            int val = std::numeric_limits<int>::min();

            // 按顺序搜索候选
            int limit = std::min<int>(Config::MOVE_CAP, static_cast<int>(rootMoves.size()));
            for (int i = 0; i < limit; ++i) {
                const Move &mv = rootMoves[i];
                placeStone(mv.r, mv.c, me);
                int s;
                if (bd.isFiveFrom(mv.r, mv.c, me)) {
                    s = Config::WIN_SCORE - 1;
                } else {
                    s = -negamax(depth - 1, -beta, -alpha, enemy);
                }
                undoStone(mv.r, mv.c, me);

                if (s > val) {
                    val = s;
                    localBestR = mv.r;
                    localBestC = mv.c;
                }
                if (s > alpha) {
                    alpha = s;
                }

                // 超时检查
                if (nearTimeout()) {
                    // 临近超时，直接输出当前已知最好
                    int outR = -1, outC = -1;
                    if (localBestR != -1) {
                        outR = localBestR;
                        outC = localBestC;
                    } else if (bestR != -1) {
                        outR = bestR;
                        outC = bestC;
                    } else {
                        outR = rootMoves[0].r;
                        outC = rootMoves[0].c;
                    }
                    outputMove(outR, outC);
                    return;
                }
            }

            if (localBestR != -1) {
                bestR = localBestR;
                bestC = localBestC;
                bestScore = val;
            }

            if (nearTimeout()) {
                // 临近超时，直接输出迭代到目前为止的最好
                if (bestR == -1) {
                    bestR = rootMoves[0].r;
                    bestC = rootMoves[0].c;
                }
                outputMove(bestR, bestC);
                return;
            }
        }

        if (bestR == -1) {
            bestR = rootMoves[0].r;
            bestC = rootMoves[0].c;
        }
        outputMove(bestR, bestC);
    }

    // END 指令
    void onEnd(int who) {
        // 平台会关闭进程，这里不做额外处理
        (void)who;
    }

  private:
    // 放置/撤回棋子（维护 Zobrist）
    inline void placeStone(int r, int c, Color col) {
        Color prev = bd.at(r, c);
        bd.setCell(r, c, col);
        zob.apply(r, c, prev, col);
    }
    inline void undoStone(int r, int c, Color col) {
        Color prev = bd.at(r, c);
        (void)col;
        bd.setCell(r, c, Color::empty);
        zob.apply(r, c, prev, Color::empty);
    }

    // 超时检测
    inline bool timeout() const {
        auto now = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - tStart).count();
        return ms >= Config::SOFT_TIME_MS;
    }

    // 顶层输出
    void outputMove(int r, int c) {
        placeStone(r, c, me);
        std::cout << r << ' ' << c << "\n";
        std::cout.flush();
        steps += 1;
    }

    // 生成候选并打上启发式分数
    std::vector<Move> makeCandidates(Color side) const {
        std::vector<Move> cand = bd.generateCandidates(side, Config::NEIGHBOR_RADIUS);
        for (auto &m : cand) {
            int att = evaluatePlacementQuick(bd, m.r, m.c, side);
            int def = evaluatePlacementQuick(bd, m.r, m.c, opp(side));
            m.hint = Config::ATTACK_WEIGHT * att + Config::DEFENSE_WEIGHT * def;
        }
        return cand;
    }

    // 排序（高分在前）
    void sortCandidates(std::vector<Move> &mv, Color side) const {
        (void)side;
        std::sort(mv.begin(), mv.end(), [](const Move &a, const Move &b) {
            if (a.hint != b.hint) {
                return a.hint > b.hint;
            }
            if (a.r != b.r) {
                return a.r < b.r;
            }
            return a.c < b.c;
        });
    }

    // 即时战术：
    // 1) 我方任一候选一手成五，立即走；
    // 2) 对手任一候选一手成五，立即堵；
    bool tryImmediate(int &outR, int &outC, int &outScore) {
        // 我方尝试一手取胜
        std::vector<Move> m1 = makeCandidates(me);
        sortCandidates(m1, me);
        int lim1 = std::min<int>(Config::MOVE_CAP, static_cast<int>(m1.size()));
        for (int i = 0; i < lim1; ++i) {
            const Move &mv = m1[i];
            placeStone(mv.r, mv.c, me);
            bool win = bd.isFiveFrom(mv.r, mv.c, me);
            undoStone(mv.r, mv.c, me);
            if (win) {
                outR = mv.r;
                outC = mv.c;
                outScore = Config::WIN_SCORE;
                return true;
            }
            if (timeout()) {
                break;
            }
        }
        // 我方堵住对手一手取胜
        std::vector<Move> m2 = makeCandidates(enemy);
        sortCandidates(m2, enemy);
        int lim2 = std::min<int>(Config::MOVE_CAP, static_cast<int>(m2.size()));
        for (int i = 0; i < lim2; ++i) {
            const Move &mv = m2[i];
            placeStone(mv.r, mv.c, enemy);
            bool oppWin = bd.isFiveFrom(mv.r, mv.c, enemy);
            undoStone(mv.r, mv.c, enemy);
            if (oppWin && bd.isEmpty(mv.r, mv.c)) {
                outR = mv.r;
                outC = mv.c;
                outScore = ScoreRule::OPEN_FOUR; // 作为提示值
                return true;
            }
            if (timeout()) {
                break;
            }
        }
        return false;
    }

    // AlphaBeta（Negamax 写法）
    int negamax(int depth, int alpha, int beta, Color side) {
        // 超时或深度到达
        if (depth == 0 || timeout()) {
            return evaluateBoardFull(bd, side);
        }

        // 置换表探测
        TTEntry &ent = tt[static_cast<size_t>(zob.cur & Config::TT_MASK)];
        if (ent.hash == (zob.cur | (1ull << 63)) && ent.depth >= depth) {
            return ent.value;
        }

        // 候选与排序
        std::vector<Move> cand = makeCandidates(side);
        sortCandidates(cand, side);
        int limit = std::min<int>(Config::MOVE_CAP, static_cast<int>(cand.size()));

        int best = std::numeric_limits<int>::min();
        for (int i = 0; i < limit; ++i) {
            const Move &mv = cand[i];
            placeStone(mv.r, mv.c, side);

            int val;
            if (bd.isFiveFrom(mv.r, mv.c, side)) {
                val = Config::WIN_SCORE - (Config::FIXED_DEPTH - depth + 1);
            } else {
                val = -negamax(depth - 1, -beta, -alpha, opp(side));
            }

            undoStone(mv.r, mv.c, side);

            if (val > best) {
                best = val;
            }
            if (val > alpha) {
                alpha = val;
            }
            if (alpha >= beta) {
                break; // Beta 剪枝
            }
            if (timeout()) {
                break;
            }
        }

        // 置换表写入
        ent.hash = (zob.cur | (1ull << 63));
        ent.depth = depth;
        ent.value = best == std::numeric_limits<int>::min() ? 0 : best;
        return ent.value;
    }
};

// 入口：解析指令，与评测平台交互
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    Brain ai;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            continue;
        }
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        if (cmd == "START") {
            int flag = 0;
            if (!(iss >> flag)) {
                // 若 START 没参数，尝试再读一次
                if (!(std::cin >> flag)) {
                    // 输入异常
                    break;
                }
                std::string dummy;
                std::getline(std::cin, dummy);
            }
            ai.onStart(flag);
        } else if (cmd == "PLACE") {
            int x = 0, y = 0;
            iss >> x >> y;
            ai.onPlace(x, y);
        } else if (cmd == "TURN") {
            ai.onTurn();
        } else if (cmd == "END") {
            int who = 0;
            iss >> who;
            ai.onEnd(who);
            break;
        } else {
            // 未知指令，忽略
        }
    }
    return 0;
}