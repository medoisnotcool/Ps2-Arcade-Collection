#include "app.h"
#include "games.h"
#include "font3d.h"
#include "gfx3d.h"
#include "color.h"

#include <math.h>

#define CARD_W 8.90f
#define CARD_H 4.05f

static int s_state = APP_MENU;
static int s_current_game = 0;
static int s_menu_sel = 0;
static int s_pause_sel = 0;
static uint32_t s_frame = 0;

static const float CARD_X[4] = {-5.05f, 5.05f, -5.05f, 5.05f};
static const float CARD_Y[4] = {1.72f, 1.72f, -3.18f, -3.18f};
static const char *CARD_NAMES[4] = {
    "PAC-MAN", "TETRIS", "SPACE INVADERS", "PONG"
};
static const char *CARD_NUMBERS[4] = {"01", "02", "03", "04"};

typedef struct {
    float x, y, phase;
} Star;

static const Star STARS[18] = {
    {-9.2f,  6.1f, 0.2f}, {-7.4f,  3.1f, 1.1f}, {-5.7f,  5.4f, 2.3f},
    {-3.1f,  6.3f, 0.8f}, {-1.2f,  4.6f, 3.0f}, { 1.4f,  6.2f, 1.7f},
    { 3.8f,  4.9f, 2.7f}, { 6.4f,  6.3f, 0.5f}, { 8.9f,  5.1f, 1.9f},
    {-9.0f, -2.4f, 2.1f}, {-6.3f, -5.8f, 0.7f}, {-3.8f, -3.9f, 2.9f},
    {-0.8f, -5.7f, 1.4f}, { 2.1f, -3.6f, 0.3f}, { 4.9f, -5.9f, 2.4f},
    { 7.5f, -2.9f, 1.2f}, { 9.2f, -5.4f, 2.8f}, { 0.3f, -1.2f, 0.9f}
};

static void draw_background(uint32_t frame)
{
    int i;
    gfx3d_add_gradient_quad(-10.0f, -7.0f, 10.0f, 7.0f,
                            -5.0f, C_BG2, C_BG);
    for (i = -10; i <= 10; i += 2) {
        gfx3d_add_box((float)i, 0.0f, 0.022f, 14.0f,
                      -4.86f, 0.018f, C_PANEL_EDGE);
    }
    for (i = -6; i <= 6; i += 2) {
        gfx3d_add_box(0.0f, (float)i, 20.0f, 0.022f,
                      -4.86f, 0.018f, C_PANEL_EDGE);
    }
    for (i = 0; i < 18; i++) {
        float pulse = 0.55f + 0.45f * sinf((float)frame * 0.035f + STARS[i].phase);
        GColor c = gfx3d_mix(C_PANEL_EDGE, C_CYAN, pulse * 0.65f);
        gfx3d_add_box(STARS[i].x, STARS[i].y, 0.055f, 0.055f,
                      -4.70f, 0.035f, c);
    }
}

static void draw_pac_icon(float cx, float cy, float z)
{
    GVec3 a, b, c;
    gfx3d_add_sphere(cx - 0.18f, cy, z, 0.62f, C_YELLOW, 14, 8);
    a = gfx3d_vec(cx - 0.18f, cy, z + 0.66f);
    b = gfx3d_vec(cx + 0.48f, cy + 0.36f, z + 0.66f);
    c = gfx3d_vec(cx + 0.48f, cy - 0.36f, z + 0.66f);
    gfx3d_add_triangle(a, b, c, C_BLACK, C_BLACK, C_BLACK);
    gfx3d_add_box(cx - 0.03f, cy + 0.34f, 0.10f, 0.10f,
                  z + 0.70f, 0.035f, C_BLACK);
    gfx3d_add_box(cx + 0.62f, cy, 0.13f, 0.13f, z + 0.18f, 0.08f, C_WHITE);
    gfx3d_add_box(cx + 0.98f, cy, 0.13f, 0.13f, z + 0.18f, 0.08f, C_WHITE);
    gfx3d_add_box(cx + 1.34f, cy, 0.13f, 0.13f, z + 0.18f, 0.08f, C_WHITE);
}

static void draw_tetris_icon(float cx, float cy, float z)
{
    static const float px[4] = {-0.62f, 0.0f, 0.62f, 0.0f};
    static const float py[4] = {0.30f, 0.30f, 0.30f, -0.33f};
    static const GColor cols[4] = {
        {64, 232, 255, 0x80}, {255, 220, 64, 0x80},
        {178, 92, 255, 0x80}, {76, 236, 132, 0x80}
    };
    int i;
    for (i = 0; i < 4; i++)
        gfx3d_add_box(cx + px[i], cy + py[i], 0.56f, 0.56f,
                      z + 0.22f, 0.30f, cols[i]);
}

