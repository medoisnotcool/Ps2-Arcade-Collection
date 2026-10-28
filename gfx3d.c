#include "gfx3d.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef HOST_SVG
#include <gsInline.h>
#endif

#ifndef PI
#define PI 3.14159265358979323846f
#endif

typedef struct {
    float sx[3], sy[3];
    int   iz[3];
    uint8_t r[3], g[3], b[3];
    float depth;
    int order;
} Face;

static Face g_faces[GFX3D_MAX_FACES];
static int g_face_count = 0;
static int g_order = 0;
static int g_overflow = 0;
static int g_width = 640;
static int g_height = 448;
static float g_scale = 32.0f;
static GColor g_bg = {4, 6, 18, 0x80};
static char g_svg_path[256] = "frame.svg";

#ifndef HOST_SVG
static GSPRIMPOINT g_points[GFX3D_BATCH_FACES * 3] __attribute__((aligned(128)));
#endif

static float clampf(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

GColor gfx3d_shade(GColor c, float factor)
{
    GColor out;
    out.r = (uint8_t)clampf((float)c.r * factor, 0.0f, 255.0f);
    out.g = (uint8_t)clampf((float)c.g * factor, 0.0f, 255.0f);
    out.b = (uint8_t)clampf((float)c.b * factor, 0.0f, 255.0f);
    out.a = c.a;
    return out;
}

GColor gfx3d_mix(GColor a, GColor b, float t)
{
    GColor out;
    t = clampf(t, 0.0f, 1.0f);
    out.r = (uint8_t)(a.r + (b.r - a.r) * t);
    out.g = (uint8_t)(a.g + (b.g - a.g) * t);
    out.b = (uint8_t)(a.b + (b.b - a.b) * t);
    out.a = a.a;
    return out;
}

GVec3 gfx3d_vec(float x, float y, float z)
{
    GVec3 v;
    v.x = x; v.y = y; v.z = z;
    return v;
}

void gfx3d_init(GSGLOBAL *gs)
{
    g_width = gs->Width;
    g_height = gs->Height;
    if (g_width <= 0) g_width = 640;
    if (g_height <= 0) g_height = 448;
    {
        float sx = (float)g_width / 20.0f;
        float sy = (float)g_height / 14.0f;
        g_scale = sx < sy ? sx : sy;
    }
    g_face_count = 0;
    g_order = 0;
    g_overflow = 0;
}

void gfx3d_set_svg_output(const char *path)
{
    if (!path) return;
    strncpy(g_svg_path, path, sizeof(g_svg_path) - 1);
    g_svg_path[sizeof(g_svg_path) - 1] = '\0';
}

void gfx3d_begin(GColor background)
{
    g_bg = background;
    g_face_count = 0;
    g_order = 0;
    g_overflow = 0;
}

int gfx3d_face_count(void) { return g_face_count; }
int gfx3d_overflow(void) { return g_overflow; }

void gfx3d_add_triangle(GVec3 a, GVec3 b, GVec3 c,
                        GColor ca, GColor cb, GColor cc)
{
    Face *f;
    if (g_face_count >= GFX3D_MAX_FACES) {
        g_overflow = 1;
        return;
    }
    f = &g_faces[g_face_count++];
    f->sx[0] = (float)g_width * 0.5f + a.x * g_scale;
    f->sy[0] = (float)g_height * 0.5f - a.y * g_scale;
    f->sx[1] = (float)g_width * 0.5f + b.x * g_scale;
    f->sy[1] = (float)g_height * 0.5f - b.y * g_scale;
    f->sx[2] = (float)g_width * 0.5f + c.x * g_scale;
    f->sy[2] = (float)g_height * 0.5f - c.y * g_scale;
    f->iz[0] = 0; f->iz[1] = 0; f->iz[2] = 0;
    f->r[0] = ca.r; f->g[0] = ca.g; f->b[0] = ca.b;
    f->r[1] = cb.r; f->g[1] = cb.g; f->b[1] = cb.b;
    f->r[2] = cc.r; f->g[2] = cc.g; f->b[2] = cc.b;
    f->depth = (a.z + b.z + c.z) / 3.0f;
    f->order = g_order++;
}

void gfx3d_add_quad(GVec3 a, GVec3 b, GVec3 c, GVec3 d,
                    GColor ca, GColor cb, GColor cc, GColor cd)
{
    gfx3d_add_triangle(a, b, c, ca, cb, cc);
    gfx3d_add_triangle(a, c, d, ca, cc, cd);
}

void gfx3d_add_flat_quad(float x0, float y0, float x1, float y1,
                         float z, GColor color)
{
    GVec3 a = gfx3d_vec(x0, y0, z);
    GVec3 b = gfx3d_vec(x1, y0, z);
    GVec3 c = gfx3d_vec(x1, y1, z);
    GVec3 d = gfx3d_vec(x0, y1, z);
    gfx3d_add_quad(a, b, c, d, color, color, color, color);
}

void gfx3d_add_gradient_quad(float x0, float y0, float x1, float y1,
                             float z, GColor top, GColor bottom)
{
    GVec3 a = gfx3d_vec(x0, y0, z);
    GVec3 b = gfx3d_vec(x1, y0, z);
    GVec3 c = gfx3d_vec(x1, y1, z);
    GVec3 d = gfx3d_vec(x0, y1, z);
    gfx3d_add_quad(a, b, c, d, bottom, bottom, top, top);
}

void gfx3d_add_box(float cx, float cy, float w, float h,
                   float z, float depth, GColor color)
{
    float hw = w * 0.5f;
    float hh = h * 0.5f;
    float zb = z - depth;
    float bx = -depth * 0.42f;
    float by = -depth * 0.28f;
    GVec3 f0, f1, f2, f3, b0, b1, b3;
    GColor front_lo, front_hi, side_l, side_b;

    f0 = gfx3d_vec(cx - hw, cy - hh, z);
    f1 = gfx3d_vec(cx + hw, cy - hh, z);
    f2 = gfx3d_vec(cx + hw, cy + hh, z);
    f3 = gfx3d_vec(cx - hw, cy + hh, z);
    b0 = gfx3d_vec(cx - hw + bx, cy - hh + by, zb);
    b1 = gfx3d_vec(cx + hw + bx, cy - hh + by, zb);
    b3 = gfx3d_vec(cx - hw + bx, cy + hh + by, zb);

    front_lo = gfx3d_shade(color, 0.86f);
    front_hi = gfx3d_shade(color, 1.16f);
    side_l = gfx3d_shade(color, 0.38f);
    side_b = gfx3d_shade(color, 0.26f);

    /* Front face, with a vertical lighting gradient. */
    gfx3d_add_quad(f0, f1, f2, f3,
                   front_lo, front_lo, front_hi, front_hi);
    /* Visible left and bottom sides. */
    gfx3d_add_quad(b0, f0, f3, b3,
                   side_l, side_l, gfx3d_shade(side_l, 1.25f), gfx3d_shade(side_l, 1.25f));
    gfx3d_add_quad(b0, b1, f1, f0,
                   side_b, side_b, side_l, side_l);
}

void gfx3d_add_sphere(float cx, float cy, float cz, float radius,
                      GColor color, int segments, int rings)
{
    enum { MAX_SEG = 20, MAX_RING = 16 };
    GVec3 v[(MAX_SEG + 1) * (MAX_RING + 1)];
    GColor vc[(MAX_SEG + 1) * (MAX_RING + 1)];
    float lx = -0.34f, ly = 0.48f, lz = 0.81f;
    int i, j;

    if (segments < 6) segments = 6;
    if (segments > MAX_SEG) segments = MAX_SEG;
    if (rings < 4) rings = 4;
    if (rings > MAX_RING) rings = MAX_RING;

    for (i = 0; i <= rings; i++) {
        float theta = PI * (float)i / (float)rings;
        for (j = 0; j <= segments; j++) {
            float phi = 2.0f * PI * (float)j / (float)segments;
            float nxv = sinf(theta) * cosf(phi);
            float nyv = cosf(theta);
            float nzv = sinf(theta) * sinf(phi);
            float lit = nxv * lx + nyv * ly + nzv * lz;
            int idx = i * (segments + 1) + j;
            if (lit < 0.0f) lit = 0.0f;
            v[idx] = gfx3d_vec(cx + radius * nxv,
                               cy + radius * nyv,
                               cz + radius * nzv);
            vc[idx] = gfx3d_shade(color, 0.42f + 0.78f * lit);
        }
    }

    for (i = 0; i < rings; i++) {
        for (j = 0; j < segments; j++) {
            int a = i * (segments + 1) + j;
            int b = a + 1;
            int c = a + (segments + 1);
            int d = c + 1;
            /* Average of the four vertex normals, used as the face normal. */
            float mnx = 0.25f * (v[a].x + v[b].x + v[c].x + v[d].x - 4.0f * cx) / radius;
            float mny = 0.25f * (v[a].y + v[b].y + v[c].y + v[d].y - 4.0f * cy) / radius;
            float mnz = 0.25f * (v[a].z + v[b].z + v[c].z + v[d].z - 4.0f * cz) / radius;
            float facing = -0.18f * mnx - 0.14f * mny + mnz;
            if (facing > -0.02f) {
                gfx3d_add_quad(v[a], v[b], v[d], v[c],
                               vc[a], vc[b], vc[d], vc[c]);
            }
        }
    }
}

static int face_compare(const void *pa, const void *pb)
{
    const Face *a = (const Face *)pa;
    const Face *b = (const Face *)pb;
    if (a->depth < b->depth) return -1;
    if (a->depth > b->depth) return 1;
    if (a->order < b->order) return -1;
    if (a->order > b->order) return 1;
    return 0;
}

#ifdef HOST_SVG
void gfx3d_present(GSGLOBAL *gs)
{
    FILE *fp;
    int i;
    (void)gs;
    qsort(g_faces, g_face_count, sizeof(Face), face_compare);
    fp = fopen(g_svg_path, "w");
    if (!fp) return;
    fprintf(fp, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 %d %d\">\n",
            g_width, g_height, g_width, g_height);
    fprintf(fp, "<rect width=\"%d\" height=\"%d\" fill=\"rgb(%u,%u,%u)\"/>\n",
            g_width, g_height, g_bg.r, g_bg.g, g_bg.b);
    fprintf(fp, "<g stroke-width=\"0.35\" shape-rendering=\"geometricPrecision\">\n");
    for (i = 0; i < g_face_count; i++) {
        const Face *f = &g_faces[i];
        unsigned r = (f->r[0] + f->r[1] + f->r[2]) / 3u;
        unsigned g = (f->g[0] + f->g[1] + f->g[2]) / 3u;
        unsigned b = (f->b[0] + f->b[1] + f->b[2]) / 3u;
        fprintf(fp,
                "<polygon points=\"%.2f,%.2f %.2f,%.2f %.2f,%.2f\" fill=\"rgb(%u,%u,%u)\" stroke=\"rgb(%u,%u,%u)\"/>\n",
                f->sx[0], f->sy[0], f->sx[1], f->sy[1], f->sx[2], f->sy[2],
                r, g, b, r, g, b);
    }
    fprintf(fp, "</g>\n</svg>\n");
    fclose(fp);
}
#else
void gfx3d_present(GSGLOBAL *gs)
{
    int start = 0;
    qsort(g_faces, g_face_count, sizeof(Face), face_compare);
    while (start < g_face_count) {
        int batch = g_face_count - start;
        int p = 0;
        int i;
        if (batch > GFX3D_BATCH_FACES) batch = GFX3D_BATCH_FACES;
        for (i = 0; i < batch; i++) {
            const Face *f = &g_faces[start + i];
            int k;
            for (k = 0; k < 3; k++) {
                g_points[p].rgbaq = color_to_RGBAQ(f->r[k], f->g[k], f->b[k], 0x80, 0.0f);
                g_points[p].xyz2 = vertex_to_XYZ2(gs, f->sx[k], f->sy[k], f->iz[k]);
                p++;
            }
        }
        gsKit_prim_list_triangle_gouraud_3d(gs, p, g_points);
        start += batch;
    }
}
#endif
