#include "games.h"
#include "font3d.h"
#include "gfx3d.h"
#include "color.h"

#include <math.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Shared helpers                                                      */
/* ------------------------------------------------------------------ */

static const char *FULL_NAMES[GAME_COUNT] = {
    "PAC-MAN 3D", "TETRIS 3D", "SPACE INVADERS 3D", "PONG 3D"
};
static const char *SHORT_NAMES[GAME_COUNT] = {
    "PAC-MAN", "TETRIS", "SPACE INVADERS", "PONG"
};

static uint32_t g_rng = 0x13579BDFu;

static uint32_t rnd(void)
{
    uint32_t x = g_rng;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    g_rng = x;
    return x;
}

static int clampi(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static float clampf(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static void draw_number(uint32_t value, int digits, float x, float y,
                        float z, float size, GColor color)
{
    char b[16];
    font3d_uint(b, sizeof(b), value, digits);
    font3d_draw(b, x, y, z, size, color);
}

static void draw_common_hud(int id)
{
    gfx3d_add_box(0.0f, 6.18f, 19.4f, 0.92f, 1.35f, 0.16f, C_PANEL_DARK);
    font3d_draw(FULL_NAMES[id], -9.35f, 5.84f, 1.62f, 0.088f, C_CYAN);
    font3d_draw_right("START: PAUSE", 9.35f, 5.86f, 1.62f, 0.062f, C_MUTED);
    font3d_draw_centered("D-PAD PLAY   START: PAUSE", 0.0f, -6.58f, 1.62f, 0.052f, C_MUTED);
}

static void draw_message(const char *text, GColor color)
{
    gfx3d_add_box(0.0f, 0.0f, 7.6f, 1.15f, 1.92f, 0.22f, C_PANEL);
    font3d_draw_centered(text, 0.0f, -0.36f, 2.18f, 0.115f, color);
}

/* ------------------------------------------------------------------ */
/* Game 1: PAC-MAZE 3D                                                 */
/* ------------------------------------------------------------------ */

#define PAC_W 15
#define PAC_H 13
#define PAC_CELL 0.66f
#define PAC_LEFT -8.45f
#define PAC_TOP 3.90f

static const char PAC_MAZE[PAC_H][PAC_W + 1] = {
    "###############",
    "#.............#",
    "#.##.###.##.#.#",
    "#.............#",
    "#.##.#.#.#.##.#",
    "#....#.#......#",
    "#.##.#.#.#.##.#",
    "#.............#",
    "#.##.###.##.#.#",
    "#.............#",
    "#.##.#.#.#.##.#",
    "#.............#",
    "###############"
};

typedef struct {
    int x, y, dx, dy;
} PacGhost;

static uint8_t pac_pellet[PAC_H][PAC_W];
static uint32_t pac_score, pac_level, pac_pellets_left, pac_tick;
static int pac_x, pac_y, pac_dirx, pac_diry, pac_nextx, pac_nexty;
static int pac_message_timer, pac_message_kind, pac_grace, pac_power;
static PacGhost pac_ghosts[4];

static const GColor PAC_GHOST_COLORS[4] = {
    {255, 82, 162, 0x80},
    {64, 232, 255, 0x80},
    {255, 138, 48, 0x80},
    {178, 92, 255, 0x80}
};

static int pac_wall(int x, int y)
{
    if (x < 0 || y < 0 || x >= PAC_W || y >= PAC_H) return 1;
    return PAC_MAZE[y][x] == '#';
}

static int pac_spawn_cell(int x, int y)
{
    return (x == 7 && y == 7) ||
           (x == 1 && y == 1) || (x == 13 && y == 1) ||
           (x == 1 && y == 11) || (x == 13 && y == 11);
}

static float pac_cx(int c) { return PAC_LEFT + ((float)c + 0.5f) * PAC_CELL; }
static float pac_cy(int r) { return PAC_TOP - ((float)r + 0.5f) * PAC_CELL; }

static void pac_reset_positions(void)
{
    pac_x = 7; pac_y = 7;
    pac_dirx = 0; pac_diry = 0;
    pac_nextx = 0; pac_nexty = 0;
    pac_ghosts[0].x = 1;  pac_ghosts[0].y = 1;  pac_ghosts[0].dx = 1;  pac_ghosts[0].dy = 0;
    pac_ghosts[1].x = 13; pac_ghosts[1].y = 1;  pac_ghosts[1].dx = -1; pac_ghosts[1].dy = 0;
    pac_ghosts[2].x = 1;  pac_ghosts[2].y = 11; pac_ghosts[2].dx = 0;  pac_ghosts[2].dy = -1;
    pac_ghosts[3].x = 13; pac_ghosts[3].y = 11; pac_ghosts[3].dx = 0;  pac_ghosts[3].dy = -1;
    pac_grace = 70;
    pac_power = 0;
}

static void pac_refill_pellets(void)
{
    int x, y;
    pac_pellets_left = 0;
    for (y = 0; y < PAC_H; y++) {
        for (x = 0; x < PAC_W; x++) {
            if (!pac_wall(x, y) && !pac_spawn_cell(x, y)) {
                pac_pellet[y][x] =
                    ((x == 1 || x == 13) && (y == 3 || y == 9)) ? 2 : 1;
                pac_pellets_left++;
            } else {
                pac_pellet[y][x] = 0;
            }
        }
    }
}

static void pac_reset(void)
{
    g_rng = 0x0A11CE55u;
    pac_score = 0;
    pac_level = 1;
    pac_tick = 0;
    pac_message_timer = 0;
    pac_message_kind = 0;
    pac_refill_pellets();
    pac_reset_positions();
}

static void pac_next_level(void)
{
    pac_level++;
    pac_score += 1000;
    pac_refill_pellets();
    pac_reset_positions();
    pac_message_timer = 95;
    pac_message_kind = 1;
}

static void pac_caught(void)
{
    pac_reset_positions();
    pac_message_timer = 65;
    pac_message_kind = 2;
}

static void pac_move_ghost(PacGhost *g)
{
    static const int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    int cand[4][2];
    int n = 0, i;
    for (i = 0; i < 4; i++) {
        int dx = dirs[i][0], dy = dirs[i][1];
        if (dx == -g->dx && dy == -g->dy) continue;
        if (!pac_wall(g->x + dx, g->y + dy)) {
            cand[n][0] = dx; cand[n][1] = dy; n++;
        }
    }
    if (n == 0) {
        for (i = 0; i < 4; i++) {
            int dx = dirs[i][0], dy = dirs[i][1];
            if (!pac_wall(g->x + dx, g->y + dy)) {
                cand[n][0] = dx; cand[n][1] = dy; n++;
            }
        }
    }
    if (n > 0) {
        int current_ok = !pac_wall(g->x + g->dx, g->y + g->dy);
        if (!current_ok || (rnd() & 3u) == 0u) {
            int pick = (int)(rnd() % (uint32_t)n);
            g->dx = cand[pick][0];
            g->dy = cand[pick][1];
        }
        g->x += g->dx;
        g->y += g->dy;
    }
}

static void pac_update(const InputState *in, uint32_t frame)
{
    int i;
    (void)frame;
    pac_tick++;
    if (pac_message_timer > 0) pac_message_timer--;
    if (pac_grace > 0) pac_grace--;
    if (pac_power > 0) pac_power--;

    if (in->held & IN_UP)    { pac_nextx = 0;  pac_nexty = -1; }
    if (in->held & IN_DOWN)  { pac_nextx = 0;  pac_nexty = 1; }
    if (in->held & IN_LEFT)  { pac_nextx = -1; pac_nexty = 0; }
    if (in->held & IN_RIGHT) { pac_nextx = 1;  pac_nexty = 0; }

    if (pac_tick % 5u == 0u) {
        if (!pac_wall(pac_x + pac_nextx, pac_y + pac_nexty)) {
            pac_dirx = pac_nextx;
            pac_diry = pac_nexty;
        }
        if (!pac_wall(pac_x + pac_dirx, pac_y + pac_diry)) {
            pac_x += pac_dirx;
            pac_y += pac_diry;
            if (pac_pellet[pac_y][pac_x]) {
                if (pac_pellet[pac_y][pac_x] == 2) {
                    pac_power = 400;
                    pac_score += 50;
                } else pac_score += 10;
                pac_pellet[pac_y][pac_x] = 0;
                pac_pellets_left--;
                if (pac_pellets_left == 0) pac_next_level();
            }
        }
    }

    {
        int difficulty = (int)(pac_level > 9u ? 8u : pac_level - 1u);
        int ghost_period = clampi(13 - difficulty, 5, 13);
        if (pac_tick % (uint32_t)ghost_period == 0u)
            for (i = 0; i < 4; i++) pac_move_ghost(&pac_ghosts[i]);
    }

    if (pac_grace == 0) {
        for (i = 0; i < 4; i++) {
            if (pac_ghosts[i].x == pac_x && pac_ghosts[i].y == pac_y) {
                if (pac_power > 0) {
                    static const int spawn_x[4] = {1, 13, 1, 13};
                    static const int spawn_y[4] = {1, 1, 11, 11};
                    pac_ghosts[i].x = spawn_x[i];
                    pac_ghosts[i].y = spawn_y[i];
                    pac_score += 200;
                } else {
                    pac_caught();
                    break;
                }
            }
        }
    }
}

static void pac_draw_player(float cx, float cy)
{
    float wx = (float)pac_dirx;
    float wy = -(float)pac_diry;
    float len = sqrtf(wx * wx + wy * wy);
    float px, py, r = 0.30f, z = 1.45f;
    GVec3 a, b, c;
    if (len < 0.001f) { wx = 1.0f; wy = 0.0f; }
    else { wx /= len; wy /= len; }
    px = -wy; py = wx;
    gfx3d_add_sphere(cx, cy, 1.13f, r, C_YELLOW, 12, 7);
    a = gfx3d_vec(cx, cy, z);
    b = gfx3d_vec(cx + r * wx + r * 0.62f * px,
                  cy + r * wy + r * 0.62f * py, z);
    c = gfx3d_vec(cx + r * wx - r * 0.62f * px,
                  cy + r * wy - r * 0.62f * py, z);
    gfx3d_add_triangle(a, b, c, C_BLACK, C_BLACK, C_BLACK);
}

static void pac_draw(void)
{
    int x, y, i;
    GColor wall = gfx3d_mix(C_BLUE, C_PURPLE,
                            (float)((pac_level - 1u) % 7u) / 7.0f);
    char b[16];

    gfx3d_add_box(-3.48f, -0.42f, 10.52f, 9.22f, 0.0f, 0.42f, C_PANEL_DARK);
    gfx3d_add_box(5.82f, 0.0f, 5.25f, 8.75f, 0.08f, 0.40f, C_PANEL);

    for (y = 0; y < PAC_H; y++) {
        for (x = 0; x < PAC_W; x++) {
            float cx = pac_cx(x), cy = pac_cy(y);
            if (pac_wall(x, y)) {
                gfx3d_add_box(cx, cy, PAC_CELL * 0.92f, PAC_CELL * 0.92f,
                              0.56f, 0.30f, wall);
            } else if (pac_pellet[y][x]) {
                float ps = pac_pellet[y][x] == 2 ? 0.24f : 0.115f;
                gfx3d_add_box(cx, cy, ps, ps, 0.82f, 0.075f,
                              pac_pellet[y][x] == 2 ? C_WHITE : C_YELLOW);
            }
        }
    }

    for (i = 0; i < 4; i++) {
        float cx = pac_cx(pac_ghosts[i].x);
        float cy = pac_cy(pac_ghosts[i].y);
        gfx3d_add_sphere(cx, cy, 1.02f, 0.265f,
                         pac_power > 0 ? C_BLUE : PAC_GHOST_COLORS[i], 10, 6);
        gfx3d_add_box(cx - 0.085f, cy + 0.045f, 0.095f, 0.115f, 1.31f, 0.035f, C_WHITE);
        gfx3d_add_box(cx + 0.085f, cy + 0.045f, 0.095f, 0.115f, 1.31f, 0.035f, C_WHITE);
        gfx3d_add_box(cx - 0.085f, cy + 0.035f, 0.040f, 0.050f, 1.37f, 0.025f, C_BLACK);
        gfx3d_add_box(cx + 0.085f, cy + 0.035f, 0.040f, 0.050f, 1.37f, 0.025f, C_BLACK);
    }
    pac_draw_player(pac_cx(pac_x), pac_cy(pac_y));

    font3d_draw("SCORE", 3.58f, 2.92f, 1.15f, 0.078f, C_MUTED);
    draw_number(pac_score, 6, 3.58f, 2.05f, 1.15f, 0.125f, C_WHITE);
    font3d_draw("LEVEL", 3.58f, 1.10f, 1.15f, 0.078f, C_MUTED);
    draw_number(pac_level, 4, 3.58f, 0.24f, 1.15f, 0.125f, C_YELLOW);
    font3d_draw("PELLETS", 3.58f, -0.72f, 1.15f, 0.070f, C_MUTED);
    draw_number(pac_pellets_left, 3, 3.58f, -1.53f, 1.15f, 0.120f, C_CYAN);
    font3d_draw("D-PAD MOVE", 3.58f, -2.62f, 1.15f, 0.058f, C_WHITE);
    font3d_draw(pac_power > 0 ? "POWER ACTIVE" : "POWER PELLETS",
                3.58f, -3.22f, 1.15f, 0.052f, pac_power > 0 ? C_CYAN : C_MUTED);
    font3d_uint(b, sizeof(b), pac_level, 2);
    font3d_draw_right(b, 8.15f, 3.70f, 1.15f, 0.070f, C_PURPLE);

    draw_common_hud(GAME_PAC);
    if (pac_message_timer > 0)
        draw_message(pac_message_kind == 1 ? "LEVEL CLEAR!" : "CAUGHT!",
                     pac_message_kind == 1 ? C_GREEN : C_RED);
}

/* ------------------------------------------------------------------ */
/* Game 2: BLOCKFALL 3D                                                */
/* ------------------------------------------------------------------ */

#define TET_W 10
#define TET_H 20
#define TET_CELL 0.44f
#define TET_LEFT -6.30f
#define TET_TOP 4.10f

static const char *TET_SHAPES[7][4] = {
    {"....", "XXXX", "....", "...."},
    {".XX.", ".XX.", "....", "...."},
    {".X..", "XXX.", "....", "...."},
    {".XX.", "XX..", "....", "...."},
    {"XX..", ".XX.", "....", "...."},
    {"X...", "XXX.", "....", "...."},
    {"..X.", "XXX.", "....", "...."}
};

static const GColor TET_COLORS[7] = {
    {64, 232, 255, 0x80},
    {255, 220, 64, 0x80},
    {178, 92, 255, 0x80},
    {76, 236, 132, 0x80},
    {255, 74, 98, 0x80},
    {64, 132, 255, 0x80},
    {255, 138, 48, 0x80}
};

static uint8_t tet_board[TET_H][TET_W];
static uint32_t tet_score, tet_lines, tet_level;
static int tet_px, tet_py, tet_type, tet_rot, tet_next;
static int tet_hold, tet_hold_used;
static int tet_gravity, tet_message_timer, tet_message_kind;

static float tet_cx(int c) { return TET_LEFT + ((float)c + 0.5f) * TET_CELL; }
static float tet_cy(int r) { return TET_TOP - ((float)r + 0.5f) * TET_CELL; }

static void tet_get_cells(int type, int rot, int out[4][2])
{
    int x, y, n = 0;
    rot &= 3;
    for (y = 0; y < 4; y++) {
        for (x = 0; x < 4; x++) {
            if (TET_SHAPES[type][y][x] == 'X') {
                int px = x, py = y, r;
                for (r = 0; r < rot; r++) {
                    int nx = py;
                    int ny = 3 - px;
                    px = nx; py = ny;
                }
                if (n < 4) { out[n][0] = px; out[n][1] = py; n++; }
            }
        }
    }
    while (n < 4) { out[n][0] = 0; out[n][1] = 0; n++; }
}

static int tet_collides(int px, int py, int rot)
{
    int cells[4][2], i;
    tet_get_cells(tet_type, rot, cells);
    for (i = 0; i < 4; i++) {
        int bx = px + cells[i][0];
        int by = py + cells[i][1];
        if (bx < 0 || bx >= TET_W || by >= TET_H) return 1;
        if (by >= 0 && tet_board[by][bx]) return 1;
    }
    return 0;
}

static void tet_spawn(void)
{
    tet_type = tet_next;
    tet_next = (int)(rnd() % 7u);
    tet_px = 3;
    tet_py = -1;
    tet_rot = 0;
    tet_gravity = 0;
    tet_hold_used = 0;
}

static void tet_clear_lines(void)
{
    int y, cleared = 0;
    for (y = 0; y < TET_H; y++) {
        int x, full = 1;
        for (x = 0; x < TET_W; x++) if (!tet_board[y][x]) { full = 0; break; }
        if (full) {
            int yy, xx;
            for (yy = y; yy > 0; yy--)
                for (xx = 0; xx < TET_W; xx++)
                    tet_board[yy][xx] = tet_board[yy - 1][xx];
            for (xx = 0; xx < TET_W; xx++) tet_board[0][xx] = 0;
            cleared++;
            y--;
        }
    }
    if (cleared > 0) {
        tet_lines += (uint32_t)cleared;
        tet_score += (uint32_t)(cleared * cleared) * 100u * tet_level;
        tet_level = tet_lines / 10u + 1u;
        tet_message_timer = 65;
        tet_message_kind = cleared >= 4 ? 2 : 1;
    }
}

static void tet_lock(void)
{
    int cells[4][2], i, topout = 0;
    tet_get_cells(tet_type, tet_rot, cells);
    for (i = 0; i < 4; i++) {
        int bx = tet_px + cells[i][0];
        int by = tet_py + cells[i][1];
        if (by < 0) { topout = 1; break; }
        if (bx >= 0 && bx < TET_W && by < TET_H)
            tet_board[by][bx] = (uint8_t)(tet_type + 1);
    }
    if (topout) {
        memset(tet_board, 0, sizeof(tet_board));
        tet_message_timer = 80;
        tet_message_kind = 3;
    } else {
        tet_clear_lines();
    }
    tet_spawn();
    if (tet_collides(tet_px, tet_py, tet_rot)) {
        memset(tet_board, 0, sizeof(tet_board));
        tet_spawn();
    }
}

static int tet_try_move(int dx, int dy)
{
    if (!tet_collides(tet_px + dx, tet_py + dy, tet_rot)) {
        tet_px += dx; tet_py += dy;
        return 1;
    }
    return 0;
}

static void tet_rotate(void)
{
    static const int kicks[5] = {0, -1, 1, -2, 2};
    int i, nr = (tet_rot + 1) & 3;
    for (i = 0; i < 5; i++) {
        if (!tet_collides(tet_px + kicks[i], tet_py, nr)) {
            tet_px += kicks[i];
            tet_rot = nr;
            return;
        }
    }
}

static void tet_reset(void)
{
    g_rng = 0x20B10C4u;
    memset(tet_board, 0, sizeof(tet_board));
    tet_score = 0; tet_lines = 0; tet_level = 1;
    tet_hold = -1; tet_hold_used = 0;
    tet_next = (int)(rnd() % 7u);
    tet_message_timer = 0; tet_message_kind = 0;
    tet_spawn();
}

/* One hold per falling piece. Locking a piece enables hold again. */
static void tet_hold_piece(void)
{
    int old_type;
    if (tet_hold_used) return;
    old_type = tet_type;
    if (tet_hold < 0) {
        tet_hold = old_type;
        tet_spawn();
    } else {
        tet_type = tet_hold;
        tet_hold = old_type;
        tet_px = 3;
        tet_py = -1;
        tet_rot = 0;
        tet_gravity = 0;
    }
    tet_hold_used = 1;
    if (tet_collides(tet_px, tet_py, tet_rot)) {
        memset(tet_board, 0, sizeof(tet_board));
        tet_message_timer = 80;
        tet_message_kind = 3;
    }
}

static void tet_update(const InputState *in, uint32_t frame)
{
    int period;
    if (tet_message_timer > 0) tet_message_timer--;

    if (in->pressed & IN_CIRCLE) tet_hold_piece();
    if (in->pressed & IN_X) {
        while (tet_try_move(0, 1)) tet_score += 2u;
        tet_lock();
        return;
    }

    if ((in->pressed & IN_LEFT) || ((in->held & IN_LEFT) && (frame % 6u) == 0u))
        tet_try_move(-1, 0);
    if ((in->pressed & IN_RIGHT) || ((in->held & IN_RIGHT) && (frame % 6u) == 0u))
        tet_try_move(1, 0);
    if (in->pressed & IN_UP) tet_rotate();
    if ((in->pressed & IN_DOWN) || ((in->held & IN_DOWN) && (frame % 3u) == 0u)) {
        if (tet_try_move(0, 1)) { tet_score++; tet_gravity = 0; }
    }

    tet_gravity++;
    period = clampi(25 - (int)(tet_level > 11u ? 10u : tet_level - 1u) * 2, 5, 25);
    if (tet_gravity >= period) {
        tet_gravity = 0;
        if (!tet_try_move(0, 1)) tet_lock();
    }
}

static void tet_draw_grid(void)
{
    int i;
    for (i = 0; i <= TET_W; i++) {
        float x = TET_LEFT + (float)i * TET_CELL;
        gfx3d_add_box(x, TET_TOP - TET_H * TET_CELL * 0.5f,
                      0.026f, TET_H * TET_CELL, 0.20f, 0.025f, C_PANEL_EDGE);
    }
    for (i = 0; i <= TET_H; i++) {
        float y = TET_TOP - (float)i * TET_CELL;
        gfx3d_add_box(TET_LEFT + TET_W * TET_CELL * 0.5f, y,
                      TET_W * TET_CELL, 0.026f, 0.20f, 0.025f, C_PANEL_EDGE);
    }
}

static void tet_draw(void)
{
    int r, c, i;
    int cells[4][2];

    gfx3d_add_box(-4.10f, -0.30f, 5.00f, 9.40f, 0.0f, 0.40f, C_PANEL_DARK);
    gfx3d_add_box(3.80f, 0.0f, 7.10f, 9.20f, 0.05f, 0.40f, C_PANEL);
    tet_draw_grid();

    for (r = 0; r < TET_H; r++) {
        for (c = 0; c < TET_W; c++) {
            if (tet_board[r][c]) {
                int idx = tet_board[r][c] - 1;
                gfx3d_add_box(tet_cx(c), tet_cy(r), 0.395f, 0.395f,
                              0.55f, 0.22f, TET_COLORS[idx]);
            }
        }
    }

    tet_get_cells(tet_type, tet_rot, cells);
    for (i = 0; i < 4; i++) {
        int bx = tet_px + cells[i][0];
        int by = tet_py + cells[i][1];
        if (by >= 0 && by < TET_H)
            gfx3d_add_box(tet_cx(bx), tet_cy(by), 0.395f, 0.395f,
                          0.82f, 0.24f, TET_COLORS[tet_type]);
    }

    font3d_draw("SCORE", 0.75f, 3.35f, 1.15f, 0.076f, C_MUTED);
    draw_number(tet_score, 6, 0.75f, 2.48f, 1.15f, 0.118f, C_WHITE);
    font3d_draw("LINES", 0.75f, 1.42f, 1.15f, 0.070f, C_MUTED);
    draw_number(tet_lines, 4, 0.75f, 0.67f, 1.15f, 0.105f, C_CYAN);
    font3d_draw("LEVEL", 4.45f, 1.42f, 1.15f, 0.070f, C_MUTED);
    draw_number(tet_level, 3, 4.45f, 0.67f, 1.15f, 0.105f, C_YELLOW);
    font3d_draw("NEXT", 0.75f, -0.42f, 1.15f, 0.075f, C_MUTED);
    font3d_draw("HOLD", 4.45f, -0.42f, 1.15f, 0.075f, C_MUTED);

    tet_get_cells(tet_next, 0, cells);
    for (i = 0; i < 4; i++) {
        float px = 1.86f + ((float)cells[i][0] - 1.5f) * 0.38f;
        float py = -1.44f - ((float)cells[i][1] - 1.5f) * 0.38f;
        gfx3d_add_box(px, py, 0.34f, 0.34f, 0.72f, 0.18f, TET_COLORS[tet_next]);
    }
    if (tet_hold >= 0) {
        tet_get_cells(tet_hold, 0, cells);
        for (i = 0; i < 4; i++) {
            float px = 5.55f + ((float)cells[i][0] - 1.5f) * 0.38f;
            float py = -1.44f - ((float)cells[i][1] - 1.5f) * 0.38f;
            GColor col = tet_hold_used ? gfx3d_shade(TET_COLORS[tet_hold], 0.48f) : TET_COLORS[tet_hold];
            gfx3d_add_box(px, py, 0.34f, 0.34f, 0.72f, 0.18f, col);
        }
    }

    font3d_draw("D-PAD MOVE   UP ROTATE", 0.75f, -2.80f, 1.15f, 0.048f, C_WHITE);
    font3d_draw("DOWN SOFT DROP", 0.75f, -3.36f, 1.15f, 0.053f, C_MUTED);
    font3d_draw("X HARD DROP", 0.75f, -3.92f, 1.15f, 0.055f, C_CYAN);
    font3d_draw("CIRCLE HOLD", 0.75f, -4.48f, 1.15f, 0.055f, C_YELLOW);

    draw_common_hud(GAME_TETRIS);
    if (tet_message_timer > 0) {
        const char *msg = tet_message_kind == 3 ? "STACK RESET!" :
                          (tet_message_kind == 2 ? "FOUR LINES!" : "LINE CLEAR!");
        draw_message(msg, tet_message_kind == 3 ? C_RED : C_GREEN);
    }
}

/* ------------------------------------------------------------------ */
/* Game 3: INVADERS 3D                                                 */
/* ------------------------------------------------------------------ */

#define INV_COLS 7
#define INV_ROWS 3
#define MAX_ENEMY_BULLETS 3

typedef struct {
    float x, y;
    int active;
} Bullet;

static uint8_t inv_alive[INV_ROWS][INV_COLS];
static float inv_offset_x, inv_offset_y, inv_dir;
static float inv_player_x, inv_bullet_x, inv_bullet_y;
static int inv_bullet_active, inv_fire_cooldown;
static Bullet inv_enemy_bullets[MAX_ENEMY_BULLETS];
static uint32_t inv_score, inv_wave;
static int inv_message_timer, inv_message_kind;

static const GColor INV_ROW_COLORS[INV_ROWS] = {
    {255, 82, 162, 0x80},
    {178, 92, 255, 0x80},
    {64, 232, 255, 0x80}
};

static float inv_cell_x(int col) { return -6.30f + (float)col * 2.10f + inv_offset_x; }
static float inv_cell_y(int row) { return 2.85f - (float)row * 1.14f + inv_offset_y; }

static void inv_clear_bullets(void)
{
    int i;
    inv_bullet_active = 0;
    for (i = 0; i < MAX_ENEMY_BULLETS; i++) inv_enemy_bullets[i].active = 0;
}

static void inv_reset_wave(int show_message)
{
    int r, c;
    for (r = 0; r < INV_ROWS; r++)
        for (c = 0; c < INV_COLS; c++) inv_alive[r][c] = 1;
    inv_offset_x = 0.0f;
    inv_offset_y = 0.0f;
    inv_dir = 1.0f;
    inv_player_x = 0.0f;
    inv_fire_cooldown = 0;
    inv_clear_bullets();
    if (show_message) {
        inv_message_timer = 85;
        inv_message_kind = 1;
    }
}

static void inv_reset(void)
{
    g_rng = 0x1BADB002u;
    inv_score = 0;
    inv_wave = 1;
    inv_message_timer = 0;
    inv_message_kind = 0;
    inv_reset_wave(0);
}

static int inv_count_alive(void)
{
    int r, c, n = 0;
    for (r = 0; r < INV_ROWS; r++)
        for (c = 0; c < INV_COLS; c++) if (inv_alive[r][c]) n++;
    return n;
}

static void inv_spawn_enemy_bullet(void)
{
    int alive[INV_ROWS * INV_COLS][2];
    int n = 0, r, c, i;
    for (r = 0; r < INV_ROWS; r++)
        for (c = 0; c < INV_COLS; c++)
            if (inv_alive[r][c]) { alive[n][0] = r; alive[n][1] = c; n++; }
    if (n == 0) return;
    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!inv_enemy_bullets[i].active) {
            int pick = (int)(rnd() % (uint32_t)n);
            int rr = alive[pick][0], cc = alive[pick][1];
            inv_enemy_bullets[i].x = inv_cell_x(cc);
            inv_enemy_bullets[i].y = inv_cell_y(rr) - 0.35f;
            inv_enemy_bullets[i].active = 1;
            return;
        }
    }
}

