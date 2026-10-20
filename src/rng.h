/* rng.h — a 16-bit xorshift: the crowd, the stands, the puddles */
#pragma once

unsigned int rng(void);
void rng_mix(unsigned int v);        /* stir something in (a frame count, a raster line) */
