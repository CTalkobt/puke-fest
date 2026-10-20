/* turns.c — a new game, and two players taking turns (the overlay
 * "turns": it is only in bank 0 while it runs)
 *
 * The HUD row's drawing is here too (main.c checks what changed).
 *
 * A turn lasts until a pass-out; then the other player plays (if they have
 * lives left) their own game: score, lives, screen and round, kept in pl[].
 */
#pragma cc45 overlay(turns)
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

/* the current player's game into pl[cur], or player p's out of it */
void keep(void) {
    pl[cur].score = score;
    pl[cur].lives = lives;
    pl[cur].level = level;
    pl[cur].round = round;
}

static void take(unsigned char p) {
    cur = p;
    score = pl[p].score;
    lives = pl[p].lives;
    level = pl[p].level;
    round = pl[p].round;
}

/* "player 2" (screen codes) into s, from s[0] */
static void player_name(char *s, unsigned char p) {
    static const char name[] = s"player ";
    unsigned char k;
    for (k = 0; name[k]; k++) s[k] = name[k];
    s[k++] = (char)('1' + p);
    s[k] = 0;
}

/* the other player's turn: a card, then their screen */
void next_turn(void) {
    static const char out[] = s" is out";
    static const char ready[] = s" - get ready";
    char a[20], b[22];
    unsigned char k, n;
    keep();
    player_name(a, cur);
    take(cur ^ 1);
    player_name(b, cur);
    if (!pl[cur ^ 1].lives) {                 /* the one before is out for good: */
        for (n = 0; a[n]; n++) {}             /* "player 1 is out", "player 2 - get ready" */
        for (k = 0; out[k]; k++) a[n + k] = out[k];
        a[n + k] = 0;
        for (n = 0; b[n]; n++) {}
        for (k = 0; ready[k]; k++) b[n + k] = ready[k];
        b[n + k] = 0;
        mux_hide();
        message(a, b, C_YELLOW);
    } else {                                  /* "player 2", "get ready" */
        mux_hide();
        message(b, s"get ready", C_YELLOW);
    }
    state = ST_TURN;
}


void new_game(void) {
#ifdef START_LEVEL
    level = START_LEVEL;              /* (tests) */
#else
    level = attract ? (unsigned char)(rng() % NLEVELS) : start_level;
#endif
    round = 0;
    lives = LIVES;
    score = 0;
    if (attract) players = 1;
    for (cur = 0; cur < 2; cur++) {       /* both start the same */
        pl[cur].score = 0;
        pl[cur].lives = cur < players ? LIVES : 0;
        pl[cur].level = level;
        pl[cur].round = 0;
    }
    cur = 0;
#ifdef DEMO
    start_screen();
#else
    if (attract) start_screen();
    else story(0);
#endif
}

/* ---- the HUD row: drawing what changed (main.c's hud decides) -------- */

void hud_nausea(void) {
    unsigned char i, n, c;
    shown_nausea = nausea;
    n = (unsigned char)(nausea < 0 ? -nausea : nausea);
    video_tint(nausea < 0 ? (n > SICK_TINT_MAX ? SICK_TINT_MAX : n) : 0);
    c = nausea < 0 ? C_LRED : (nausea < 3 ? C_YELLOW : C_LGREEN);
    video_fill(7, HUD_ROW, 1, nausea < 0 ? '-' : '+', c);
    video_num(8, HUD_ROW, n, 2, c);
    shown_puke = 0xFF;
    if (nausea > PUKE_AT)                     /* the bar; at PUKE_AT and below, the countdown */
        for (i = 0; i < 10; i++)
            video_fill(11 + i, HUD_ROW, 1, i < n ? 0xA0 : 0x2D, i < n ? c : C_GREY);
}

void hud_puke(unsigned char seconds) {
    shown_puke = seconds;
    if (seconds) {
        video_text(11, HUD_ROW, s"puke in   ", C_LRED);
        video_num(19, HUD_ROW, seconds, 2, C_LRED);
    } else video_text(11, HUD_ROW, s"puke now! ", C_YELLOW);
}

void hud_tickets(void) {
    shown_tickets = map_tickets;
    video_num(28, HUD_ROW, map_tickets, 1, map_tickets ? C_ORANGE : C_LGREEN);
}

void hud_score(void) {
    char buf[7];
    unsigned long v = score;
    unsigned char i;
    shown_score = score;
    for (i = 6; i--; ) {
        buf[i] = (char)(0x30 + (unsigned char)(v % 10));
        v /= 10;
    }
    buf[6] = 0;
    video_text(40, HUD_ROW, buf, C_WHITE);
}

void hud_bag(void) {
    shown_bag = player_bag;
    video_text(30, HUD_ROW, player_bag == BAG_NONE ? s"    " : player_bag == BAG_EMPTY ? s"bag " : s"bag!",
               player_bag == BAG_FULL ? C_LGREEN : C_WHITE);
}

void hud_lives(void) {
    unsigned char i;
    shown_lives = lives;
    for (i = 0; i < 3; i++) video_fill(77 + i, HUD_ROW, 1, i < lives ? 0x53 : 0x20, C_RED);
}
