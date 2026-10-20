/* ticks.h — times: frames at 50 Hz (tuning.h) as this machine's frames */
#pragma once

extern unsigned char ticks_hz;               /* 50 (PAL) or 60 (NTSC) */
unsigned int t_of(unsigned int frames);      /* 50 Hz frames -> ours */
unsigned int t_hard(unsigned int frames, unsigned char d);   /* and 8 / (8 + d) of it */
