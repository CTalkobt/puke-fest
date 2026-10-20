/* video.c — the display: VIC-IV 80 x 50 (640 x 400) with 16-bit characters
 *
 * Characters above $FF are full colour (FCM: 64 bytes, a byte per pixel,
 * the byte is the palette entry); the tiles are there, from TILE_CHAR on,
 * 16 characters to a 32 x 32 square (in rows). Characters below $100 come
 * from the normal character set and show their colour RAM colour: the HUD
 * row and messages.
 *
 * The screen (8000 bytes) is at the top of bank 5 and the art (loaded from
 * disk, load.s45) in bank 4 and on: bank 0 keeps the code. Cells are
 * written by DMA.
 *
 * Setting it up and back is in setup.c, the title's logo in front.c (the
 * overlays). The screen keeps the palette bank the ROM set up (its entries 0-15 are the
 * system colours; the tiles' are from PAL_FIRST); the sprites get the next
 * bank. $D070: MAPEDPAL (the bank at $D100-$D3FF) bits 7-6, BTPALSEL (the
 * screen's) 5-4, SPRPALSEL 3-2, ABTPALSEL 1-0.
 */
#include <dma.h>
#include "hw.h"
#include "tuning.h"
#include "assets.h"
#include "video.h"
#include "sfx.h"
#include "tune.h"

unsigned int spr_y0 = 112;                    /* hw.h: the play area's first line for sprites */
unsigned char v_pal_screen;                   /* $D070 for editing the screen's bank */
unsigned int v_textx;                         /* TEXTXPOS as the ROM set it */

/* the colour the screen turns toward as the player gets sicker */
#define SICK_R 150
#define SICK_G 200
#define SICK_B 60
static unsigned int blank_row[COLS];          /* a row of spaces */

#define PAL_R(i) REG(0xD100 + (i))
#define PAL_G(i) REG(0xD200 + (i))
#define PAL_B(i) REG(0xD300 + (i))

/* palette registers take their intensity nibble-swapped */
unsigned char video_nib(unsigned int v) {
    return (unsigned char)(v << 4 | v >> 4);
}

/* the screen's mood (level.c sets it): its palette effects */
unsigned char v_mood;
unsigned char v_fade = 16;                    /* the tiles' brightness, / 16 (video_fade) */
static unsigned char tint, phase;

/* palette entry e shows tile colour src: at night darker and bluer, then
 * as sick as the player (tint) */
static unsigned char shade(unsigned char c, unsigned char night_c, unsigned char sick_c) {
    if (v_fade != 16) c = (unsigned char)((c * v_fade) >> 4);
    if (v_mood == MOOD_NIGHT) c = (unsigned char)((c * (16 - NIGHT) + night_c * NIGHT) >> 4);
    return video_nib((c * (16 - tint) + sick_c * tint) >> 4);
}

static void put(unsigned char e, unsigned char src) {
    PAL_R(e) = shade(pal_r[src], 10, SICK_R);
    PAL_G(e) = shade(pal_g[src], 16, SICK_G);
    PAL_B(e) = shade(pal_b[src], 60, SICK_B);
}

void video_tint(unsigned char k) {
    unsigned char i;
    tint = k;
    REG(0xD070) = v_pal_screen;
    for (i = 0; i < NPAL; i++) put(PAL_FIRST + i, i);
}

/* the tiles fade to brightness to (/ 16) over a few frames; the music
 * and the effects go on meanwhile */
void video_fade(unsigned char to) {
    while (v_fade != to) {
        v_fade = to > v_fade ? v_fade + 2 : v_fade - 2;
        video_wait_frame();
        video_tint(tint);
        tune_update();
        sfx_update();
    }
}

/* once a frame: water shimmers (its 3 colours go round), the ride's
 * stripes swap, the mirrors' glass flashes now and then */
static const unsigned char water[3] = {PE_WATER0, PE_WATER1, PE_WATER2};