static void inv_update(const InputState *in, uint32_t frame)
{
    int i, r, c;
    int period;
    if (inv_message_timer > 0) inv_message_timer--;

    if (in->held & IN_LEFT)  inv_player_x -= 0.17f;
    if (in->held & IN_RIGHT) inv_player_x += 0.17f;
    inv_player_x = clampf(inv_player_x, -8.0f, 8.0f);

    if (inv_fire_cooldown > 0) inv_fire_cooldown--;
    if ((in->held & IN_UP) && !inv_bullet_active && inv_fire_cooldown == 0) {
        inv_bullet_active = 1;
        inv_bullet_x = inv_player_x;
        inv_bullet_y = -3.70f;
        inv_fire_cooldown = 11;
    }

    if (inv_bullet_active) {
        inv_bullet_y += 0.29f;
        if (inv_bullet_y > 4.85f) inv_bullet_active = 0;
        else {
            for (r = 0; r < INV_ROWS && inv_bullet_active; r++) {
                for (c = 0; c < INV_COLS && inv_bullet_active; c++) {
                    if (inv_alive[r][c] &&
                        fabsf(inv_bullet_x - inv_cell_x(c)) < 0.46f &&
                        fabsf(inv_bullet_y - inv_cell_y(r)) < 0.38f) {
                        inv_alive[r][c] = 0;
                        inv_bullet_active = 0;
                        inv_score += 100u * (uint32_t)(INV_ROWS - r);
                    }
                }
            }
        }
    }

    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        Bullet *b = &inv_enemy_bullets[i];
        if (!b->active) continue;
        b->y -= 0.13f;
        if (b->y < -4.65f) { b->active = 0; continue; }
        if (fabsf(b->x - inv_player_x) < 0.48f && fabsf(b->y + 4.18f) < 0.34f) {
            b->active = 0;
            inv_player_x = 0.0f;
            inv_clear_bullets();
            inv_message_timer = 55;
            inv_message_kind = 2;
        }
    }

    if (inv_count_alive() == 0) {
        inv_wave++;
        inv_score += 500;
        inv_reset_wave(1);
    }

    period = clampi(20 - (int)(inv_wave > 8u ? 7u : inv_wave - 1u) * 2, 6, 20);
    if (frame % (uint32_t)period == 0u) {
        inv_offset_x += inv_dir * 0.22f;
        if (inv_offset_x > 1.40f || inv_offset_x < -1.40f) {
            inv_offset_x = clampf(inv_offset_x, -1.40f, 1.40f);
            inv_dir = -inv_dir;
            inv_offset_y -= 0.27f;
        }
        if ((frame / (uint32_t)period) % 5u == 0u) inv_spawn_enemy_bullet();
        if (inv_offset_y < -1.80f) {
            inv_wave++;
            inv_reset_wave(1);
            inv_message_kind = 3;
        }
    }
}

