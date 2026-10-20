/* options.c — the options screen (the overlay "options": it is only in
 * bank 0 while it runs)
 *
 * O on the title. Up / down choose a line, FIRE or left / right change it;
 * DONE (or O again) goes back to the title. Music off stops the tunes (the
 * sound effects go on); practice mode is an easier game, kept out of the
 * high scores (people.c, front.c).
 */
#pragma cc45 overlay(options)
#include "hw.h"
#include "tuning.h"
#include "assets.h"
#include "ctl.h"
#include "mux.h"
#include "sfx.h"
#include "tune.h"
#include "video.h"
#include "game.h"

#define LINES 3
static unsigned char row;

static void line(unsigned char n, const char *label, const char *value) {
    unsigned char r = 21 + n * 3;
    video_fill(26, r, 30, 0x20, C_WHITE);
    video_text(26, r, n == row ? s">" : s" ", C_YELLOW);
    video_text(28, r, label, n == row ? C_YELLOW : C_WHITE);
    if (value) video_text(44, r, value, n == row ? C_YELLOW : C_GREY);
}

static void draw(void) {
    line(0, s"music", tune_off ? s"off" : s"on ");
    line(1, s"practice", practice ? s"on " : s"off");
    line(2, s"done", 0);
}

void options(void) {
    mux_hide();
    video_clear();
    video_logo((COLS - LOGO_W) / 2, 2);
    centre(16, s"options", C_CYAN);
    centre(33, s"practice: the crowd gets sick slower, the hazards", C_GREY);
    centre(35, s"wait longer, the aunt is slower - no high scores.", C_GREY);
    centre(40, s"up / down, fire", C_GREY);
    row = 0;
    draw();
    state = ST_OPTIONS;
}

void options_input(unsigned char pressed) {
    if (pressed & C_UP) row = row ? row - 1 : LINES - 1;
    if (pressed & C_DOWN) row = row == LINES - 1 ? 0 : row + 1;
    if ((pressed & C_OPTIONS) || ((pressed & C_FIRE) && row == LINES - 1)) {
        title(0);
        return;
    }
    if (pressed & (C_FIRE | C_LEFT | C_RIGHT)) {
        sfx_pickup();
        if (row == 0) {
            tune_off ^= 1;
            if (tune_off) tune_stop();
            else tune_play(TUNE_TITLE);
        } else practice ^= 1;
    }
    if (pressed) draw();
}