static void draw_invader_icon(float cx, float cy, float z)
{
    static const char *pattern[3] = {" X X ", "XXXXX", "X X X"};
    int r, c;
    for (r = 0; r < 3; r++) {
        for (c = 0; c < 5; c++) {
            if (pattern[r][c] == 'X') {
                float x = cx + ((float)c - 2.0f) * 0.32f;
                float y = cy + (1.0f - (float)r) * 0.32f;
                GColor col = (r == 0) ? C_PINK : ((r == 1) ? C_PURPLE : C_CYAN);
                gfx3d_add_box(x, y, 0.275f, 0.275f, z + 0.22f, 0.18f, col);
            }
        }
    }
}

static void draw_pong_icon(float cx, float cy, float z)
{
    int i;
    gfx3d_add_box(cx - 1.02f, cy + 0.08f, 0.25f, 1.30f,
                  z + 0.25f, 0.22f, C_CYAN);
    gfx3d_add_box(cx + 1.02f, cy - 0.08f, 0.25f, 1.30f,
                  z + 0.25f, 0.22f, C_PINK);
    gfx3d_add_sphere(cx, cy + 0.30f, z + 0.38f, 0.22f, C_YELLOW, 10, 6);
    for (i = 0; i < 4; i++)
        gfx3d_add_box(cx, cy - 0.85f + (float)i * 0.42f,
                      0.055f, 0.20f, z + 0.10f, 0.04f, C_PANEL_EDGE);
}

static void draw_card_icon(int id, float cx, float cy, float z)
{
    switch (id) {
        case 0: draw_pac_icon(cx, cy, z); break;
        case 1: draw_tetris_icon(cx, cy, z); break;
        case 2: draw_invader_icon(cx, cy, z); break;
        case 3: draw_pong_icon(cx, cy, z); break;
        default: break;
    }
}

static void draw_menu(uint32_t frame)
{
    int i;
    float pulse = 0.5f + 0.5f * sinf((float)frame * 0.075f);
    GColor accent = gfx3d_mix(C_CYAN, C_PURPLE, pulse);

    font3d_draw_centered("3D ARCADE", 0.0f, 5.48f, 1.45f, 0.185f, C_WHITE);
    font3d_draw_centered("FOUR ENDLESS GAMES", 0.0f, 4.96f, 1.45f, 0.068f, accent);

    for (i = 0; i < 4; i++) {
        float cx = CARD_X[i], cy = CARD_Y[i];
        int selected = (i == s_menu_sel);
        GColor card_col = selected ? gfx3d_mix(C_PANEL, C_BLUE, 0.32f) : C_PANEL_DARK;
        GColor name_col = selected ? C_WHITE : C_MUTED;

        gfx3d_add_box(cx, cy, CARD_W, CARD_H, 0.30f, 0.48f, card_col);
        if (selected) {
            GColor border = gfx3d_mix(C_CYAN, C_PURPLE, pulse);
            gfx3d_add_box(cx, cy + CARD_H * 0.5f, CARD_W + 0.12f, 0.12f, 0.92f, 0.09f, border);
            gfx3d_add_box(cx, cy - CARD_H * 0.5f, CARD_W + 0.12f, 0.12f, 0.92f, 0.09f, border);
            gfx3d_add_box(cx - CARD_W * 0.5f, cy, 0.12f, CARD_H + 0.12f, 0.92f, 0.09f, border);
            gfx3d_add_box(cx + CARD_W * 0.5f, cy, 0.12f, CARD_H + 0.12f, 0.92f, 0.09f, border);
        }

        draw_card_icon(i, cx - 2.92f, cy + 0.08f, 0.86f);
        font3d_draw(CARD_NAMES[i], cx - 1.18f, cy + 0.24f, 1.10f,
                    i == 2 ? 0.060f : 0.095f, name_col);
        font3d_draw("ENDLESS", cx - 1.18f, cy - 0.54f, 1.10f, 0.062f,
                    selected ? C_CYAN : C_MUTED);
        font3d_draw("X PLAY", cx - 1.18f, cy - 1.20f, 1.10f, 0.056f,
                    selected ? C_YELLOW : C_MUTED);
        font3d_draw_right(CARD_NUMBERS[i], cx + 3.88f, cy + 1.32f, 1.10f, 0.075f,
                          selected ? C_YELLOW : C_PANEL_EDGE);
    }

    font3d_draw_centered("D-PAD CHOOSE   X PLAY", 0.0f, -6.28f, 1.45f, 0.075f, C_WHITE);
    font3d_draw_centered("FIXED 3D VIEW   VECTOR TEXT", 0.0f, -6.82f, 1.45f, 0.048f, C_MUTED);
}