static void inv_draw_voxel(float cx, float cy, float s, GColor color, float z)
{
    static const char *pattern[3] = {" X X ", "XXXXX", "X X X"};
    int r, c;
    for (r = 0; r < 3; r++) {
        for (c = 0; c < 5; c++) {
            if (pattern[r][c] == 'X') {
                float x = cx + ((float)c - 2.0f) * s;
                float y = cy + (1.0f - (float)r) * s;
                gfx3d_add_box(x, y, s * 0.86f, s * 0.86f, z, 0.14f, color);
            }
        }
    }
}

static void inv_draw_ship(float x)
{
    gfx3d_add_box(x, -4.20f, 1.08f, 0.28f, 0.82f, 0.24f, C_CYAN);
    gfx3d_add_box(x, -3.98f, 0.34f, 0.44f, 0.86f, 0.20f, C_WHITE);
    gfx3d_add_box(x - 0.48f, -4.23f, 0.26f, 0.20f, 0.80f, 0.16f, C_BLUE);
    gfx3d_add_box(x + 0.48f, -4.23f, 0.26f, 0.20f, 0.80f, 0.16f, C_BLUE);
}

static void inv_draw(void)
{
    int r, c, i;
    char b[16];
    gfx3d_add_box(0.0f, -0.18f, 18.70f, 10.30f, 0.0f, 0.40f, C_PANEL_DARK);

    /* Fixed 3D star cubes; these never move with a camera. */
    for (i = 0; i < 14; i++) {
        float sx = -8.2f + (float)((i * 37) % 17) * 1.02f;
        float sy = -4.2f + (float)((i * 53) % 9) * 1.05f;
        gfx3d_add_box(sx, sy, 0.045f, 0.045f, 0.12f, 0.03f, C_PANEL_EDGE);
    }

    font3d_draw("SCORE", -8.65f, 4.45f, 1.10f, 0.064f, C_MUTED);
    draw_number(inv_score, 6, -8.65f, 3.70f, 1.10f, 0.082f, C_WHITE);
    font3d_draw("WAVE", 5.55f, 4.45f, 1.10f, 0.064f, C_MUTED);
    draw_number(inv_wave, 4, 5.55f, 3.70f, 1.10f, 0.082f, C_YELLOW);
    font3d_uint(b, sizeof(b), inv_count_alive(), 2);
    font3d_draw_centered("TARGETS", 0.0f, 4.45f, 1.10f, 0.062f, C_MUTED);
    font3d_draw_centered(b, 0.0f, 3.70f, 1.10f, 0.082f, C_CYAN);

    for (r = 0; r < INV_ROWS; r++) {
        for (c = 0; c < INV_COLS; c++) {
            if (inv_alive[r][c])
                inv_draw_voxel(inv_cell_x(c), inv_cell_y(r), 0.175f,
                               INV_ROW_COLORS[r], 0.72f);
        }
    }

    inv_draw_ship(inv_player_x);
    if (inv_bullet_active)
        gfx3d_add_box(inv_bullet_x, inv_bullet_y, 0.12f, 0.34f, 1.00f, 0.10f, C_YELLOW);
    for (i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (inv_enemy_bullets[i].active)
            gfx3d_add_box(inv_enemy_bullets[i].x, inv_enemy_bullets[i].y,
                          0.11f, 0.30f, 0.98f, 0.09f, C_RED);
    }

    font3d_draw_centered("LEFT RIGHT MOVE   UP FIRE", 0.0f, -5.38f, 1.10f, 0.056f, C_MUTED);
    draw_common_hud(GAME_INVADER);
    if (inv_message_timer > 0)
        draw_message(inv_message_kind == 2 ? "HIT RESET!" :
                     (inv_message_kind == 3 ? "WAVE RESET!" : "WAVE CLEAR!"),
                     inv_message_kind == 1 ? C_GREEN : C_RED);
}

