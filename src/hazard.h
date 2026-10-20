/* hazard.h — things at the fair that make you sick: corn dog stands,
 * teacups, fireworks
 *
 * Each goes off every HAZARD_EVERY to + HAZARD_RND frames (fewer on harder
 * screens): its warning tile shows for HAZARD_WARN frames first (get
 * away!), then it costs the player and the crowd within its radius.
 */
#pragma once

#define HZ_CORNDOG  1                /* hazard_update: what went off */
#define HZ_TEACUP   2
#define HZ_FIREWORK 4
#define HZ_LIT      8                /* a firework was lit (its warning) */
#define HZ_WARNING 16                /* a corn dog stand or a teacup about to go */
extern unsigned char hz_fire_x, hz_fire_y;   /* the last firework to go off or light */

void hazard_timing(unsigned char difficulty);   /* (each screen) */
void hazard_reset(void);             /* find them on the map just loaded */
unsigned char hazard_update(void);   /* once a frame: HZ_ bits of what went off */
