/* people.c — setting up a screen's crowd and hazards (the overlay
 * "people": it is only in bank 0 while it runs)
 */
#pragma cc45 overlay(people)
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

/* ---- the crowd -------------------------------------------------------- */

void npc_timing(unsigned char d) {
    npc_t_drift = t_hard(NPC_DRIFT, d);
    npc_t_kid_drift = t_hard(KID_DRIFT, d);
    npc_aunt_speed = AUNT_SPEED + d > AUNT_FASTEST ? AUNT_FASTEST : AUNT_SPEED + d;
    if (practice) {                       /* the options' practice mode: easier */
        npc_t_drift += npc_t_drift / 2;
        npc_t_kid_drift += npc_t_kid_drift / 2;
        npc_aunt_speed -= PRACTICE_AUNT_SLOWER;
    }
    npc_t_warn_min = t_of(NPC_WARN_MIN);
    npc_t_warn_rnd = t_of(NPC_WARN_RND);
    npc_t_chase = t_of(AUNT_CHASE);
    npc_t_rest = t_of(AUNT_REST);
}

void npc_reset(unsigned char level) {
    unsigned char i, first = npc_first[level];
#ifndef DEMO
    rng_mix((unsigned int)FRAMECOUNT << 8 | REG(0xD012));   /* a different fair every time */
#endif
    npc_count = npc_first[level + 1] - first;
    if (npc_count > NPC_MAX) npc_count = NPC_MAX;
    for (i = 0; i < npc_count; i++) {
        n_kind[i] = npc_type[first + i];
        n_sx[i] = npc_sx[first + i];
        n_sy[i] = npc_sy[first + i];
        n_px[i] = ((signed int)n_sx[i] * SQ + SQ / 2) * 16;
        n_py[i] = ((signed int)n_sy[i] * SQ + SQ * 3 / 4) * 16;
        n_look[i] = n_kind[i] == NPC_KID ? FRAME_KID : n_kind[i] == NPC_AUNT ? FRAME_AUNT
                  : FRAME_CROWD + (i % NPC_VARIANTS) * NPC_FRAMES;
        n_way[i] = STAND;
        n_face[i] = DIR_DOWN;
        n_moving[i] = n_anim[i] = n_freeze[i] = 0;
        n_turn[i] = (unsigned char)(1 + rng() % NPC_TURN_MIN);
        n_nausea[i] = (signed char)(NPC_NAUSEA_MIN + rng() % NPC_NAUSEA_RND);
        n_puke_at[i] = npc_tolerance();
        n_drift[i] = npc_next_drift(i);
        n_warn[i] = n_chase[i] = n_rest[i] = 0;
    }
}


/* ---- the hazards ------------------------------------------------------ */

void hazard_timing(unsigned char d) {
    hz_t_every = t_hard(HAZARD_EVERY, d);
    hz_t_rnd = t_hard(HAZARD_RND, d);
    hz_t_warn = t_of(HAZARD_WARN);
    hz_t_show = t_of(HAZARD_SHOW);
    if (practice) {
        hz_t_every += hz_t_every / 2;
        hz_t_rnd += hz_t_rnd / 2;
    }
}

void hazard_reset(void) {
    unsigned char x, y, k, t;
    hz_n = 0;
    for (y = 0; y < map_h; y++)
        for (x = 0; x < map_w; x++) {
            t = map_get(x, y);
            for (k = 0; k < HZ_KINDS; k++)
                if (t == hz_tile[k] && hz_n < HZ_MAX) {
                    hz_x[hz_n] = x;
                    hz_y[hz_n] = y;
                    hz_kind[hz_n] = k;
                    hz_phase[hz_n] = HZ_IDLE;
                    hz_left[hz_n] = hz_t_every / 2 + rng() % hz_t_rnd;   /* the first a little sooner */
                    hz_n++;
                }
        }
}