void video_mood(void) {
    static unsigned char tick;
    unsigned char k, flash;
    if (++tick < MOOD_RATE) return;
    tick = 0;
    phase++;
    REG(0xD070) = v_pal_screen;
    switch (v_mood) {
    case MOOD_WATER:
        for (k = 0; k < 3; k++) put(water[k], water[(k + phase) % 3] - PAL_FIRST);
        break;
    case MOOD_RIDE:
        put(PE_RIDE0, (phase & 1 ? PE_RIDE1 : PE_RIDE0) - PAL_FIRST);
        put(PE_RIDE1, (phase & 1 ? PE_RIDE0 : PE_RIDE1) - PAL_FIRST);
        break;
    case MOOD_GLASS:
        flash = (phase & 7) == 0;
        put(PE_GLASS0, (flash ? PE_GLASS1 : PE_GLASS0) - PAL_FIRST);
        break;
    }
}

void video_sway(signed char dx) {
    unsigned int x = v_textx + dx;
    REG(0xD04C) = (unsigned char)x;
    REG(0xD04D) = (REG(0xD04D) & 0xF0) | (unsigned char)((x >> 8) & 0x0F);
}

void video_clear(void) {
    unsigned char r;
    for (r = 0; r < COLS; r++) blank_row[r] = 0x20;
    for (r = 0; r < ROWS; r++)
        dma_copy_far(SCREEN_LIN + (unsigned int)r * COLS * 2, (unsigned long)(unsigned int)blank_row, COLS * 2);
}

void video_wait_frame(void) {
    unsigned char f = FRAMECOUNT;
    while (FRAMECOUNT == f) {}
}

void video_square(unsigned char x, unsigned char y, unsigned char tile) {
    unsigned int ch = TILE_CHAR + (unsigned int)tile * (SQ_CHARS * SQ_CHARS);
    unsigned int at = ((PLAY_ROW + y * SQ_CHARS) * COLS + x * SQ_CHARS) * 2, row[SQ_CHARS];
    unsigned char r, c;
    for (r = 0; r < SQ_CHARS; r++, at += COLS * 2) {
        for (c = 0; c < SQ_CHARS; c++) row[c] = ch++;
        dma_copy_far(SCREEN_LIN + at, (unsigned long)(unsigned int)row, sizeof row);
    }
}

/* a cell: a character number (a screen code, or above $FF full colour)
 * and, for a screen code, its colour */
void video_cell(unsigned int at, unsigned int ch, unsigned char colour) {
    dma_copy_far(SCREEN_LIN + at * 2, (unsigned long)(unsigned int)&ch, 2);
    dma_fill_far(COLOR_LIN + at * 2 + 1, 1, colour);
}

void video_text(unsigned char col, unsigned char row, const char *s, unsigned char colour) {
    unsigned int at = row * COLS + col;
    while (*s && col++ < COLS) video_cell(at++, (unsigned char)*s++, colour);
}

void video_page(unsigned int at, unsigned char row) {
    unsigned char head[4], text[COLS + 1];
    for (;;) {
        dma_copy_far((unsigned long)(unsigned int)head, DATA_LIN + at, 4);
        if (head[0] == 255) return;
        dma_copy_far((unsigned long)(unsigned int)text, DATA_LIN + at + 4, head[3]);
        text[head[3]] = 0;
        video_text(head[2], row ? row : head[0], (const char *)text, head[1]);
        at += 4 + head[3];
    }
}

void video_num(unsigned char col, unsigned char row, unsigned int v, unsigned char digits, unsigned char colour) {
    char buf[6];
    buf[digits] = 0;
    while (digits--) {
        buf[digits] = (char)(0x30 + v % 10);
        v /= 10;
    }
    video_text(col, row, buf, colour);
}

void video_fill(unsigned char col, unsigned char row, unsigned char n, unsigned char ch, unsigned char colour) {
    unsigned int at = row * COLS + col;
    while (n--) video_cell(at++, ch, colour);
}


/* the animated tiles (the puddles' stench): every ANIM_RATE frames, the
 * next image of each is copied over its characters, so every square
 * showing it moves at once */
void video_animate(void) {
    static unsigned char tick, step;
    unsigned char i;
    if (++tick < ANIM_RATE) return;
    tick = 0;
    step = (unsigned char)((step + 1) % ANIM_STEPS);
    for (i = 0; i < NANIM; i++)                   /* the top 8 characters change */
        dma_copy_far(TILE_LIN + (unsigned long)anim_tile[i] * (SQ * SQ),
                     ANIM_LIN + ((unsigned long)i * ANIM_STEPS + step) * (SQ * SQ / 2), SQ * SQ / 2);
}
