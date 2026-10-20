/* front.c — the title and the high scores (the overlay "front": it is
 * only in bank 0 while it runs)
 */
#pragma cc45 overlay(front)
#include <dma.h>
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

#define PAL_R(i) REG(0xD100 + (i))
#define PAL_G(i) REG(0xD200 + (i))
#define PAL_B(i) REG(0xD300 + (i))

/* The logo's slime moves by palette cycling (see tools/art.py): the
 * drips' 4 flow entries take 4 shades in turn, so their bands run down;
 * each set of drop entries lights one blob at a time, top to bottom, then
 * none for a step - a drop falls. Set B runs 2 steps behind set A. */
static const unsigned char flow_r[4] = {226, 214, 176, 118};
static const unsigned char flow_g[4] = {250, 214, 192, 136};
static const unsigned char flow_b[4] = {176, 96, 44, 22};
#define DROP_R 214
#define DROP_G 214
#define DROP_B 96
#define DROP_STEPS 5                          /* 4 blobs, then a pause */

static void set_entry(unsigned char e, unsigned char r, unsigned char g, unsigned char b) {
    PAL_R(e) = video_nib(r);
    PAL_G(e) = video_nib(g);
    PAL_B(e) = video_nib(b);
}

static void drops(unsigned char first, unsigned char step) {
    unsigned char k;
    for (k = 0; k < 4; k++)
        if (k == step) set_entry(first + k, DROP_R, DROP_G, DROP_B);
        else set_entry(first + k, 0, 0, 0);
}

static unsigned char flow, drop;

static void logo_step(void) {
    unsigned char k, c;
    REG(0xD070) = v_pal_screen;
    flow = (flow + 1) & 3;
    for (k = 0; k < 4; k++) {                 /* band k shows the shade that was above it */
        c = (unsigned char)((k - flow) & 3);
        set_entry(LOGO_FLOW + k, flow_r[c], flow_g[c], flow_b[c]);
    }
    if (++drop == DROP_STEPS) drop = 0;
    drops(LOGO_DROP_A, drop);
    drops(LOGO_DROP_B, (unsigned char)((drop + 2) % DROP_STEPS));
}

void video_logo_animate(void) {
    static unsigned char tick;
    if (++tick < LOGO_RATE) return;
    tick = 0;
    logo_step();
}

void video_logo(unsigned char col, unsigned char row) {
    unsigned char x, y, c, line[LOGO_W];
    for (y = 0; y < LOGO_H; y++) {
        dma_copy_far((unsigned long)(unsigned int)line, GFX_LIN + LOGO_OFS + (unsigned int)y * LOGO_W, LOGO_W);
        for (x = 0; x < LOGO_W; x++)
            if ((c = line[x]) != 255)
                video_cell((row + y) * COLS + col + x, TILE_CHAR + LOGO_CHAR + c, 0);
    }
    logo_step();                              /* the slime's colours */
}

/* the cast, walking on the spot under the logo: who, their first picture */
#define CAST_ROW 18
static const unsigned char cast_col[4] = {16, 32, 48, 64};
static const unsigned char cast_frame[4] = {0, FRAME_CROWD, FRAME_KID, FRAME_AUNT};

void title_cast(void) {
    static unsigned char tick;
    unsigned char k, step = 1 + ((++tick >> 3) & 1);
    mux_begin();
    for (k = 0; k < 4; k++)
        mux_add(k, cast_frame[k] + step, SPR_X0 + cast_col[k] * 8 - 16, SPR_ROW(CAST_ROW));
    mux_commit();
    video_logo_animate();
}

static void label(unsigned char col, unsigned char row, const char *s, unsigned char colour) {
    unsigned char n = 0;
    while (s[n]) n++;
    video_text((unsigned char)(col - n / 2), row, s, colour);
}

/* the lower half of the title: the rules or the high scores, in turn */
unsigned char page;
unsigned int page_left;

void title_page(void) {
    unsigned char r;
    for (r = 27; r < 42; r++) video_fill(0, r, COLS, 0x20, C_WHITE);
    if (page) {
        video_page(PAGE_SCORES_TITLE, 0);
        hs_show(31);
    } else video_page(PAGE_RULES, 0);
    page_left = t_of(TITLE_PAGE);
}

