/* map.h — the current screen's map: a tile per 16 x 16 square
 *
 * Maps carry their size; for now they fit the play area (20 x 12) and
 * smaller ones are centred. Pixel coordinates are the map's own: SQ (32) a
 * square.
 */
#pragma once
#include "hw.h"
#include "assets.h"

#define MAP_MAX (PLAY_W * PLAY_H)

extern unsigned char map_w, map_h;
extern unsigned char map_tickets;          /* tickets still on the map */
extern unsigned char map_sq[MAP_MAX];       /* the squares (level.c sets them up) */
extern unsigned char map_ox, map_oy;        /* where the map sits in the play area */

void map_load(unsigned char level);        /* and draw it */
void map_draw(void);                       /* all of it again */
unsigned char map_get(unsigned char x, unsigned char y);
void map_set(unsigned char x, unsigned char y, unsigned char tile);  /* and draw it */
unsigned char map_attr(unsigned char x, unsigned char y);
unsigned char map_solid(signed int px, signed int py);  /* a pixel; outside counts */
/* is the box around feet at pixel (x, y) (tuning.h BOX_) clear of solid squares? */
unsigned char map_clear(signed int x, signed int y);
void map_open_gates(void);
unsigned int map_spr_x(signed int px);     /* sprite coordinates of map pixels */
unsigned int map_spr_y(signed int py);
