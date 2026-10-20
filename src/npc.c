/* npc.c — everyone else at the fair
 *
 * The crowd (and the kid) wander: a direction (or standing still) for a
 * while, then another, never into a solid square or a puddle. The fair
 * makes them a little sicker every so often (NPC_DRIFT, KID_DRIFT), and a
 * stench within PUKE_RADIUS squares costs them 1 (NPC_SMELL_IN times in
 * NPC_SMELL_OF). At their own puking point (NPC_PUKE_AT and up to
 * NPC_TOLERANCE - 1 below) they stop and turn green for 1 to 3 s, then
 * puke and feel better (NPC_RELIEF_). The spread keeps one puke from
 * setting off the rest all on the same frame.
 *
 * The aunt wanders too, until the player comes within AUNT_SEE squares:
 * then she heads for him (along the longer way first), for up to
 * AUNT_CHASE frames. If she reaches him: a hug (NE_HUG). After a chase she
 * rests AUNT_REST frames.
 */
#include "hw.h"
#include "tuning.h"
#include "assets.h"
#include "map.h"
#include "mux.h"
#include "nausea.h"
#include "player.h"
#include "rng.h"
#include "npc.h"
#include "npc_int.h"
#include "ticks.h"


unsigned char npc_count;
#ifdef PUKE_LOG
unsigned int puke_log[64];                           /* the frame of each crowd puke (tests) */
unsigned char puke_logged;
static unsigned int ticks;
#endif
unsigned char n_kind[NPC_MAX];
signed int n_px[NPC_MAX], n_py[NPC_MAX];          /* their feet, 1/16 pixels */
unsigned char n_sx[NPC_MAX], n_sy[NPC_MAX];       /* their squares */
unsigned char n_way[NPC_MAX], n_face[NPC_MAX], n_moving[NPC_MAX], n_anim[NPC_MAX];
unsigned char n_turn[NPC_MAX], n_look[NPC_MAX], n_freeze[NPC_MAX];   /* n_look: the first picture */
signed char n_nausea[NPC_MAX], n_puke_at[NPC_MAX];
unsigned int n_drift[NPC_MAX], n_warn[NPC_MAX];
unsigned int n_chase[NPC_MAX], n_rest[NPC_MAX];   /* the aunt */
unsigned int npc_t_drift, npc_t_kid_drift, npc_t_warn_min, npc_t_warn_rnd, npc_t_chase, npc_t_rest;
signed int npc_aunt_speed;

signed char npc_tolerance(void) {
    return (signed char)(NPC_PUKE_AT - (signed char)(rng() % NPC_TOLERANCE));
}

unsigned int npc_next_drift(unsigned char i) {
    unsigned int t = n_kind[i] == NPC_KID ? npc_t_kid_drift : npc_t_drift;
    return t / 2 + rng() % t;
}

static unsigned char within(unsigned char i, unsigned char x, unsigned char y, unsigned char radius) {
    unsigned char dx = n_sx[i] > x ? n_sx[i] - x : x - n_sx[i];
    unsigned char dy = n_sy[i] > y ? n_sy[i] - y : y - n_sy[i];
    return (dx > dy ? dx : dy) <= radius;
}

void npc_smell(unsigned char x, unsigned char y) {
    unsigned char i;
    for (i = 0; i < npc_count; i++)
        if (n_kind[i] != NPC_AUNT && within(i, x, y, PUKE_RADIUS) && n_nausea[i] > -9 &&
            rng() % NPC_SMELL_OF < NPC_SMELL_IN)
            n_nausea[i]--;
}

void npc_whiff(unsigned char x, unsigned char y, unsigned char radius) {
    unsigned char i;
    for (i = 0; i < npc_count; i++)
        if (n_kind[i] != NPC_AUNT && within(i, x, y, radius) && n_nausea[i] > -9) n_nausea[i]--;
}

unsigned char npc_at(signed int x, signed int y, unsigned char skip) {
    unsigned char i;
    signed int d;
    for (i = 0; i < npc_count; i++) {
        if (i == skip) continue;
        d = (n_px[i] >> 4) - x;
        if (d <= -BODY_X || d >= BODY_X) continue;
        d = (n_py[i] >> 4) - y;
        if (d > -BODY_Y && d < BODY_Y) return i;
    }
    return NPC_NONE;
}

void npc_make_way(unsigned char i) {
    signed int dx = (n_px[i] >> 4) - player_x(), dy = (n_py[i] >> 4) - player_y();
    if (n_kind[i] == NPC_AUNT || n_warn[i] || n_freeze[i]) return;   /* (busy) */
    /* away from him: across his way if it can, else on */
    if ((dx < 0 ? -dx : dx) > (dy < 0 ? -dy : dy)) n_way[i] = dy < 0 ? DIR_UP : DIR_DOWN;
    else n_way[i] = dx < 0 ? DIR_LEFT : DIR_RIGHT;
    n_turn[i] = NPC_TURN_MIN;
}

/* too close to the player? */
static unsigned char near_player(signed int x, signed int y) {
    signed int d = player_x() - x;
    if (d <= -BODY_X || d >= BODY_X) return 0;
    d = player_y() - y;
    return d > -BODY_Y && d < BODY_Y;
}

