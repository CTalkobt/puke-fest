/* nausea.h — the nausea rules
 *
 * Every change goes through nausea_event or nausea_stink (or nausea_add,
 * for the player himself): new hazards and characters only emit events.
 */
#pragma once

#define NZ_WARN    1                 /* nausea_tick: the warning before a puke starts */
#define NZ_PUKE    2                 /* the player pukes now */
#define NZ_RECOVER 3                 /* he got a little better by himself */
#define NZ_STINK   4                 /* a puke's stench reached him */
#define NZ_DODGE   5                 /* he got out of someone else's stench in time */

extern signed char nausea;
extern unsigned char nausea_pukes;   /* the player's pukes on this screen */

void nausea_timing(void);            /* (ticks.h: ticks_hz set) */
void nausea_reset(void);             /* the start of a screen */
void nausea_add(signed char amount);
/* something at square (x, y): the player feels it now, within radius squares */
void nausea_event(unsigned char x, unsigned char y, signed char amount, unsigned char radius);
/* a puke at square (x, y), the player's (npc 0) or the crowd's: its stench
 * reaches PUKE_RADIUS squares STINK_DELAY later, the player and the crowd
 * (npc_smell) */
void nausea_stink(unsigned char x, unsigned char y, unsigned char npc);
unsigned char nausea_tick(void);     /* once a frame: an NZ_ event or 0 */
unsigned char nausea_out(void);      /* passed out? */
unsigned char nausea_warning(void);  /* a puke is coming */
unsigned int nausea_puke_in(void);   /* frames to the next puke (at PUKE_AT or below) */
