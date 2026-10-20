/* pfest — keep your lunch down at the county fair (MEGA65, cc45)
 *
 * Seven screens of the fair, each harder (and harder again every round):
 * tickets and the exit gate, the player's nausea, pukes and puddles, the
 * crowd, the cotton-candy kid, the Hugging Aunt, corn dog stands, teacups,
 * fireworks; mints, ginger ale and benches; lives, high scores on the disk.
 * 80 x 50 (640 x 400); the art and data load from disk (PFGFX1-3) at the start.
 *
 * Joystick in either port, W A S D or the cursor keys; FIRE / SPACE starts;
 * RUN/STOP quits to BASIC.
 *
 * Left alone, the title shows the rules, the high scores, then a demo: the
 * machine plays a screen (ai.c). P pauses.
 *
 * Build with -DDEMO to have the machine play from the start, queasy (for
 * screenshots and soak tests).
 */
#include <input.h>
#include "hw.h"
#include "tuning.h"
#include "assets.h"
#include "ai.h"
#include "bag.h"
#include "ctl.h"
#include "hazard.h"
#include "map.h"
#include "mux.h"
#include "nausea.h"
#include "npc.h"
#include "player.h"
#include "sfx.h"
#include "spr.h"
#include "tune.h"
#include "hiscore.h"
#include "ticks.h"
#include "rng.h"
#include "video.h"
#include "game.h"

/* its functions keep their variables in static storage and nothing
 * recurses: a 1 KB stack is plenty */
#pragma cc45 stack_size(512)

unsigned char state, level, round, lives;
#ifdef DEMO
unsigned char attract = 1;           /* the machine plays (ai.c) */
#else
unsigned char attract;
#endif
unsigned int attract_left;
unsigned char prev_ctl = 0xFF;       /* as if all held: a press counts once let go (RETURN of RUN) */
unsigned long score;
unsigned int pause, t_pause, t_intro;
unsigned char flash;                 /* frames of border flash left */
unsigned char story_win;             /* the story shows the win page */
unsigned char start_level;           /* chosen on the title */
unsigned char players = 1, cur;
unsigned char practice;
#ifdef PRESS_TEST2
unsigned char *test_turns;            /* (tests: whose turn each pass-out was) */
#endif
player_t pl[2];
const unsigned char screen_tune[NLEVELS] = {TUNE_STROLL, TUNE_MARCH, TUNE_POLKA, TUNE_TEACUPS,
                                            TUNE_FUNHOUSE, TUNE_BOAT, TUNE_FIREWORKS};

/* ------------------------------------------------------------------------ */
/* The HUD row */

signed char shown_nausea = 99;       /* what the HUD shows (hud_frame: none yet) */
unsigned char shown_tickets = 99, shown_lives = 99, shown_bag = 99, shown_puke = 0xFF;
unsigned long shown_score = 0xFFFFFFFFUL;

/* the HUD: what changed is drawn again (turns.c draws it) */
void hud(void) {
    unsigned char i;
    if (nausea != shown_nausea) hud_nausea();
    if (nausea <= PUKE_AT && state == ST_PLAY) {   /* "puke in 12": the seconds to the next */
        i = nausea_warning() ? 0 : (unsigned char)((nausea_puke_in() + ticks_hz - 1) / ticks_hz);
        if (i != shown_puke) hud_puke(i);
    }
    if (map_tickets != shown_tickets) hud_tickets();
    if (score != shown_score) hud_score();
    if (player_bag != shown_bag) hud_bag();
    if (lives != shown_lives) hud_lives();
}

/* ------------------------------------------------------------------------ */
/* Messages: lines of text in a box in the middle of the play area */

void box(unsigned char row, unsigned char rows) {
    unsigned char r;
    for (r = 0; r < rows; r++) video_fill(18, row + r, 44, 0x20, C_WHITE);
}

void centre(unsigned char row, const char *s, unsigned char colour) {
    unsigned char n = 0;
    while (s[n]) n++;
    video_text((unsigned char)((COLS - n) / 2), row, s, colour);
}

/* ------------------------------------------------------------------------ */
/* The game */

