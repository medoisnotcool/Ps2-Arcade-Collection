/* Diagnostic only: distinguishes GS startup, simple triangles, list triangles,
   and controller module initialization. It does not contain copyrighted assets. */
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libpad.h>
#include <gsKit.h>
#include <dmaKit.h>
#include <gsInline.h>
#include <stdio.h>

static char pad_buffer[256] __attribute__((aligned(64)));
static GSPRIMPOINT pts[3] __attribute__((aligned(128)));
static GSGLOBAL *gs;

static void draw_test(int stage, unsigned int buttons)
{
    const u64 navy = GS_SETREG_RGBAQ(10, 15, 44, 128, 0);
    const u64 white = GS_SETREG_RGBAQ(255, 255, 255, 128, 0);
    const u64 red = GS_SETREG_RGBAQ(235, 45, 45, 128, 0);
    const u64 green = GS_SETREG_RGBAQ(40, 235, 90, 128, 0);
    const u64 yellow = GS_SETREG_RGBAQ(255, 225, 30, 128, 0);
    const u64 cyan = GS_SETREG_RGBAQ(40, 225, 255, 128, 0);
    int w = gs->Width, h = gs->Height;
    float sx = w / 640.0f, sy = h / 448.0f;

    gsKit_clear(gs, navy);
    gsKit_set_test(gs, GS_ZTEST_OFF);
    /* A thick white border and center marker prove basic sprites are visible. */
    gsKit_prim_sprite(gs, 40*sx, 40*sy, 600*sx, 50*sy, 1, white);
    gsKit_prim_sprite(gs, 40*sx, 398*sy, 600*sx, 408*sy, 1, white);
    gsKit_prim_sprite(gs, 40*sx, 40*sy, 50*sx, 408*sy, 1, white);
    gsKit_prim_sprite(gs, 590*sx, 40*sy, 600*sx, 408*sy, 1, white);
    /* Red = before pad init; green = padInit returned. White = X detected. */
    gsKit_prim_sprite(gs, 75*sx, 70*sy, 155*sx, 150*sy, 1,
                      (buttons & PAD_CROSS) ? white : (stage ? green : red));
    gsKit_prim_triangle_gouraud_3d(gs, 185*sx, 350*sy, 1,
                                    275*sx, 175*sy, 1,
                                    365*sx, 350*sy, 1,
                                    yellow, yellow, yellow);
    pts[0].rgbaq = color_to_RGBAQ(40,225,255,128,0.0f);
    pts[1].rgbaq = color_to_RGBAQ(40,225,255,128,0.0f);
    pts[2].rgbaq = color_to_RGBAQ(40,225,255,128,0.0f);
    pts[0].xyz2 = vertex_to_XYZ2(gs, 385*sx, 350*sy, 1);
    pts[1].xyz2 = vertex_to_XYZ2(gs, 475*sx, 175*sy, 1);
    pts[2].xyz2 = vertex_to_XYZ2(gs, 565*sx, 350*sy, 1);
    (void)cyan;
    gsKit_prim_list_triangle_gouraud_3d(gs, 3, pts);
    gsKit_queue_exec(gs);
    gsKit_sync_flip(gs);
}

int main(void)
{
    struct padButtonStatus pad;
    int m1, m2, init_result, open_result;
    int s;
    sceSifInitRpc(0);
    gs = gsKit_init_global_custom(3 * 1024 * 1024, 256 * 1024);
    gs->PrimAlphaEnable = GS_SETTING_OFF;
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
    draw_test(0, 0); draw_test(0, 0); /* both buffers contain stage 0 */
    printf("ARCADE DIAG: display initialized; loading controller modules\n");
    m1 = SifLoadModule("rom0:SIO2MAN", 0, NULL);
    m2 = SifLoadModule("rom0:PADMAN", 0, NULL);
    printf("ARCADE DIAG: module returns sio=%d pad=%d\n", m1, m2);
    init_result = padInit(0);
    open_result = padPortOpen(0, 0, pad_buffer);
    printf("ARCADE DIAG: padInit=%d padPortOpen=%d\n", init_result, open_result);
    for (;;) {
        unsigned int buttons = 0;
        s = padGetState(0, 0);
        if ((s == PAD_STATE_STABLE || s == PAD_STATE_FINDCTP1) && padRead(0, 0, &pad))
            buttons = (unsigned int)(0xffff ^ pad.btns);
        draw_test(1, buttons);
    }
}
