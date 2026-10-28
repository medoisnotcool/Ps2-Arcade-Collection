#include "font3d.h"
#include "gfx3d.h"

#include <math.h>
#include <string.h>

typedef struct {
    float x1, y1, x2, y2;
} Stroke;

typedef struct {
    const Stroke *strokes;
    unsigned char count;
} Glyph;

#define S(x1, y1, x2, y2) { (float)(x1), (float)(y1), (float)(x2), (float)(y2) }
#define DEF(name, ...) static const Stroke name[] = { __VA_ARGS__ }
#define G(ch, name) [ch] = { name, (unsigned char)(sizeof(name) / sizeof(name[0])) }

DEF(gl_A, S(0,0,0,6), S(0,6,1,7), S(1,7,4,7), S(4,7,5,6), S(5,6,5,0), S(0,3.5,5,3.5));
DEF(gl_B, S(0,0,0,7), S(0,7,4,7), S(4,7,5,6), S(5,6,5,4.2), S(5,4.2,4,3.5), S(4,3.5,0,3.5),
           S(0,0,4,0), S(4,0,5,0.8), S(5,0.8,5,2.8), S(5,2.8,4,3.5));
DEF(gl_C, S(5,6.2,4,7), S(4,7,1,7), S(1,7,0,6), S(0,6,0,1), S(0,1,1,0), S(1,0,4,0), S(4,0,5,1));
DEF(gl_D, S(0,0,0,7), S(0,7,3.8,7), S(3.8,7,5,5.8), S(5,5.8,5,1.2), S(5,1.2,3.8,0), S(3.8,0,0,0));
DEF(gl_E, S(5,7,0,7), S(0,7,0,0), S(0,0,5,0), S(0,3.5,4.2,3.5));
DEF(gl_F, S(0,0,0,7), S(0,7,5,7), S(0,3.5,4.2,3.5));
DEF(gl_G, S(5,6.2,4,7), S(4,7,1,7), S(1,7,0,6), S(0,6,0,1), S(0,1,1,0), S(1,0,4,0),
           S(4,0,5,1), S(5,1,5,3), S(5,3,3,3));
DEF(gl_H, S(0,0,0,7), S(5,0,5,7), S(0,3.5,5,3.5));
DEF(gl_I, S(1,7,4,7), S(2.5,7,2.5,0), S(1,0,4,0));
DEF(gl_J, S(0,1,1,0), S(1,0,4,0), S(4,0,5,1), S(5,1,5,7));
DEF(gl_K, S(0,0,0,7), S(5,7,0,3.2), S(0,3.2,5,0));
DEF(gl_L, S(0,7,0,0), S(0,0,5,0));
DEF(gl_M, S(0,0,0,7), S(0,7,2.5,4), S(2.5,4,5,7), S(5,7,5,0));
DEF(gl_N, S(0,0,0,7), S(0,7,5,0), S(5,0,5,7));
DEF(gl_O, S(1,7,4,7), S(4,7,5,6), S(5,6,5,1), S(5,1,4,0), S(4,0,1,0),
           S(1,0,0,1), S(0,1,0,6), S(0,6,1,7));
DEF(gl_P, S(0,0,0,7), S(0,7,4,7), S(4,7,5,6), S(5,6,5,4.2), S(5,4.2,4,3.5), S(4,3.5,0,3.5));
DEF(gl_Q, S(1,7,4,7), S(4,7,5,6), S(5,6,5,1), S(5,1,4,0), S(4,0,1,0),
           S(1,0,0,1), S(0,1,0,6), S(0,6,1,7), S(3.2,2,5.3,-0.5));
DEF(gl_R, S(0,0,0,7), S(0,7,4,7), S(4,7,5,6), S(5,6,5,4.2), S(5,4.2,4,3.5),
           S(4,3.5,0,3.5), S(3,3.5,5,0));
DEF(gl_S, S(5,6.2,4,7), S(4,7,1,7), S(1,7,0,6), S(0,6,0,4.2), S(0,4.2,1,3.5),
           S(1,3.5,4,3.5), S(4,3.5,5,2.8), S(5,2.8,5,1), S(5,1,4,0), S(4,0,1,0), S(1,0,0,0.8));
DEF(gl_T, S(0,7,5,7), S(2.5,7,2.5,0));
DEF(gl_U, S(0,7,0,1), S(0,1,1,0), S(1,0,4,0), S(4,0,5,1), S(5,1,5,7));
DEF(gl_V, S(0,7,2.5,0), S(2.5,0,5,7));
DEF(gl_W, S(0,7,1,0), S(1,0,2.5,4), S(2.5,4,4,0), S(4,0,5,7));
DEF(gl_X, S(0,7,5,0), S(5,7,0,0));
DEF(gl_Y, S(0,7,2.5,4), S(2.5,4,5,7), S(2.5,4,2.5,0));
DEF(gl_Z, S(0,7,5,7), S(5,7,0,0), S(0,0,5,0));

