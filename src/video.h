/* video.h — the screen: 80 x 50, FCM tiles in the play area, text in the HUD row */
#pragma once

/* load.s45: the art from disk (1 if it is all there); text through the KERNAL */
unsigned char gfx_load(void);                /* files gfx_first .. gfx_last - 1 (0-2: PFGFX1-3) */
extern unsigned char gfx_first, gfx_last;
extern const char *k_text;                 /* PETSCII, ending in 0 */
void k_print(void);
void video_on(void);
void video_off(void);
void video_clear(void);                    /* every cell a space */
void video_wait_frame(void);               /* until the next video frame */

/* a square of the play area (0-19, 0-11) shows a tile (4 x 4 characters) */
void video_square(unsigned char x, unsigned char y, unsigned char tile);

/* text (screen codes) at a cell, in a system colour */
void video_text(unsigned char col, unsigned char row, const char *s, unsigned char colour);
/* a text page from the data (assets/text.txt, assets.h PAGE_); row: all on
 * that row instead of their own (0: their own) */
void video_page(unsigned int at, unsigned char row);
void video_num(unsigned char col, unsigned char row, unsigned int v, unsigned char digits, unsigned char colour);
void video_fill(unsigned char col, unsigned char row, unsigned char n, unsigned char ch, unsigned char colour);

void video_animate(void);                  /* once a frame: animated tiles */

/* the screen's tiles turn toward sick green by k / 16 (0-16) */
void video_tint(unsigned char k);
/* the screen's mood: palette effects (video_mood once a frame) */
#define MOOD_NONE  0
#define MOOD_WATER 1                 /* the water shimmers */
#define MOOD_RIDE  2                 /* the ride's stripes spin */
#define MOOD_GLASS 3                 /* the mirrors flash */
#define MOOD_NIGHT 4                 /* darker and bluer */
extern unsigned char v_mood;
void video_mood(void);
/* the tiles fade to brightness to / 16 (0 black, 16 as they are) */
extern unsigned char v_fade;
void video_fade(unsigned char to);
/* the whole screen (text area) moved dx pixels across */
void video_sway(signed char dx);
/* the title logo with its top left at a cell */
void video_logo(unsigned char col, unsigned char row);
void video_logo_animate(void);             /* once a frame on the title: the slime runs */

/* shared with setup.c and front.c */
extern unsigned char v_pal_screen;
extern unsigned int v_textx;
unsigned char video_nib(unsigned int v);   /* a palette register's value */
/* a cell (row * COLS + col): a character number, its colour */
void video_cell(unsigned int at, unsigned int ch, unsigned char colour);
