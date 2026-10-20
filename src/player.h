/* player.h — the player: movement, collision, what he steps on */
#pragma once

/* player_update's events */
#define PE_TICKET 0x01
#define PE_MINT   0x02
#define PE_ALE    0x04
#define PE_PUDDLE 0x08
#define PE_EXIT   0x10
#define PE_BENCH  0x20               /* rested on a bench: +BENCH_GAIN */
#define PE_BUMP   0x40               /* walked into someone */
#define PE_BAG    0x80               /* picked up a paper bag */

extern unsigned char player_sx, player_sy;   /* the square he stands on */

#define BAG_NONE  0                  /* player_bag */
#define BAG_EMPTY 1
#define BAG_FULL  2
extern unsigned char player_bag;

void player_timing(void);
void player_reset(unsigned char sx, unsigned char sy);
unsigned char player_update(unsigned char controls);   /* once a frame: PE_ events */
void player_puke(unsigned char into_bag);   /* stand still and puke (the caller emits the event) */
void player_stun(unsigned char frames);   /* stand there, dizzy (a hug) */
unsigned char player_dizzy(void);
unsigned char player_bumped(void);   /* walked into someone this frame */
signed int player_x(void);           /* his feet, in pixels of the map */
signed int player_y(void);
void player_draw(unsigned char sick, unsigned char flash);
unsigned char player_puking(void);