#ifdef PUKE_LOG
unsigned int log_hugs, log_puffs, log_pukes, log_bumps, log_overlaps, log_frames, log_bags, log_catches, log_throws, log_hits;
unsigned int log_clear_at[8], log_out_at[8], log_own_pukes, log_stinks, log_hazard_hits;   /* frames of each clear, pass-out */
unsigned char log_clears, log_outs;   /* (tests: read from a memory dump) */
#endif

void message(const char *a, const char *b, unsigned char colour) {
    mux_sway = 0;
    video_sway(0);
    box(21, 5);
    centre(22, a, colour);
    centre(24, b, C_WHITE);
    pause = t_pause;
}

static unsigned char burst, burst_x, burst_y;   /* a firework: frames left, its square */
static unsigned char heart;          /* frames to the next heartbeat */
#define BURST_FRAMES 24

/* very sick: the screen sways, a sine wave (1/64ths) over 64 frames */
static const signed char wave[32] = {0, 12, 24, 36, 45, 53, 59, 63, 64, 63, 59, 53, 45, 36, 24, 12, 0, -12, -24, -36, -45, -53, -59, -63, -64, -63, -59, -53, -45, -36, -24, -12};

static void sway(void) {
    static unsigned char t;
    signed char dx = 0;
    if (nausea <= SWAY_FROM) {
        signed int a = (SWAY_FROM + 1 - nausea) * SWAY_GAIN;
        dx = (signed char)(a * wave[(++t >> 1) & 31] / 64);
    }
    mux_sway = dx;
    video_sway(dx);
}

static void play(unsigned char c) {
    unsigned char e = player_update(c), z, ne, hz_fired;
    if (e & PE_TICKET) {
        score += SCORE_TICKET;
        sfx_pickup();
        if (--map_tickets == 0) {
            map_open_gates();
            sfx_gate();
        }
    }
    if (e & PE_MINT) { nausea_add(MINT_GAIN); score += SCORE_MINT; sfx_good(); }
    if (e & PE_ALE) { nausea_add(ALE_GAIN); score += SCORE_ALE; sfx_good(); }
    if (e & PE_BENCH) { nausea_add(BENCH_GAIN); sfx_good(); }
    if (e & PE_PUDDLE) { nausea_add(PUDDLE_COST); sfx_splat(); }
    if (e & PE_BUMP) sfx_oof();
    if (e & PE_BAG) { sfx_pickup(); shown_bag = 0xFF; }
    if ((c & C_FIRE) && bag_throw()) {
        sfx_whoosh();
#ifdef PUKE_LOG
        log_throws++;
#endif
    }
    if (bag_update()) {
        score += SCORE_HIT;
        sfx_thud();
#ifdef PUKE_LOG
        log_hits++;
#endif
    }
#ifdef PUKE_LOG
    if (e & PE_BAG) log_bags++;
#endif
#ifdef PUKE_LOG
    if (e & PE_BUMP) log_bumps++;
    log_frames++;
    if (npc_at(player_x(), player_y(), NPC_NONE) != NPC_NONE) log_overlaps++;
#endif

    z = nausea_tick();
    if (z == NZ_WARN) sfx_warn();
    if (z == NZ_RECOVER) sfx_good();
    if (z == NZ_STINK) sfx_bad();
#ifdef PUKE_LOG
    if (z == NZ_STINK) log_stinks++;
    if (z == NZ_PUKE) log_own_pukes++;
#endif
    if (z == NZ_DODGE) { score += SCORE_DODGE; sfx_good(); }
    if (z == NZ_PUKE) {
        nausea_add(PUKE_GAIN);
        sfx_puke();
        if (player_bag == BAG_EMPTY) {    /* into the bag: no puddle, no stench */
            player_puke(1);
            player_bag = BAG_FULL;
            score += SCORE_CATCH;
#ifdef PUKE_LOG
            log_catches++;
#endif
        } else {
            player_puke(0);
            nausea_stink(player_sx, player_sy, 0);
            if (npc_near(player_sx, player_sy, PUKE_RADIUS)) sfx_eww();
            flash = 8;
        }
    }

    ne = npc_update();
#ifdef PUKE_LOG
    if (ne & (NE_PUKE | NE_KID_PUKE)) log_pukes++;
    if (ne & NE_HUG) log_hugs++;
#endif
    if (ne & NE_CAUGHT) score += SCORE_CATCH;
#ifdef PUKE_LOG
    if (ne & NE_CAUGHT) log_catches++;
#endif
    if (ne & NE_PUKE) sfx_crowd_puke(0);
    if (ne & NE_KID_PUKE) sfx_crowd_puke(1);
    if (ne & NE_HUG) {
        nausea_add(HUG_COST);
        player_stun(HUG_STUN);
        sfx_hug();
    }
    hz_fired = hazard_update();
    if (hz_fired) {
#ifdef PUKE_LOG
        log_puffs++;
#endif
        if (hz_fired & HZ_CORNDOG) sfx_puff();
        if (hz_fired & HZ_TEACUP) sfx_spin();
        if (hz_fired & HZ_LIT) sfx_whistle();
        if (hz_fired & HZ_WARNING) sfx_alert();
        if (hz_fired & HZ_FIREWORK) {
            sfx_bang();
            flash = 4;
            burst = BURST_FRAMES;
            burst_x = hz_fire_x;
            burst_y = hz_fire_y;
        }
    }

    if (e & PE_EXIT) {
#ifdef PUKE_LOG
        if (log_clears < 8) log_clear_at[log_clears++] = log_frames;
#endif
        score += SCORE_CLEAR;
        if (nausea_pukes == 0) score += SCORE_NO_PUKE;
        state = ST_CLEAR;
        tune_play(TUNE_CLEAR);
        message(s"screen clear!", nausea_pukes ? s"bonus 500" : s"no pukes! bonus 1500", C_LGREEN);
    } else if (nausea_out()) {
#ifdef PUKE_LOG
        if (log_outs < 8) log_out_at[log_outs++] = log_frames;
#endif
        lives--;
        state = ST_OUT;
        sfx_passout();
        tune_play(TUNE_PASSOUT);
        message(s"you passed out!", lives ? s"get up and try again" : s"", C_LRED);
    }
    if (nausea <= HEART_FROM) {           /* the heart pounds, faster the sicker */
        if (!heart--) {
            sfx_heart();
            heart = (unsigned char)(HEART_FRAMES + (nausea - HEART_FROM) * 6);
        }
    } else heart = 0;
    sway();
    tune_sick(nausea);
    mux_begin();
    player_draw(nausea < 0, nausea_warning());
    npc_draw();
    bag_draw();
    if (burst) {                          /* a firework bursting over its tube */
        burst--;
        mux_add(14, FRAME_BURST + (burst < BURST_FRAMES / 2), map_spr_x((signed int)burst_x * SQ),
                map_spr_y((signed int)burst_y * SQ - 30));
    }
    mux_commit();
}