/* try to walk one step the way it faces; 0 if blocked */
static unsigned char step(unsigned char i, unsigned char w) {
    signed int nx = n_px[i], ny = n_py[i];
    signed int v = n_kind[i] == NPC_KID ? KID_SPEED : n_kind[i] == NPC_AUNT ? npc_aunt_speed : NPC_SPEED;
    switch (w) {
    case DIR_DOWN: ny += v; break;
    case DIR_UP: ny -= v; break;
    case DIR_LEFT: nx -= v; break;
    default: nx += v; break;
    }
    if (!map_clear(nx >> 4, ny >> 4)) return 0;
    if (map_attr((unsigned char)(nx >> 9), (unsigned char)(ny >> 9)) & A_PUKE) return 0;
    if (near_player(nx >> 4, ny >> 4) || npc_at(nx >> 4, ny >> 4, i) != NPC_NONE) return 0;
    n_px[i] = nx;
    n_py[i] = ny;
    n_sx[i] = (unsigned char)(nx >> 9);
    n_sy[i] = (unsigned char)(ny >> 9);
    n_moving[i] = 1;
    n_face[i] = w;
    n_anim[i]++;
    return 1;
}

static void wander(unsigned char i) {
    if (--n_turn[i] == 0) {
        n_way[i] = (unsigned char)(rng() % 6);         /* 4 ways, standing twice as likely */
        if (n_way[i] > STAND) n_way[i] = STAND;
        n_turn[i] = n_kind[i] == NPC_KID ? (unsigned char)(KID_TURN_MIN + rng() % KID_TURN_RND)
                                     : (unsigned char)(NPC_TURN_MIN + rng() % NPC_TURN_RND);
    }
    if (n_way[i] != STAND && !step(i, n_way[i])) n_turn[i] = 1;   /* blocked: another way next frame */
}

/* a puke: 1 if it went into the player's bag (no puddle, no stench) */
static unsigned char puke(unsigned char i) {
    unsigned char t = tile_puke[map_get(n_sx[i], n_sy[i])], caught = 0;
#ifdef PUKE_LOG
    if (puke_logged < 64) puke_log[puke_logged++] = ticks;
#endif
    if (player_bag == BAG_EMPTY && within(i, player_sx, player_sy, BAG_REACH)) {
        player_bag = BAG_FULL;
        caught = 1;
    } else {
        if (t != NO_TILE) map_set(n_sx[i], n_sy[i], t);
        nausea_stink(n_sx[i], n_sy[i], 1);
    }
    n_nausea[i] = (signed char)(NPC_RELIEF_MIN + rng() % NPC_RELIEF_RND);
    n_puke_at[i] = npc_tolerance();
    n_freeze[i] = NPC_FREEZE;
    n_face[i] = DIR_DOWN;
    return caught;
}

unsigned char npc_near(unsigned char x, unsigned char y, unsigned char radius) {
    unsigned char i;
    for (i = 0; i < npc_count; i++)
        if (within(i, x, y, radius)) return 1;
    return 0;
}

/* the aunt's frame: 1 if she hugs the player */
static unsigned char aunt(unsigned char i) {
    signed int dx = player_x() - (n_px[i] >> 4), dy = player_y() - (n_py[i] >> 4);
    signed int ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;
    unsigned char w;
    if (n_rest[i]) n_rest[i]--;
    if (n_chase[i]) {
        if (--n_chase[i] == 0) n_rest[i] = npc_t_rest;
    } else if (!n_rest[i] && within(i, player_sx, player_sy, AUNT_SEE)) n_chase[i] = npc_t_chase;
    if (!n_chase[i]) {
        wander(i);
        return 0;
    }
    if (ax < AUNT_REACH_X && ay < AUNT_REACH_Y) {
        if (player_dizzy()) return 0;
        n_chase[i] = 0;
        n_rest[i] = npc_t_rest;
        return 1;
    }
    /* the longer way first; if that is blocked, the other */
    w = ax > ay ? (dx > 0 ? DIR_RIGHT : DIR_LEFT) : (dy > 0 ? DIR_DOWN : DIR_UP);
    if (!step(i, w) && (ax > ay ? ay : ax) > 2)
        step(i, ax > ay ? (dy > 0 ? DIR_DOWN : DIR_UP) : (dx > 0 ? DIR_RIGHT : DIR_LEFT));
    return 0;
}

unsigned char npc_update(void) {
    unsigned char i, e = 0;
#ifdef PUKE_LOG
    ticks++;
#endif
    for (i = 0; i < npc_count; i++) {
        n_moving[i] = 0;
        if (n_freeze[i]) {                             /* puking, or (the aunt) hit by a bag */
            n_freeze[i]--;
            continue;
        }
        if (n_kind[i] == NPC_AUNT) {
            if (aunt(i)) e |= NE_HUG;
            continue;
        }
        if (n_warn[i]) {                               /* green, standing still */
            if (--n_warn[i] == 0) {
                if (puke(i)) e |= NE_CAUGHT;
                e |= n_kind[i] == NPC_KID ? NE_KID_PUKE : NE_PUKE;
            }
            continue;
        }
        if (--n_drift[i] == 0) {
            n_drift[i] = npc_next_drift(i);
            n_nausea[i]--;
        }
        if (n_nausea[i] <= n_puke_at[i]) {
            n_warn[i] = npc_t_warn_min + rng() % npc_t_warn_rnd;
            continue;
        }
        wander(i);
    }
    return e;
}

void npc_draw(void) {
    unsigned char i, f;
    for (i = 0; i < npc_count; i++) {
        f = n_look[i];
        if (n_kind[i] == NPC_AUNT) f += n_face[i] * 3 + (n_moving[i] ? 1 + ((n_anim[i] >> 3) & 1) : 0);
        else if (n_freeze[i]) f += NPC_PUKE + (n_freeze[i] < NPC_FREEZE / 2);
        else if (n_warn[i]) f += NPC_SICK + n_face[i];
        else {
            f += n_face[i] * 3;
            if (n_moving[i]) f += 1 + ((n_anim[i] >> 3) & 1);
        }
        mux_add(1 + i, f, map_spr_x((n_px[i] >> 4) - 16), map_spr_y((n_py[i] >> 4) - 41));
    }
}