DEF(gl_0, S(1,7,4,7), S(4,7,5,6), S(5,6,5,1), S(5,1,4,0), S(4,0,1,0),
           S(1,0,0,1), S(0,1,0,6), S(0,6,1,7));
DEF(gl_1, S(1,5.5,2.5,7), S(2.5,7,2.5,0), S(1,0,4,0));
DEF(gl_2, S(0,6,1,7), S(1,7,4,7), S(4,7,5,6), S(5,6,5,4.5), S(5,4.5,0,0), S(0,0,5,0));
DEF(gl_3, S(0,6.5,1,7), S(1,7,4,7), S(4,7,5,6), S(5,6,5,4.2), S(5,4.2,4,3.5),
           S(4,3.5,2.6,3.5), S(4,3.5,5,2.8), S(5,2.8,5,1), S(5,1,4,0), S(4,0,1,0), S(1,0,0,1));
DEF(gl_4, S(4,0,4,7), S(4,7,0,2.5), S(0,2.5,5,2.5));
DEF(gl_5, S(5,7,0,7), S(0,7,0,3.8), S(0,3.8,4,3.8), S(4,3.8,5,3), S(5,3,5,1),
           S(5,1,4,0), S(4,0,1,0), S(1,0,0,1));
DEF(gl_6, S(5,6,4,7), S(4,7,1,7), S(1,7,0,6), S(0,6,0,1), S(0,1,1,0), S(1,0,4,0),
           S(4,0,5,1), S(5,1,5,2.8), S(5,2.8,4,3.6), S(4,3.6,0,3.6));
DEF(gl_7, S(0,7,5,7), S(5,7,1.8,0));
DEF(gl_8, S(1,7,4,7), S(4,7,5,6), S(5,6,5,4.2), S(5,4.2,4,3.5), S(4,3.5,1,3.5),
           S(1,3.5,0,2.8), S(0,2.8,0,1), S(0,1,1,0), S(1,0,4,0), S(4,0,5,1),
           S(5,1,5,2.8), S(5,2.8,4,3.5), S(0,4.2,1,3.5), S(0,6,0,4.2), S(0,6,1,7));
DEF(gl_9, S(5,3.4,4,3.4), S(4,3.4,1,3.4), S(1,3.4,0,4.2), S(0,4.2,0,6),
           S(0,6,1,7), S(1,7,4,7), S(4,7,5,6), S(5,6,5,1), S(5,1,4,0),
           S(4,0,1,0), S(1,0,0,1));

DEF(gl_MINUS, S(0.6,3.5,4.4,3.5));
DEF(gl_COLON, S(2,1.6,2,2.1), S(2,4.9,2,5.4));
DEF(gl_DOT,   S(2.2,0.2,2.2,0.8));
DEF(gl_BANG,  S(2.5,7,2.5,2), S(2.2,0.2,2.2,0.8));
DEF(gl_SLASH, S(0,0,5,7));
DEF(gl_PLUS,  S(0.5,3.5,4.5,3.5), S(2.5,1.5,2.5,5.5));

