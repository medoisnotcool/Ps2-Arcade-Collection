#ifndef ARCADE_APP_H
#define ARCADE_APP_H

#include <stdint.h>
#include "gfx3d.h"
#include "input.h"

#define APP_MENU  0
#define APP_GAME  1
#define APP_PAUSE 2

void app_init(GSGLOBAL *gs);
void app_frame(GSGLOBAL *gs, const InputState *input);

/* Introspection helpers used by the desktop preview harness. */
int app_state(void);
int app_current_game(void);
int app_menu_selection(void);
int app_pause_selection(void);
uint32_t app_frame_number(void);
void app_debug_force_state(int state, int game);

#endif
