/* bag.h — the full paper bag in the air */
#pragma once

unsigned char bag_throw(void);       /* at the nearest person in range: 1 if thrown */
unsigned char bag_update(void);      /* once a frame: 1 when it hits */
void bag_draw(void);                 /* mux_add it (id 15) */
void bag_reset(void);