static const Glyph glyphs[128] = {
    ['A'] = { gl_A, (unsigned char)(sizeof(gl_A) / sizeof(gl_A[0])) },
    ['B'] = { gl_B, (unsigned char)(sizeof(gl_B) / sizeof(gl_B[0])) },
    ['C'] = { gl_C, (unsigned char)(sizeof(gl_C) / sizeof(gl_C[0])) },
    ['D'] = { gl_D, (unsigned char)(sizeof(gl_D) / sizeof(gl_D[0])) },
    ['E'] = { gl_E, (unsigned char)(sizeof(gl_E) / sizeof(gl_E[0])) },
    ['F'] = { gl_F, (unsigned char)(sizeof(gl_F) / sizeof(gl_F[0])) },
    ['G'] = { gl_G, (unsigned char)(sizeof(gl_G) / sizeof(gl_G[0])) },
    ['H'] = { gl_H, (unsigned char)(sizeof(gl_H) / sizeof(gl_H[0])) },
    ['I'] = { gl_I, (unsigned char)(sizeof(gl_I) / sizeof(gl_I[0])) },
    ['J'] = { gl_J, (unsigned char)(sizeof(gl_J) / sizeof(gl_J[0])) },
    ['K'] = { gl_K, (unsigned char)(sizeof(gl_K) / sizeof(gl_K[0])) },
    ['L'] = { gl_L, (unsigned char)(sizeof(gl_L) / sizeof(gl_L[0])) },
    ['M'] = { gl_M, (unsigned char)(sizeof(gl_M) / sizeof(gl_M[0])) },
    ['N'] = { gl_N, (unsigned char)(sizeof(gl_N) / sizeof(gl_N[0])) },
    ['O'] = { gl_O, (unsigned char)(sizeof(gl_O) / sizeof(gl_O[0])) },
    ['P'] = { gl_P, (unsigned char)(sizeof(gl_P) / sizeof(gl_P[0])) },
    ['Q'] = { gl_Q, (unsigned char)(sizeof(gl_Q) / sizeof(gl_Q[0])) },
    ['R'] = { gl_R, (unsigned char)(sizeof(gl_R) / sizeof(gl_R[0])) },
    ['S'] = { gl_S, (unsigned char)(sizeof(gl_S) / sizeof(gl_S[0])) },
    ['T'] = { gl_T, (unsigned char)(sizeof(gl_T) / sizeof(gl_T[0])) },
    ['U'] = { gl_U, (unsigned char)(sizeof(gl_U) / sizeof(gl_U[0])) },
    ['V'] = { gl_V, (unsigned char)(sizeof(gl_V) / sizeof(gl_V[0])) },
    ['W'] = { gl_W, (unsigned char)(sizeof(gl_W) / sizeof(gl_W[0])) },
    ['X'] = { gl_X, (unsigned char)(sizeof(gl_X) / sizeof(gl_X[0])) },
    ['Y'] = { gl_Y, (unsigned char)(sizeof(gl_Y) / sizeof(gl_Y[0])) },
    ['Z'] = { gl_Z, (unsigned char)(sizeof(gl_Z) / sizeof(gl_Z[0])) },
    ['0'] = { gl_0, (unsigned char)(sizeof(gl_0) / sizeof(gl_0[0])) },
    ['1'] = { gl_1, (unsigned char)(sizeof(gl_1) / sizeof(gl_1[0])) },
    ['2'] = { gl_2, (unsigned char)(sizeof(gl_2) / sizeof(gl_2[0])) },
    ['3'] = { gl_3, (unsigned char)(sizeof(gl_3) / sizeof(gl_3[0])) },
    ['4'] = { gl_4, (unsigned char)(sizeof(gl_4) / sizeof(gl_4[0])) },
    ['5'] = { gl_5, (unsigned char)(sizeof(gl_5) / sizeof(gl_5[0])) },
    ['6'] = { gl_6, (unsigned char)(sizeof(gl_6) / sizeof(gl_6[0])) },
    ['7'] = { gl_7, (unsigned char)(sizeof(gl_7) / sizeof(gl_7[0])) },
    ['8'] = { gl_8, (unsigned char)(sizeof(gl_8) / sizeof(gl_8[0])) },
    ['9'] = { gl_9, (unsigned char)(sizeof(gl_9) / sizeof(gl_9[0])) },
    ['-'] = { gl_MINUS, (unsigned char)(sizeof(gl_MINUS) / sizeof(gl_MINUS[0])) },
    [':'] = { gl_COLON, (unsigned char)(sizeof(gl_COLON) / sizeof(gl_COLON[0])) },
    ['.'] = { gl_DOT, (unsigned char)(sizeof(gl_DOT) / sizeof(gl_DOT[0])) },
    ['!'] = { gl_BANG, (unsigned char)(sizeof(gl_BANG) / sizeof(gl_BANG[0])) },
    ['/'] = { gl_SLASH, (unsigned char)(sizeof(gl_SLASH) / sizeof(gl_SLASH[0])) },
    ['+'] = { gl_PLUS, (unsigned char)(sizeof(gl_PLUS) / sizeof(gl_PLUS[0])) }
};

static unsigned char normalize_char(char c)
{
    unsigned char u = (unsigned char)c;
    if (u >= 'a' && u <= 'z') u = (unsigned char)(u - 'a' + 'A');
    if (u >= 128) return '?';
    return u;
}

static float char_width(unsigned char c, float size)
{
    if (c == ' ') return 3.4f * size;
    if (c == ':' || c == '.' || c == '!' || c == '-' || c == '?') return 4.2f * size;
    if (c == '/') return 5.2f * size;
    return 6.2f * size;
}

float font3d_width(const char *text, float size)
{
    float w = 0.0f;
    if (!text) return 0.0f;
    while (*text) {
        w += char_width(normalize_char(*text), size);
        text++;
    }
    if (w > 0.0f) w -= 1.2f * size;
    return w;
}

