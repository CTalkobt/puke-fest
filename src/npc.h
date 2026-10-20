/* npc.h — everyone else at the fair
 *
 * The crowd wanders, with their own nausea: nobody sees it, but they turn
 * green before they puke. Their pukes leave puddles and stink like the
 * player's (nausea_stink), and the stench makes the crowd sicker too: one
 * puke can set off more. The cotton-candy kid is one of the crowd, faster
 * and sicker. The Hugging Aunt never gets sick: she chases the player.
 */
#pragma once

#define NPC_CROWD 0                    /* assets: npc_type */
#define NPC_KID   1
#define NPC_AUNT  2

#define NE_PUKE   1                    /* npc_update: someone puked */
#define NE_HUG    2                    /* the aunt hugged the player */
#define NE_KID_PUKE 4                  /* the kid puked */
#define NE_CAUGHT 8                    /* ...into the player's bag */

extern unsigned char npc_count;

void npc_timing(unsigned char difficulty);   /* (each screen) */
void npc_reset(unsigned char level);   /* the level's people at their starting squares */
unsigned char npc_update(void);        /* once a frame: NE_ events */
void npc_draw(void);                   /* mux_add each (ids 1 on) */
void npc_smell(unsigned char x, unsigned char y);   /* a puke's stench at square (x, y) */
void npc_whiff(unsigned char x, unsigned char y, unsigned char radius);   /* a corn dog puff */
/* who (0-) has a body too close to feet at pixel (x, y); NPC_NONE: nobody (skip: leave out) */
#define NPC_NONE 0xFF
unsigned char npc_at(signed int x, signed int y, unsigned char skip);
/* the player walked into i: it makes way (turns away from him) */
void npc_make_way(unsigned char i);
/* is anyone within radius squares of square (x, y)? */
unsigned char npc_near(unsigned char x, unsigned char y, unsigned char radius);