/* ------------------------------------------------------------------ */
/* Game 4: PONG 3D                                                     */
/* ------------------------------------------------------------------ */

static float pong_player_y, pong_ai_y;
static float pong_ball_x, pong_ball_y, pong_ball_vx, pong_ball_vy;
static uint32_t pong_player_score, pong_ai_score, pong_rally;
static int pong_message_timer, pong_message_kind;

static void pong_reset_ball(int direction)
{
    float total = (float)(pong_player_score + pong_ai_score);
    float speed = 0.135f + (total < 50.0f ? total * 0.0022f : 0.11f);
    float jitter = (float)(rnd() % 100u) / 100.0f - 0.5f;
    pong_ball_x = 0.0f;
    pong_ball_y = 0.0f;
    pong_ball_vx = (direction >= 0 ? speed : -speed);
    pong_ball_vy = jitter * speed * 1.35f;
    if (fabsf(pong_ball_vy) < 0.035f) pong_ball_vy = 0.045f;
    pong_rally = 0;
}

static void pong_reset(void)
{
    g_rng = 0x0F00DBABu;
    pong_player_y = 0.0f;
    pong_ai_y = 0.0f;
    pong_player_score = 0;
    pong_ai_score = 0;
    pong_message_timer = 0;
    pong_message_kind = 0;
    pong_reset_ball((rnd() & 1u) ? 1 : -1);
}

