/* ctl.c — the controls
 *
 * The MEGA65's cursor left and up keys show in the matrix as right SHIFT +
 * cursor right / down; $D60F tells them apart (bit 0 left, bit 1 up).
 */
#include <input.h>
#include <mega65.h>
#include "hw.h"
#include "ctl.h"

#define CRSR_LEFTUP REG(0xD60F)

unsigned char ctl_read(void) {
    unsigned char c = 0, joy = input_joy(1) | input_joy(2), lu = CRSR_LEFTUP & 3;
    if (input_pressed(KEY_STOP)) c |= C_QUIT;
    if (input_pressed(KEY_W) || (lu & 2) || (joy & JOY_UP)) c |= C_UP;
    if (input_pressed(KEY_S) || (input_pressed(KEY_DOWN) && !(lu & 2)) || (joy & JOY_DOWN)) c |= C_DOWN;
    if (input_pressed(KEY_A) || (lu & 1) || (joy & JOY_LEFT)) c |= C_LEFT;
    if (input_pressed(KEY_D) || (input_pressed(KEY_RIGHT) && !(lu & 1)) || (joy & JOY_RIGHT)) c |= C_RIGHT;
    if (input_pressed(KEY_SPACE) || input_pressed(KEY_RETURN) || (joy & JOY_FIRE)) c |= C_FIRE;
    if (input_pressed(KEY_P)) c |= C_PAUSE;
    if (input_pressed(KEY_O)) c |= C_OPTIONS;
    return c;
}
