/* tune.h — the music player: voices 1-3 of SID 1 (mirrored on SID 3)
 *
 * tune_play starts a tune (music.h TUNE_), not sick; tune_update runs it,
 * once a frame. tune_sick makes the music as sick as the player: more
 * wobble below 0, faster and out of tune from SWAY_FROM down (play calls
 * it every frame, so every screen's tune gets sick).
 */
#pragma once
#include "music.h"

void tune_play(unsigned char tune);
void tune_stop(void);
void tune_update(void);
void tune_sick(signed char nausea);
unsigned char tune_playing(void);    /* 0 once a tune that does not loop is over */
extern unsigned char tune_off;       /* music off (the options): tune_play plays nothing */