int main(void) {
    if (!boot()) return 1;
    for (;;) {
        unsigned char c, pressed;
        video_wait_frame();
        c = ctl_read();
        pressed = c & (unsigned char)~prev_ctl;   /* just pressed */
        prev_ctl = c;
        if (c & C_QUIT) break;
#ifdef PRESS_TEST4
        {                                 /* (tests: the options: music off, practice on, then a game) */
            static unsigned int fr;
            fr++;
            if (fr == 100 || fr == 250) pressed |= C_OPTIONS;
            if (fr == 130 || fr == 190 || fr == 300 || fr == 400) pressed |= C_FIRE;
            if (fr == 160) pressed |= C_DOWN;
        }
#endif
#ifdef PRESS_TEST3
        {                                 /* (tests: someone choosing on the title every 5 s for 30 s) */
            static unsigned int fr;
            if (++fr < 1500 && fr % 250 == 0) pressed |= (fr / 250) & 1 ? C_RIGHT : C_LEFT;
        }
#endif
#ifdef PRESS_TEST2
        {                                 /* (tests: a two-player game to the end) */
            static unsigned int fr, inplay;
            static unsigned char turns_seen[8], nturns;
            fr++;
            if (state == ST_TITLE && fr == 100) pressed |= C_DOWN;
            if (state == ST_TITLE && fr == 150) pressed |= C_FIRE;
            if ((state == ST_STORY || state == ST_OVER || state == ST_ENTRY) && !(fr & 15)) pressed |= C_FIRE;
            if (state == ST_PLAY && ++inplay == 60) {
                inplay = 0;
                score += 3000;
                nausea = NAUSEA_PASSOUT;
                if (nturns < 8) turns_seen[nturns++] = cur + 1;
                test_turns = turns_seen;
            }
        }
#endif
#ifdef PRESS_TEST
        {                                 /* (tests: the presses of a player, then a pass-out) */
            static unsigned int fr;
            fr++;
            if (fr == 100 || fr == 130) pressed |= C_RIGHT;
            if (fr == 160) pressed |= C_DOWN;
            if (fr == 200 || fr == 300) pressed |= C_FIRE;
            if (fr == 420) nausea = NAUSEA_PASSOUT;
        }
#endif
        if (attract) {                    /* the machine plays: a press ends it */
#ifndef DEMO
            if ((pressed & C_FIRE) || (state != ST_PLAY && state != ST_INTRO) || !--attract_left) {
                title(0);
                continue;
            }
#endif
            c = state == ST_PLAY ? ai_read() : 0;
            pressed = 0;
        }
        sfx_update();
        if (state != ST_PAUSE) tune_update();
        video_animate();
        if (state == ST_PLAY) video_mood();
        if (flash) REG(0xD020) = --flash ? C_GREEN : C_BLACK;

        switch (state) {
        case ST_TITLE:
            if (pressed & (C_LEFT | C_RIGHT | C_UP | C_DOWN))
                page_left = t_of(TITLE_PAGE);   /* someone is there: the demo waits */
            if (pressed & (C_LEFT | C_RIGHT)) {    /* the starting screen: its music */
                if ((pressed & C_LEFT) && start_level) start_level--;
                if ((pressed & C_RIGHT) && start_level < NLEVELS - 1) start_level++;
                tune_play(screen_tune[start_level]);
                title_select();
            }
            if (pressed & (C_UP | C_DOWN)) {       /* 1 or 2 players */
                players = 3 - players;
                title_select();
            }
            if (pressed & C_OPTIONS) {
                options();
                break;
            }
            if (pressed & C_FIRE) new_game();
            else {
                title_cast();
                if (--page_left == 0) {
                    if (page) {                   /* rules, scores, then a demo */
                        attract = 1;
                        attract_left = t_of(ATTRACT_FRAMES);
                        new_game();
                    } else {
                        page = 1;
                        title_page();
                    }
                }
            }
            break;
        case ST_STORY:
            video_logo_animate();
            if ((pressed & C_FIRE) || !--pause) {
                if (story_win) story(0);     /* the win page, then round 2's first chapter */
                else start_screen();
            }
            break;
        case ST_PAUSE:
            if (pressed & (C_PAUSE | C_FIRE)) {
                sfx_mute(0);
                map_draw();               /* the box away */
                state = ST_PLAY;
            }
            break;
        case ST_INTRO:
            if (--pause == 0) {
                map_draw();               /* the box away */
                state = ST_PLAY;
            }
            break;
        case ST_PLAY:
            if (pressed & C_PAUSE) {
                sfx_mute(1);
                mux_sway = 0;
                video_sway(0);
                box(22, 3);
                centre(23, s"paused - press p", C_YELLOW);
                state = ST_PAUSE;
                break;
            }
            play(c);
            break;
        case ST_OUT:
        case ST_CLEAR:
            if (--pause) break;
            if (state == ST_CLEAR && ++level == NLEVELS) {
                level = 0;
                round++;
                score += SCORE_WIN;
                if (attract) start_screen();
                else story(1);
            } else if (state == ST_CLEAR && !attract) story(0);
            else if (state == ST_OUT && players == 2 && pl[cur ^ 1].lives) next_turn();
            else if (lives) start_screen();
            else {
                keep();
                state = ST_OVER;
                mux_hide();
                message(s"game over", s"press fire", C_LRED);
                tune_play(TUNE_GAMEOVER);
            }
            break;
        case ST_OPTIONS:
            options_input(pressed);
            break;
        case ST_TURN:
            if (--pause == 0) start_screen();
            break;
        case ST_WIN:
            if (--pause == 0) start_screen();
            break;
        case ST_OVER:
#ifdef DEMO
            if (--pause == 0) new_game();
#endif
            if (pressed & C_FIRE) game_over_done();
            break;
        case ST_ENTRY:
            entry(c, pressed);
            break;
        }
        if (state != ST_TITLE && state != ST_STORY && state != ST_OPTIONS) hud();
    }
    shutdown();
    return 0;
}
