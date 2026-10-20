/* ticks.c — times in this machine's frames
 *
 * In 16 bits: NTSC adds a fifth (60 / 50); harder screens take 8 / (8 + d)
 * of a time (divided first: a frame or two lost, never an overflow).
 */
#include "ticks.h"

unsigned char ticks_hz = 50;

unsigned int t_of(unsigned int frames) {
    return ticks_hz == 60 ? frames + frames / 5 : frames;
}

unsigned int t_hard(unsigned int frames, unsigned char d) {
    return t_of(frames) / (8 + d) * 8;
}
