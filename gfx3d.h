#ifndef ARCADE_GFX3D_H
#define ARCADE_GFX3D_H

#include <stdint.h>
#include "color.h"

#ifdef HOST_SVG
typedef struct {
    int Width;
    int Height;
    int OffsetX;
    int OffsetY;
} GSGLOBAL;
#else
#include <gsKit.h>
#endif

typedef struct {
    float x, y, z;
} GVec3;

#define GFX3D_MAX_FACES 9000
#define GFX3D_BATCH_FACES 180

/* Fixed orthographic 3D stage: x = -10..10, y = -7..7, z = painter depth. */
void gfx3d_init(GSGLOBAL *gs);
void gfx3d_begin(GColor background);
void gfx3d_present(GSGLOBAL *gs);
void gfx3d_set_svg_output(const char *path);
int  gfx3d_face_count(void);
int  gfx3d_overflow(void);

GColor gfx3d_shade(GColor c, float factor);
GColor gfx3d_mix(GColor a, GColor b, float t);
GVec3  gfx3d_vec(float x, float y, float z);

void gfx3d_add_triangle(GVec3 a, GVec3 b, GVec3 c,
                        GColor ca, GColor cb, GColor cc);
void gfx3d_add_quad(GVec3 a, GVec3 b, GVec3 c, GVec3 d,
                    GColor ca, GColor cb, GColor cc, GColor cd);
void gfx3d_add_flat_quad(float x0, float y0, float x1, float y1,
                         float z, GColor color);
void gfx3d_add_gradient_quad(float x0, float y0, float x1, float y1,
                             float z, GColor top, GColor bottom);

/* A solid cuboid whose visible front face is located at (cx,cy,z).
   The back face is offset to the lower-left to reveal two shaded sides. */
void gfx3d_add_box(float cx, float cy, float w, float h,
                   float z, float depth, GColor color);

/* Low-poly shaded sphere, with its front hemisphere at positive z. */
void gfx3d_add_sphere(float cx, float cy, float cz, float radius,
                      GColor color, int segments, int rings);

#endif