static void draw_pause(uint32_t frame)
{
    int i;
    float pulse = 0.5f + 0.5f * sinf((float)frame * 0.10f);
    (void)pulse;

    gfx3d_add_flat_quad(-10.0f, -7.0f, 10.0f, 7.0f, 3.00f, C_BG);
    gfx3d_add_box(0.0f, 0.0f, 10.35f, 6.25f, 3.40f, 0.50f, C_PANEL);
    font3d_draw_centered("PAUSED", 0.0f, 1.73f, 4.05f, 0.145f, C_CYAN);
    font3d_draw_centered(games_name(s_current_game), 0.0f, 1.10f, 4.05f, 0.068f, C_WHITE);

    for (i = 0; i < 2; i++) {
        float y = 0.16f - (float)i * 1.08f;
        int selected = (i == s_pause_sel);
        GColor box_col = selected ? gfx3d_mix(C_BLUE, C_CYAN, 0.30f) : C_PANEL_DARK;
        GColor text_col = selected ? C_WHITE : C_MUTED;
        gfx3d_add_box(0.0f, y, 7.20f, 0.82f, 3.78f, 0.22f, box_col);
        font3d_draw_centered(i == 0 ? "RESUME" : "GAME SELECT",
                             0.0f, y - 0.27f, 4.12f, 0.108f, text_col);
    }
    font3d_draw_centered("D-PAD CHOOSE   X CONFIRM   START RESUME",
                         0.0f, -2.18f, 4.12f, 0.057f, C_MUTED);
}

static void update_menu(const InputState *in)
{
    if (in->pressed & IN_LEFT)  s_menu_sel &= ~1;
    if (in->pressed & IN_RIGHT) s_menu_sel |= 1;
    if (in->pressed & IN_UP)    s_menu_sel &= ~2;
    if (in->pressed & IN_DOWN)  s_menu_sel |= 2;
    s_menu_sel &= 3;
    if (in->pressed & IN_X) {
        s_current_game = s_menu_sel;
        games_reset(s_current_game);
        s_state = APP_GAME;
    }
}

static void update_pause(const InputState *in)
{
    if ((in->pressed & IN_UP) || (in->pressed & IN_DOWN))
        s_pause_sel ^= 1;
    if (in->pressed & IN_START) {
        s_state = APP_GAME;
    } else if (in->pressed & IN_X) {
        if (s_pause_sel == 0) s_state = APP_GAME;
        else s_state = APP_MENU;
    }
}

void app_init(GSGLOBAL *gs)
{
    gfx3d_init(gs);
    games_reset_all();
    s_state = APP_MENU;
    s_current_game = 0;
    s_menu_sel = 0;
    s_pause_sel = 0;
    s_frame = 0;
}

void app_frame(GSGLOBAL *gs, const InputState *input)
{
    InputState in;
    if (input) in = *input;
    else { in.held = 0; in.pressed = 0; }

    s_frame++;
    gfx3d_begin(C_BG);

    switch (s_state) {
        case APP_MENU:
            update_menu(&in);
            break;
        case APP_GAME:
            if (in.pressed & IN_START) {
                s_state = APP_PAUSE;
                s_pause_sel = 0;
            } else {
                games_update(s_current_game, &in, s_frame);
            }
            break;
        case APP_PAUSE:
            update_pause(&in);
            break;
        default:
            s_state = APP_MENU;
            break;
    }

    draw_background(s_frame);
    if (s_state == APP_MENU) {
        draw_menu(s_frame);
    } else {
        games_draw(s_current_game, s_frame);
        if (s_state == APP_PAUSE) draw_pause(s_frame);
    }
    gfx3d_present(gs);
}

int app_state(void) { return s_state; }
int app_current_game(void) { return s_current_game; }
int app_menu_selection(void) { return s_menu_sel; }
int app_pause_selection(void) { return s_pause_sel; }
uint32_t app_frame_number(void) { return s_frame; }

void app_debug_force_state(int state, int game)
{
    s_state = state;
    if (game >= 0 && game < GAME_COUNT) {
        s_current_game = game;
        s_menu_sel = game;
        games_reset(game);
    }
}