static GColor front_color(GColor base, float grid_y)
{
    float t = grid_y / 7.0f;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return gfx3d_shade(base, 0.82f + 0.30f * t);
}

static void draw_stroke(const Stroke *s, float ox, float oy, float z,
                        float size, float depth, GColor color)
{
    float x1 = ox + s->x1 * size;
    float y1 = oy + s->y1 * size;
    float x2 = ox + s->x2 * size;
    float y2 = oy + s->y2 * size;
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = sqrtf(dx * dx + dy * dy);
    float px, py, tx, ty, half;
    float sx, sy, zb, n_dot, t_dot;
    GVec3 f0, f1, f2, f3, b0, b1, b2, b3;
    GColor c0, c1, c2, c3;
    GColor side1, side2;

    if (len < 0.0001f) return;
    px = -dy / len;
    py = dx / len;
    tx = dx / len;
    ty = dy / len;
    half = 0.55f * size;
    sx = -depth * 0.42f;
    sy = -depth * 0.28f;
    zb = z - depth;
    /* Fixed oblique view direction. Only the two camera-facing walls of each
       extruded stroke need to be emitted. */
    n_dot = px * 0.42f + py * 0.28f;
    t_dot = tx * 0.42f + ty * 0.28f;

    f0 = gfx3d_vec(x1 + px * half, y1 + py * half, z);
    f1 = gfx3d_vec(x2 + px * half, y2 + py * half, z);
    f2 = gfx3d_vec(x2 - px * half, y2 - py * half, z);
    f3 = gfx3d_vec(x1 - px * half, y1 - py * half, z);
    b0 = gfx3d_vec(f0.x + sx, f0.y + sy, zb);
    b1 = gfx3d_vec(f1.x + sx, f1.y + sy, zb);
    b2 = gfx3d_vec(f2.x + sx, f2.y + sy, zb);
    b3 = gfx3d_vec(f3.x + sx, f3.y + sy, zb);

    c0 = front_color(color, s->y1);
    c1 = front_color(color, s->y2);
    c2 = c1;
    c3 = c0;
    side1 = gfx3d_shade(color, 0.28f);
    side2 = gfx3d_shade(color, 0.40f);

    /* Emit only the camera-facing extruded walls. The front face is on a
       higher z plane and wins the painter sort, leaving crisp 3D bevels. */
    if (n_dot > 0.0f)
        gfx3d_add_quad(b0, b1, f1, f0, side1, side1, side2, side2);
    if (t_dot > 0.0f)
        gfx3d_add_quad(b1, b2, f2, f1, side1, side1, side2, side2);
    if (-n_dot > 0.0f)
        gfx3d_add_quad(b2, b3, f3, f2, side1, side1, side2, side2);
    if (-t_dot > 0.0f)
        gfx3d_add_quad(b3, b0, f0, f3, side1, side1, side2, side2);
    gfx3d_add_quad(f0, f1, f2, f3, c0, c1, c2, c3);
}

void font3d_draw(const char *text, float x, float y, float z,
                 float size, GColor color)
{
    float depth = size * 1.55f;
    if (!text) return;
    while (*text) {
        unsigned char c = normalize_char(*text);
        const Glyph *g = &glyphs[c];
        unsigned i;
        for (i = 0; i < g->count; i++)
            draw_stroke(&g->strokes[i], x, y, z, size, depth, color);
        x += char_width(c, size);
        text++;
    }
}

void font3d_draw_centered(const char *text, float cx, float cy, float z,
                          float size, GColor color)
{
    float w = font3d_width(text, size);
    font3d_draw(text, cx - w * 0.5f, cy, z, size, color);
}

void font3d_draw_right(const char *text, float right, float cy, float z,
                       float size, GColor color)
{
    float w = font3d_width(text, size);
    font3d_draw(text, right - w, cy, z, size, color);
}

void font3d_uint(char *out, size_t capacity, uint32_t value, int digits)
{
    uint32_t mod = 1;
    int i;
    char tmp[16];
    int n = 0;
    if (!out || capacity == 0) return;
    if (digits < 1) digits = 1;
    if (digits > 10) digits = 10;
    if (digits < 10) {
        for (i = 0; i < digits; i++) mod *= 10u;
        value %= mod;
    }
    do {
        tmp[n++] = (char)('0' + (value % 10u));
        value /= 10u;
    } while (value > 0u && n < (int)sizeof(tmp));
    while (n < digits && n < (int)sizeof(tmp)) tmp[n++] = '0';
    if (n >= (int)capacity) n = (int)capacity - 1;
    for (i = 0; i < n; i++) out[i] = tmp[n - 1 - i];
    out[n] = '\0';
}
