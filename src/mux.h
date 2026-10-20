/* mux.h — the sprite multiplexer: more characters than sprite pairs
 *
 * Each frame: mux_begin, mux_add for every character to show, mux_commit.
 * Characters are 32 x 42 pictures (two sprites); four can share a raster
 * line. When more crowd together, the ones left out take turns (they
 * flicker) - the player (id 0) is always shown.
 */
#pragma once

#define MUX_IDS 16                   /* character ids 0-15 (for taking turns) */

void mux_on(void);                   /* install the raster interrupt (after spr_on) */
void mux_off(void);
void mux_begin(void);
/* character id shows picture frame with its top left at physical (x, y) */
void mux_add(unsigned char id, unsigned char frame, unsigned int x, unsigned int y);
void mux_commit(void);               /* the display list for the next frame */
void mux_hide(void);                 /* nothing shown from the next frame on */
extern unsigned char mux_dropped;    /* characters left out of the last list */
extern signed char mux_sway;         /* added to every X (the screen's sway) */
