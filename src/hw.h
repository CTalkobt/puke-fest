/* hw.h — registers and the memory map
 *
 * cc45's plain char/int/long are unsigned: signed values say so.
 */
#pragma once

#define REG(a) (*(volatile unsigned char *)(a))

#define FRAMECOUNT REG(0xD7FA)       /* video frames elapsed (wraps) */

#define COLOR_LIN  0xFF80000UL       /* colour RAM: 2 bytes a cell */
#define RTC_LIN    0xFFD7110UL       /* the real-time clock: BCD seconds, minutes, hours, day, month, year */
#define GFX_LIN    0x0040000UL       /* pukegfx1-2.bin from disk (banks 4-5): see assets.h */
#define TILE_LIN   (GFX_LIN + GFX_TILES)
#define TILE_CHAR  (unsigned int)(TILE_LIN / 64)
#define ANIM_LIN   (GFX_LIN + GFX_ANIM)
#define SPR_LIN    (GFX_LIN + GFX_SPRITES)
#define DATA_LIN   0x0012000UL       /* pukegfx3.bin (bank 1, free while the game runs): see assets.h */
#define DATA_SPR_LIN (DATA_LIN + DATA_SPRITES)
#define SCREEN_LIN 0x005E000UL       /* the screen: 80 x 50 16-bit cells (top of bank 5; the art ends below) */

#define COLS       80
#define ROWS       50
#define HUD_ROW    0
#define PLAY_ROW   1                 /* the play area: rows 1-48 */
#define SQ         32                /* pixels a square */
#define SQ_CHARS   4                 /* characters a square, each way */
#define PLAY_W     20                /* squares */
#define PLAY_H     12

/* sprites use physical pixels (SPR640, SPRENV400): the play area's top
 * left pixel. Sprite Y is a raster line in both PAL and NTSC, but the text
 * area starts on another (TEXTYPOS: 104 PAL, 55 NTSC): video_on sets
 * spr_y0 from it. */
#define SPR_X0     80                /* measured: X 80 is the display's first pixel */
#define SPR_Y0     spr_y0
#define SPR_ROW(r) (spr_y0 + ((r) - PLAY_ROW) * 8)   /* text row r's first line */
extern unsigned int spr_y0;

/* system colours (text) */
#define C_BLACK    0
#define C_WHITE    1
#define C_RED      2
#define C_CYAN     3
#define C_GREEN    5
#define C_YELLOW   7
#define C_ORANGE   8
#define C_LRED     10
#define C_GREY     12
#define C_LGREEN   13
