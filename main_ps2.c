/* PlayStation 2 entry point for ARCADE3D.ELF. */
#include "app.h"
#include "input.h"

#include <stdint.h>
#include <tamtypes.h>

#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libpad.h>

#include <gsKit.h>
#include <dmaKit.h>
#include <gsInline.h>

static char pad_buffer[256] __attribute__((aligned(64)));

static void init_pad(void)
{
    sceSifInitRpc(0);
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:PADMAN", 0, NULL);
    padInit(0);
    padPortOpen(0, 0, pad_buffer);
}

static InputState read_pad(void)
{
    static uint32_t last_raw = 0;
    static uint32_t last_held = 0;
    struct padButtonStatus buttons;
    uint32_t raw, held = 0;
    InputState out;
    int state = padGetState(0, 0);

    if ((state == PAD_STATE_STABLE || state == PAD_STATE_FINDCTP1) &&
        padRead(0, 0, &buttons) != 0) {
        raw = 0xffffu ^ (uint32_t)buttons.btns;
        last_raw = raw;
    } else {
        raw = last_raw;
        if (state == PAD_STATE_DISCONN || state == PAD_STATE_ERROR) raw = 0;
    }

    if (raw & PAD_UP)    held |= IN_UP;
    if (raw & PAD_DOWN)  held |= IN_DOWN;
    if (raw & PAD_LEFT)  held |= IN_LEFT;
    if (raw & PAD_RIGHT) held |= IN_RIGHT;
    if (raw & PAD_CROSS) held |= IN_X;
    if (raw & PAD_CIRCLE) held |= IN_CIRCLE;
    if (raw & PAD_START) held |= IN_START;

    out.held = held;
    out.pressed = held & ~last_held;
    last_held = held;
    return out;
}

static GSGLOBAL *init_graphics(void)
{
    GSGLOBAL *gs = gsKit_init_global_custom(3 * 1024 * 1024, 256 * 1024);
    gs->PrimAlphaEnable = GS_SETTING_OFF;
    /* AA1 can force blending for triangles. Leave it off until a tested
       blending setup is implemented; sprites and 3D faces remain opaque. */
    gs->PrimAAEnable = GS_SETTING_OFF;
    gs->ZBuffering = GS_SETTING_OFF;
    gs->DoubleBuffering = GS_SETTING_ON;

    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
                D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);

    gsKit_set_clamp(gs, GS_CMODE_REPEAT);
    gsKit_vram_clear(gs);
    gsKit_init_screen(gs);
    gsKit_mode_switch(gs, GS_ONESHOT);
    gsKit_set_test(gs, GS_ZTEST_OFF);
    return gs;
}

int main(int argc, char **argv)
{
    GSGLOBAL *gs;
    (void)argc;
    (void)argv;

    init_pad();
    gs = init_graphics();
    app_init(gs);

    for (;;) {
        InputState in = read_pad();
        gsKit_clear(gs, GS_SETREG_RGBAQ(0x04, 0x06, 0x12, 0x80, 0x00));
        gsKit_set_test(gs, GS_ZTEST_OFF);
        app_frame(gs, &in);
        gsKit_queue_exec(gs);
        gsKit_sync_flip(gs);
    }

    return 0;
}
