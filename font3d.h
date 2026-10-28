#ifndef ARCADE_FONT3D_H
#define ARCADE_FONT3D_H

#include <stddef.h>
#include <stdint.h>
#include "color.h"

/* All screen text is drawn from extruded vector strokes, never a bitmap font. */
float font3d_width(const char *text, float size);
void font3d_draw(const char *text, float x, float y, float z,
                 float size, GColor color);
void font3d_draw_centered(const char *text, float cx, float cy, float z,
                          float size, GColor color);
void font3d_draw_right(const char *text, float right, float cy, float z,
                       float size, GColor color);
void font3d_uint(char *out, size_t capacity, uint32_t value, int digits);

#endif