static void pong_bounce(float paddle_y)
{
    float max_vx = 0.285f;
    pong_ball_vx = -pong_ball_vx * 1.018f;
    if (pong_ball_vx > max_vx) pong_ball_vx = max_vx;
    if (pong_ball_vx < -max_vx) pong_ball_vx = -max_vx;
    pong_ball_vy += (pong_ball_y - paddle_y) * 0.022f;
    pong_ball_vy = clampf(pong_ball_vy, -0.22f, 0.22f);
    pong_rally++;
}

static void pong_update(const InputState *in, uint32_t frame)
{
    float ai_speed;
    (void)frame;
    if (pong_message_timer > 0) pong_message_timer--;

    if (in->held & IN_UP)   pong_player_y += 0.165f;
    if (in->held & IN_DOWN) pong_player_y -= 0.165f;
    pong_player_y = clampf(pong_player_y, -3.85f, 3.85f);

    ai_speed = 0.105f + ((pong_player_score + pong_ai_score) < 60u ?
                         (float)(pong_player_score + pong_ai_score) * 0.0009f : 0.054f);
    pong_ai_y += clampf(pong_ball_y - pong_ai_y, -ai_speed, ai_speed);
    pong_ai_y = clampf(pong_ai_y, -3.85f, 3.85f);

    pong_ball_x += pong_ball_vx;
    pong_ball_y += pong_ball_vy;
    if (pong_ball_y > 4.48f) { pong_ball_y = 4.48f; pong_ball_vy = -fabsf(pong_ball_vy); }
    if (pong_ball_y < -4.48f) { pong_ball_y = -4.48f; pong_ball_vy = fabsf(pong_ball_vy); }

    if (pong_ball_vx < 0.0f && pong_ball_x < -7.72f && pong_ball_x > -8.45f &&
        fabsf(pong_ball_y - pong_player_y) < 1.08f) {
        pong_ball_x = -7.70f;
        pong_bounce(pong_player_y);
    }
    if (pong_ball_vx > 0.0f && pong_ball_x > 7.72f && pong_ball_x < 8.45f &&
        fabsf(pong_ball_y - pong_ai_y) < 1.08f) {
        pong_ball_x = 7.70f;
        pong_bounce(pong_ai_y);
    }

    if (pong_ball_x < -9.15f) {
        pong_ai_score++;
        pong_message_timer = 65;
        pong_message_kind = 1;
        pong_reset_ball(1);
    } else if (pong_ball_x > 9.15f) {
        pong_player_score++;
        pong_message_timer = 65;
        pong_message_kind = 2;
        pong_reset_ball(-1);
    }
}

