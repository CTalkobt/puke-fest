/* level.c — starting a screen: the map and its puddles, the crowd, the
 * hazards, the HUD (the overlay "level": it is only in bank 0 while it runs)
 */
#pragma cc45 overlay(level)
#include <dma.h>
#include <input.h>
#include "hw.h"
#include "tuning.h"
#include "assets.h"
#include "ai.h"
#include "ctl.h"
#include "hazard.h"
#include "hiscore.h"
#include "map.h"
#include "mux.h"
#include "nausea.h"
#include "npc.h"
#include "player.h"
#include "rng.h"
#include "sfx.h"
#include "spr.h"
#include "ticks.h"
#include "tune.h"
#include "video.h"
#include "game.h"
#include "npc_int.h"
#include "bag.h"
#include "hazard_int.h"

/* ---- the map ---------------------------------------------------------- */

static unsigned char apart(unsigned char a, unsigned char b, unsigned char x, unsigned char y) {
    unsigned char dx = a > x ? a - x : x - a, dy = b > y ? b - y : y - b;
    return (dx > dy ? dx : dy) > PUKE_RADIUS;
}

/* at least MIN_PUDDLES on the map: more at random squares that can have
 * one, not near the start and not where someone starts */
static void puddles(unsigned char level) {
    unsigned int i, tries;
    unsigned char have = 0, x, y, t, k, ok;
    for (i = 0; i < (unsigned int)map_w * map_h; i++)
        if (tile_attr[map_sq[i]] & A_PUKE) have++;
    for (tries = 0; have < MIN_PUDDLES && tries < 500; tries++) {
        x = (unsigned char)(rng() % map_w);
        y = (unsigned char)(rng() % map_h);
        t = tile_puke[map_sq[y * map_w + x]];
        if (t == NO_TILE || !apart(x, y, level_sx[level], level_sy[level])) continue;
        ok = 1;
        for (k = npc_first[level]; k < npc_first[level + 1]; k++)
            if (npc_sx[k] == x && npc_sy[k] == y) ok = 0;
        if (!ok) continue;
        map_sq[y * map_w + x] = t;
        have++;
    }
}

void map_load(unsigned char level) {
    unsigned int i;
    map_w = level_w[level];
    map_h = level_h[level];
    map_ox = (PLAY_W - map_w) / 2;
    map_oy = (PLAY_H - map_h) / 2;
    dma_copy_far((unsigned long)(unsigned int)map_sq, DATA_LIN + level_ofs[level], (unsigned int)map_w * map_h);
    map_tickets = 0;
    for (i = 0; i < (unsigned int)map_w * map_h; i++)
        if (tile_attr[map_sq[i]] & A_TICKET) map_tickets++;
    puddles(level);
    map_draw();
}


/* ---- the HUD and the screen ------------------------------------------- */

void hud_frame(void) {
    video_fill(0, HUD_ROW, COLS, 0x20, C_WHITE);
    video_text(0, HUD_ROW, s"nausea", C_WHITE);
    video_text(24, HUD_ROW, s"tix", C_WHITE);
    video_text(34, HUD_ROW, s"score", C_WHITE);
    video_text(50, HUD_ROW, s"screen", C_WHITE);
    video_num(57, HUD_ROW, level + 1, 1, C_CYAN);
    if (players == 2) {
        video_text(47, HUD_ROW, s"p", C_YELLOW);
        video_num(48, HUD_ROW, cur + 1, 1, C_YELLOW);
    }
    if (attract) video_text(59, HUD_ROW, s"demo - press fire", C_YELLOW);
    else if (practice) video_text(68, HUD_ROW, s"practice", C_LGREEN);
    else if (round) {
        video_text(61, HUD_ROW, s"round", C_WHITE);
        video_num(67, HUD_ROW, round + 1, 1, C_CYAN);
    }
    shown_nausea = 99;
    shown_tickets = shown_lives = shown_bag = 99;
    shown_score = 0xFFFFFFFFUL;
    hud();
}

/* the next screen: everything in place, its name in a box for a moment */
/* each screen's mood: the teacups spin, the mirrors flash, the water
 * shimmers, the fireworks are at night */
static const unsigned char moods[NLEVELS] = {MOOD_NONE, MOOD_NONE, MOOD_NONE, MOOD_RIDE, MOOD_GLASS, MOOD_WATER, MOOD_NIGHT};

void start_screen(void) {
    if (v_fade) {                     /* the old screen fades out */
        mux_hide();
        video_fade(0);
    }
    unsigned char d = level + round * NLEVELS;
    char line[16];
    npc_timing(d);
    hazard_timing(d);
    v_mood = moods[level];
    map_load(level);
    nausea_reset();
#if defined(DEMO) && !defined(BALANCE)
    nausea = -3;                      /* (the demo: sick at once, for screenshots) */
#endif
    player_reset(level_sx[level], level_sy[level]);
    ai_reset();
    bag_reset();
#ifdef BAG_TEST
    player_bag = BAG_FULL;            /* (tests: a full bag to throw) */
#endif
    npc_reset(level);
    hazard_reset();
    hud_frame();
    tune_sick(nausea);
    tune_play(screen_tune[level]);
    mux_begin();
    player_draw(0, 0);
    npc_draw();
    mux_commit();
    mux_sway = 0;
    video_sway(0);
    box(20, 7);
    line[0] = 19; line[1] = 3; line[2] = 18; line[3] = 5; line[4] = 5; line[5] = 14;   /* screen */
    line[6] = ' ';
    line[7] = (char)('1' + level);
    line[8] = ' '; line[9] = 15; line[10] = 6; line[11] = ' ';                      /* of */
    line[12] = (char)('0' + NLEVELS);
    line[13] = 0;
    centre(21, line, C_CYAN);
    video_page(level_name[level], 23);
    if (round) centre(25, s"(harder now)", C_LRED);
    video_fade(16);                   /* the new one fades in */
    pause = t_intro;
    state = ST_INTRO;
}



