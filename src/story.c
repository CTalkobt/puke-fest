/* story.c — the story between screens (the overlay "story": it is only in
 * bank 0 while it runs)
 *
 * Before each screen: the logo, the chapter and the screen's name, the
 * player, the chapter's text (assets/text.txt: a warning about what comes)
 * and its music. After the seventh screen, the win page first.
 */
#pragma cc45 overlay(story)
#include "hw.h"
#include "tuning.h"
#include "assets.h"
#include "mux.h"
#include "ticks.h"
#include "tune.h"
#include "video.h"
#include "game.h"

static const unsigned int chapter[NLEVELS] = {PAGE_STORY1, PAGE_STORY2, PAGE_STORY3, PAGE_STORY4,
                                              PAGE_STORY5, PAGE_STORY6, PAGE_STORY7};

void story(unsigned char win) {
    char line[12];
    mux_sway = 0;
    video_sway(0);
    v_mood = MOOD_NONE;
    v_fade = 16;
    video_tint(0);
    video_clear();
    video_logo((COLS - LOGO_W) / 2, 2);
    if (win) {
        centre(15, s"the end of the day", C_CYAN);
        video_page(PAGE_WIN, 0);
    } else {
        line[0] = 3; line[1] = 8; line[2] = 1; line[3] = 16; line[4] = 20; line[5] = 5; line[6] = 18;   /* chapter */
        line[7] = ' ';
        line[8] = (char)('1' + level);
        line[9] = 0;
        centre(15, line, C_CYAN);
        video_page(level_name[level], 17);
        video_page(chapter[level], 0);
    }
    centre(41, s"press fire", C_WHITE);
    mux_begin();                          /* the player, standing there */
    mux_add(0, 0, SPR_X0 + 40 * 8 - 16, SPR_ROW(18));
    mux_commit();
    tune_play(TUNE_STORY);
    story_win = win;
    pause = t_of(STORY_FRAMES);
    state = ST_STORY;
}