/* the starting screen, chosen with left / right */
void title_select(void) {
    char line[24];
    unsigned char k;
    static const char head[] = s"<  start at screen ";
    for (k = 0; head[k]; k++) line[k] = head[k];
    line[k++] = (char)('1' + start_level);
    line[k++] = ' ';
    line[k++] = ' ';
    line[k++] = '>';
    line[k] = 0;
    centre(46, line, C_GREY);
    video_fill(0, 48, COLS, 0x20, C_WHITE);
    video_page(level_name[start_level], 48);
    centre(42, players == 2 ? (practice ? s"^  2 players - practice  v" : s"     ^  2 players  v      ")
                            : (practice ? s"^  1 player - practice   v" : s"     ^  1 player   v      "), C_GREY);
}

void title(unsigned char scores) {
    mux_sway = 0;
    video_sway(0);
    v_mood = MOOD_NONE;
    v_fade = 16;               /* (the fireworks' night over) */
    video_tint(0);
    video_clear();
    video_logo((COLS - LOGO_W) / 2, 3);
    centre(15, s"keep your lunch down at the fair", C_WHITE);
    label(cast_col[0], CAST_ROW + 6, s"you", C_LRED);
    label(cast_col[1], CAST_ROW + 6, s"the crowd", C_CYAN);
    label(cast_col[2], CAST_ROW + 6, s"the kid", C_YELLOW);
    label(cast_col[3], CAST_ROW + 6, s"the aunt", C_LGREEN);
    page = scores;
    attract = 0;
    title_page();
    centre(44, s"press fire     o: options", C_WHITE);
    video_text(76, 49, s"v1.0", C_GREY);
    title_select();
    title_cast();
    tune_play(TUNE_TITLE);
    sfx_puke();                       /* BLARGH: welcome to Puke Fest */
    state = ST_TITLE;
}

/* game over: on to the initials if the score made the table */
static unsigned char entry_rank, entry_pos, initials[3];

static void entry_draw(void) {
    unsigned char k;
    for (k = 0; k < 3; k++)
        video_fill(38 + k, 26, 1, initials[k], k == entry_pos ? C_YELLOW : C_WHITE);
}

/* the next player (of entry_player on) whose score made the table: their
 * initials; none: the title */
static unsigned char entry_player;

static void next_entry(void) {
    static const char p1[] = s"player 1: a new high score!";
    char head[30];
    unsigned char k;
    if (practice) entry_player = players;     /* practice games: no high scores */
    for (; entry_player < players; entry_player++) {
        entry_rank = hs_rank(pl[entry_player].score);
        if (entry_rank < HS_COUNT) break;
    }
    if (entry_player == players) {
        title(1);
        return;
    }
    box(20, 9);
    if (players == 2) {
        for (k = 0; p1[k]; k++) head[k] = p1[k];
        head[k] = 0;
        head[7] = (char)('1' + entry_player);
        centre(21, head, C_YELLOW);
    } else centre(21, s"a new high score!", C_YELLOW);
    centre(23, s"your initials: up / down, fire", C_WHITE);
    initials[0] = initials[1] = initials[2] = 1;   /* a a a */
    entry_pos = 0;
    entry_draw();
    state = ST_ENTRY;
}

void game_over_done(void) {
    entry_player = 0;
    next_entry();
}

void entry(unsigned char c, unsigned char pressed) {
    unsigned char *l = &initials[entry_pos];
    if (pressed & C_UP) *l = *l == 26 ? 1 : *l + 1;
    if (pressed & C_DOWN) *l = *l == 1 ? 26 : *l - 1;
    if ((pressed & C_LEFT) && entry_pos) entry_pos--;
    if (pressed & (C_FIRE | C_RIGHT)) {
        sfx_pickup();
        if (++entry_pos == 3) {
            hs_insert(entry_rank, pl[entry_player].score, initials, pl[entry_player].round + 1,
                      pl[entry_player].level + 1);
            centre(28, s"saving...", C_GREY);
            hs_save();
            entry_player++;
            next_entry();
            return;
        }
    }
    (void)c;
    entry_draw();
}

