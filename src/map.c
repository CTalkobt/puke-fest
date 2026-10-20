/* map.c — the current screen's map */
#include <dma.h>
#include "hw.h"
#include "tuning.h"
#include "map.h"
#include "video.h"

unsigned char map_w, map_h, map_tickets;
unsigned char map_sq[MAP_MAX];
unsigned char map_ox, map_oy;               /* where the map sits in the play area */

static void draw(unsigned char x, unsigned char y) {
    video_square(map_ox + x, map_oy + y, map_sq[y * map_w + x]);
}

void map_draw(void) {
    unsigned char x, y;
    for (y = 0; y < map_h; y++)
        for (x = 0; x < map_w; x++) draw(x, y);
}

unsigned char map_get(unsigned char x, unsigned char y) {
    return map_sq[y * map_w + x];
}

void map_set(unsigned char x, unsigned char y, unsigned char tile) {
    map_sq[y * map_w + x] = tile;
    draw(x, y);
}

unsigned char map_attr(unsigned char x, unsigned char y) {
    return tile_attr[map_sq[y * map_w + x]];
}

unsigned char map_solid(signed int px, signed int py) {
    if (px < 0 || py < 0) return 1;
    if ((unsigned int)px >= (unsigned int)map_w * SQ || (unsigned int)py >= (unsigned int)map_h * SQ) return 1;
    return map_attr((unsigned char)(px >> 5), (unsigned char)(py >> 5)) & A_SOLID;
}

unsigned char map_clear(signed int x, signed int y) {
    return !map_solid(x - BOX_L, y - BOX_T) && !map_solid(x + BOX_R, y - BOX_T) &&
           !map_solid(x - BOX_L, y + BOX_B) && !map_solid(x + BOX_R, y + BOX_B);
}

void map_open_gates(void) {
    unsigned char x, y;
    for (y = 0; y < map_h; y++)
        for (x = 0; x < map_w; x++)
            if (map_attr(x, y) & A_GATE) map_set(x, y, T_GATE_OPEN);
}

/* the sprite position of a map pixel */
unsigned int map_spr_x(signed int px) { return (unsigned int)(SPR_X0 + map_ox * SQ + px); }
unsigned int map_spr_y(signed int py) { return (unsigned int)(SPR_Y0 + map_oy * SQ + py); }
