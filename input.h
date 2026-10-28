#ifndef ARCADE_INPUT_H
#define ARCADE_INPUT_H

#include <stdint.h>

#define IN_UP     0x0001u
#define IN_DOWN   0x0002u
#define IN_LEFT   0x0004u
#define IN_RIGHT  0x0008u
#define IN_X      0x0010u
#define IN_START  0x0020u
#define IN_CIRCLE 0x0040u

typedef struct {
    uint32_t held;
    uint32_t pressed;
} InputState;

#endif