static void pong_draw(void)
{
    int i;
    gfx3d_add_box(0.0f, -0.10f, 18.80f, 10.20f, 0.0f, 0.40f, C_PANEL_DARK);
    gfx3d_add_box(0.0f, 4.68f, 18.10f, 0.11f, 0.30f, 0.08f, C_CYAN);
    gfx3d_add_box(0.0f, -4.68f, 18.10f, 0.11f, 0.30f, 0.08f, C_CYAN);
    for (i = 0; i < 9; i++)
        gfx3d_add_box(0.0f, -4.0f + (float)i, 0.075f, 0.54f, 0.24f, 0.05f, C_PANEL_EDGE);

    gfx3d_add_box(-8.00f, pong_player_y, 0.46f, 1.90f, 0.78f, 0.40f, C_CYAN);
    gfx3d_add_box(8.00f, pong_ai_y, 0.46f, 1.90f, 0.78f, 0.40f, C_PINK);
    gfx3d_add_sphere(pong_ball_x, pong_ball_y, 1.05f, 0.235f, C_YELLOW, 12, 7);

    font3d_draw("PLAYER", -8.65f, 4.05f, 1.10f, 0.066f, C_MUTED);
    draw_number(pong_player_score, 4, -8.65f, 3.10f, 1.10f, 0.115f, C_CYAN);
    font3d_draw("CPU", 5.35f, 4.05f, 1.10f, 0.066f, C_MUTED);
    draw_number(pong_ai_score, 4, 5.35f, 3.10f, 1.10f, 0.115f, C_PINK);
    font3d_draw_centered("RALLY", 0.0f, 4.05f, 1.10f, 0.066f, C_MUTED);
    {
        char b[16];
        font3d_uint(b, sizeof(b), pong_rally, 3);
        font3d_draw_centered(b, 0.0f, 3.10f, 1.10f, 0.115f, C_WHITE);
    }
    font3d_draw_centered("D-PAD UP DOWN MOVE", 0.0f, -5.40f, 1.10f, 0.056f, C_MUTED);

    draw_common_hud(GAME_PONG);
    if (pong_message_timer > 0)
        draw_message(pong_message_kind == 1 ? "POINT CPU" : "POINT PLAYER",
                     pong_message_kind == 1 ? C_RED : C_GREEN);
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

const char *games_name(int id)
{
    if (id < 0 || id >= GAME_COUNT) return "UNKNOWN";
    return FULL_NAMES[id];
}

const char *games_short_name(int id)
{
    if (id < 0 || id >= GAME_COUNT) return "UNKNOWN";
    return SHORT_NAMES[id];
}

void games_reset_all(void)
{
    int i;
    for (i = 0; i < GAME_COUNT; i++) games_reset(i);
}

void games_reset(int id)
{
    switch (id) {
        case GAME_PAC: pac_reset(); break;
        case GAME_TETRIS: tet_reset(); break;
        case GAME_INVADER: inv_reset(); break;
        case GAME_PONG: pong_reset(); break;
        default: break;
    }
}

void games_update(int id, const InputState *input, uint32_t frame)
{
    switch (id) {
        case GAME_PAC: pac_update(input, frame); break;
        case GAME_TETRIS: tet_update(input, frame); break;
        case GAME_INVADER: inv_update(input, frame); break;
        case GAME_PONG: pong_update(input, frame); break;
        default: break;
    }
}

void games_draw(int id, uint32_t frame)
{
    (void)frame;
    switch (id) {
        case GAME_PAC: pac_draw(); break;
        case GAME_TETRIS: tet_draw(); break;
        case GAME_INVADER: inv_draw(); break;
        case GAME_PONG: pong_draw(); break;
        default: break;
    }
}
