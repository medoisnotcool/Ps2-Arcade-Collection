#ifndef ARCADE_COLOR_H
#define ARCADE_COLOR_H

#include <stdint.h>

typedef struct {
    uint8_t r, g, b, a;
} GColor;

#define COL(r, g, b) ((GColor){ (uint8_t)(r), (uint8_t)(g), (uint8_t)(b), 0x80 })

/* Palette shared by all four games. */
#define C_BG            COL(4, 6, 18)
#define C_BG2           COL(8, 14, 34)
#define C_PANEL         COL(14, 24, 54)
#define C_PANEL_DARK    COL(8, 14, 36)
#define C_PANEL_EDGE    COL(28, 48, 96)
#define C_WHITE         COL(232, 244, 255)
#define C_MUTED         COL(120, 150, 196)
#define C_CYAN          COL(64, 232, 255)
#define C_BLUE          COL(64, 132, 255)
#define C_PURPLE        COL(178, 92, 255)
#define C_PINK          COL(255, 82, 162)
#define C_YELLOW        COL(255, 220, 64)
#define C_ORANGE        COL(255, 138, 48)
#define C_GREEN         COL(76, 236, 132)
#define C_RED           COL(255, 74, 98)
#define C_TEAL          COL(48, 214, 190)
#define C_BLACK         COL(2, 4, 12)

#endif
