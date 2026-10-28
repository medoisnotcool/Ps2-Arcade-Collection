/* Host-side deterministic gameplay and menu regression checks.
   Include games.c to exercise its internal state transitions directly. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../games.c"
#include "../app.h"

static GSGLOBAL gs = {640, 448, 0, 0};
static void frame(uint32_t held, uint32_t pressed) {
    InputState in = { held, pressed };
    app_frame(&gs, &in);
    assert(!gfx3d_overflow());
}

static void check_menu_pause(void) {
    int game;
    gfx3d_set_svg_output("/dev/null");
    app_init(&gs);
    assert(app_state() == APP_MENU && app_menu_selection() == 0);
    frame(IN_RIGHT, IN_RIGHT);
    assert(app_menu_selection() == 1);
    frame(IN_DOWN, IN_DOWN);
    assert(app_menu_selection() == 3);
    frame(IN_LEFT, IN_LEFT);
    assert(app_menu_selection() == 2);
    frame(IN_UP, IN_UP);
    assert(app_menu_selection() == 0);
    for (game = 0; game < GAME_COUNT; game++) {
        app_debug_force_state(APP_GAME, game);
        frame(IN_START, IN_START);
        assert(app_state() == APP_PAUSE && app_pause_selection() == 0);
        frame(IN_START, IN_START);
        assert(app_state() == APP_GAME);
        frame(IN_START, IN_START);
        frame(IN_DOWN, IN_DOWN);
        assert(app_pause_selection() == 1);
        frame(IN_X, IN_X);
        assert(app_state() == APP_MENU);
        frame(IN_X, IN_X);
        assert(app_state() == APP_GAME && app_current_game() == game);
    }
}

static void check_pac(void) {
    int x, y, power = 0;
    InputState in = {0, 0};
    pac_reset();
    for (y = 0; y < PAC_H; y++) for (x = 0; x < PAC_W; x++) {
        if (pac_pellet[y][x] == 2) power++;
        assert(!pac_pellet[y][x] || !pac_wall(x, y));
    }
    assert(power == 4);
    pac_level = 1;
    pac_score = 0;
    pac_pellets_left = 1;
    memset(pac_pellet, 0, sizeof(pac_pellet));
    pac_x = 1; pac_y = 3; pac_dirx = 1; pac_diry = 0;
    pac_nextx = 1; pac_nexty = 0; pac_tick = 4;
    pac_pellet[3][2] = 1;
    pac_update(&in, 1);
    assert(pac_level == 2 && pac_pellets_left > 1 && pac_score == 1010);
    pac_reset();
    pac_x = 2; pac_y = 3; pac_dirx = -1; pac_nextx = -1;
    pac_diry = pac_nexty = 0; pac_tick = 4;
    pac_update(&in, 1);
    assert(pac_power == 400 && pac_score == 50);
    pac_grace = 0;
    pac_ghosts[0].x = pac_x; pac_ghosts[0].y = pac_y;
    pac_tick = 1;
    pac_update(&in, 1);
    assert(pac_score == 250 && pac_ghosts[0].x == 1 && pac_ghosts[0].y == 1);
}

static void check_tetris(void) {
    int x;
    tet_reset();
    for (x = 0; x < TET_W; x++) tet_board[TET_H - 1][x] = 1;
    tet_board[TET_H - 2][2] = 4;
    tet_clear_lines();
    assert(tet_lines == 1 && tet_level == 1 && tet_score == 100);
    assert(tet_board[TET_H - 1][2] == 4);
    tet_type = 1; tet_px = 3; tet_py = -2; tet_rot = 0;
    tet_lock();
    assert(tet_message_kind == 3);
    for (x = 0; x < TET_W; x++) assert(!tet_board[TET_H - 1][x]);
    tet_lines = 9; tet_level = 1;
    for (x = 0; x < TET_W; x++) tet_board[TET_H - 1][x] = 1;
    tet_clear_lines();
    assert(tet_lines == 10 && tet_level == 2);
    {
        InputState hold = {IN_CIRCLE, IN_CIRCLE};
        InputState drop = {IN_X, IN_X};
        int first, next, after_hold, old_hold;
        tet_reset();
        first = tet_type; next = tet_next;
        tet_update(&hold, 1);
        assert(tet_hold == first && tet_type == next && tet_hold_used == 1);
        after_hold = tet_type; old_hold = tet_hold;
        tet_update(&hold, 2);
        assert(tet_type == after_hold && tet_hold == old_hold);
        tet_update(&drop, 3);
        assert(tet_hold_used == 0 && tet_score > 0);
        for (x = 0; x < TET_W && !tet_board[TET_H - 1][x]; x++) {}
        assert(x < TET_W); /* hard drop landed and locked */
        after_hold = tet_type; old_hold = tet_hold;
        tet_update(&hold, 4);
        assert(tet_type == old_hold && tet_hold == after_hold && tet_hold_used == 1);
    }
}

static void check_invaders(void) {
    InputState in = {0, 0};
    inv_reset();
    assert(inv_count_alive() == INV_COLS * INV_ROWS);
    memset(inv_alive, 0, sizeof(inv_alive));
    inv_update(&in, 1);
    assert(inv_wave == 2 && inv_count_alive() == INV_COLS * INV_ROWS);
    assert(inv_score == 500);
    inv_offset_y = -1.81f;
    inv_update(&in, 18);
    assert(inv_wave == 3 && inv_message_kind == 3);
}

static void check_pong(void) {
    InputState in = {0, 0};
    pong_reset();
    pong_ball_x = -9.1f; pong_ball_vx = -0.2f; pong_ball_y = 3.0f;
    pong_update(&in, 1);
    assert(pong_ai_score == 1 && pong_player_score == 0);
    pong_ball_x = 9.1f; pong_ball_vx = 0.2f; pong_ball_y = 3.0f;
    pong_update(&in, 1);
    assert(pong_player_score == 1 && pong_ai_score == 1);
    pong_ball_x = -7.65f; pong_ball_y = pong_player_y;
    pong_ball_vx = -0.15f; pong_ball_vy = 0.0f;
    pong_update(&in, 1);
    assert(pong_ball_vx > 0 && pong_rally == 1);
}

int main(void) {
    gfx3d_init(&gs);
    check_menu_pause(); check_pac(); check_tetris(); check_invaders(); check_pong();
    puts("PASS: menu selection, pause/resume/return for all games");
    puts("PASS: Pac-Man pellets, power and advancing levels");
    puts("PASS: Tetris hard drop, hold/swap, line clearing and stack reset");
    puts("PASS: Space Invaders wave progression and reset");
    puts("PASS: Pong scoring and paddle bounce");
    return 0;
}
