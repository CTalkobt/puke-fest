/* rng.c — a 16-bit xorshift */
#include "rng.h"

static unsigned int seed = 0x5EED;

unsigned int rng(void) {
    seed ^= seed << 7;
    seed ^= seed >> 9;
    seed ^= seed << 8;
    return seed;
}

void rng_mix(unsigned int v) {
    seed ^= v;
    if (!seed) seed = 0x5EED;
}
