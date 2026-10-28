/* Desktop harness: renders the exact same scene graph to SVG and stress-tests
   the game state machines. This file is not compiled into the PS2 ELF. */
#include "app.h"
#include "games.h"
#include "gfx3d.h"
#include "input.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

static GSGLOBAL make_gs(void)
{
    GSGLOBAL gs;
    gs.Width = 640;
    gs.Height = 448;
    gs.OffsetX = 0;
    gs.OffsetY = 0;
    return gs;
}

static InputState input_none(void)
{
    InputState in;
    in.held = 0;
    in.pressed = 0;
    return in;
}

static InputState input_bits(uint32_t held, uint32_t pressed)
{
    InputState in;
    in.held = held;
    in.pressed = pressed;
    return in;
}

static void render_preview(GSGLOBAL *gs, const char *path, const InputState *in)
{
    gfx3d_set_svg_output(path);
    app_frame(gs, in);
    printf("%-34s faces=%4d overflow=%d state=%d game=%d\n",
           path, gfx3d_face_count(), gfx3d_overflow(),
           app_state(), app_current_game());
}

static uint32_t host_rand(uint32_t *s)
{
    uint32_t x = *s;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *s = x;
    return x;
}

static void stress_game(GSGLOBAL *gs, int game, int frames)
{
    uint32_t seed = 0xA5A50001u + (uint32_t)game * 7919u;
    int max_faces = 0, overflow = 0, i;
    app_debug_force_state(APP_GAME, game);
    gfx3d_set_svg_output("/dev/null");
    for (i = 0; i < frames; i++) {
        uint32_t r = host_rand(&seed);
        uint32_t held = 0;
        uint32_t pressed;
        static const uint32_t dirs[5] = {0, IN_UP, IN_DOWN, IN_LEFT, IN_RIGHT};
        held = dirs[r % 5u];
        if ((r >> 8) & 1u) held |= IN_X;
        pressed = held | (((r >> 16) & 1u) ? IN_START : 0u);
        {
            InputState in = input_bits(held, pressed);
            app_frame(gs, &in);
        }
        if (gfx3d_face_count() > max_faces) max_faces = gfx3d_face_count();
        if (gfx3d_overflow()) overflow = 1;
        /* START toggles the pause state; keep the stress run in-game. */
        if (app_state() == APP_PAUSE) {
            InputState resume = input_bits(IN_START, IN_START);
            app_frame(gs, &resume);
        }
        if (app_state() != APP_GAME) app_debug_force_state(APP_GAME, game);
    }
    printf("stress game %d: frames=%d max_faces=%d overflow=%d\n",
           game, frames, max_faces, overflow);
}

int main(void)
{
    GSGLOBAL gs = make_gs();
    InputState none = input_none();
    int i;

    app_init(&gs);

    render_preview(&gs, "previews/01-menu.svg", &none);

    app_debug_force_state(APP_GAME, 0);
    for (i = 0; i < 38; i++) {
        InputState in = input_bits(IN_RIGHT, (i == 0) ? IN_RIGHT : 0);
        gfx3d_set_svg_output("/dev/null");
        app_frame(&gs, &in);
    }
    render_preview(&gs, "previews/02-pac-maze.svg", &none);

    app_debug_force_state(APP_GAME, 1);
    for (i = 0; i < 120; i++) {
        uint32_t held = (i % 17 < 5) ? IN_DOWN : 0;
        uint32_t pressed = (i % 29 == 0) ? (IN_UP | held) : held;
        InputState in = input_bits(held, pressed);
        gfx3d_set_svg_output("/dev/null");
        app_frame(&gs, &in);
    }
    render_preview(&gs, "previews/03-blockfall.svg", &none);

    app_debug_force_state(APP_GAME, 2);
    for (i = 0; i < 45; i++) {
        InputState in = input_bits(IN_UP, (i == 0) ? IN_UP : 0);
        gfx3d_set_svg_output("/dev/null");
        app_frame(&gs, &in);
    }
    render_preview(&gs, "previews/04-invaders.svg", &none);

    app_debug_force_state(APP_GAME, 3);
    for (i = 0; i < 52; i++) {
        InputState in = input_bits((i % 20 < 10) ? IN_UP : IN_DOWN, 0);
        gfx3d_set_svg_output("/dev/null");
        app_frame(&gs, &in);
    }
    render_preview(&gs, "previews/05-pong.svg", &none);

    app_debug_force_state(APP_GAME, 0);
    {
        InputState start = input_bits(IN_START, IN_START);
        render_preview(&gs, "previews/06-pause.svg", &start);
    }

    for (i = 0; i < GAME_COUNT; i++) stress_game(&gs, i, 2200);

    return 0;
}
