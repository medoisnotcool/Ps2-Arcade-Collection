#ifndef ARCADE_GAMES_H
#define ARCADE_GAMES_H

#include <stdint.h>
#include "input.h"

#define GAME_COUNT 4
#define GAME_PAC     0
#define GAME_TETRIS  1
#define GAME_INVADER 2
#define GAME_PONG    3

const char *games_name(int id);
const char *games_short_name(int id);
void games_reset_all(void);
void games_reset(int id);
void games_update(int id, const InputState *input, uint32_t frame);
void games_draw(int id, uint32_t frame);

#endif
