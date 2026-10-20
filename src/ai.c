/* ai.c — the player played by the machine
 *
 * Every AI_THINK frames, a breadth-first search over the squares from the
 * player's finds the nearest ticket (or, once they are all taken, the open
 * gate; or a paper bag, while it has none), walking around solid squares and - when it can - puddles, and
 * notes the first square of the way there. In between it walks to that
 * square's middle. When it has not moved for a while it tries a random way.
 *
 * It only plays in the demo and attract mode: the overlay "ai" (copied in
 * once and kept while it plays; its tables, BSS, stay in bank 0).
 */
#pragma cc45 overlay(ai)
#include "hw.h"
#include "tuning.h"
#include "assets.h"
#include "ctl.h"
#include "map.h"
#include "player.h"
#include "rng.h"
#include "ai.h"

#define AI_THINK  12                 /* frames between searches */
#define FAR       0xFF

static unsigned char first[MAP_MAX];     /* each square reached: the first step toward it */
static unsigned char queue[MAP_MAX];
static unsigned char goal_x, goal_y, think, stuck, wander, wander_ctl;
static signed int last_x, last_y;
static unsigned char last_ctl;

void ai_reset(void) {
    think = stuck = wander = 0;
    goal_x = FAR;
}

static unsigned char wanted(unsigned char a, unsigned char t) {
    if (t == T_BAG && player_bag == BAG_NONE) return 1;    /* a bag first, if it is nearer */
    return map_tickets ? (a & A_TICKET) != 0 : (a & A_EXIT) != 0;
}

/* the search: 1 if a goal was found (avoiding puddles if avoid) */
static unsigned char search(unsigned char avoid) {
    unsigned char head = 0, tail = 0, at, x, y, k, n, a;
    unsigned int i;
    for (i = 0; i < (unsigned int)map_w * map_h; i++) first[i] = FAR;
    at = player_sy * map_w + player_sx;
    first[at] = at;
    queue[tail++] = at;
    while (head != tail) {
        at = queue[head++];
        x = at % map_w;
        y = at / map_w;
        for (k = 0; k < 4; k++) {
            unsigned char nx = x, ny = y;
            if (k == 0) { if (!x) continue; nx--; }
            if (k == 1) { if (x + 1 == map_w) continue; nx++; }
            if (k == 2) { if (!y) continue; ny--; }
            if (k == 3) { if (y + 1 == map_h) continue; ny++; }
            n = ny * map_w + nx;
            if (first[n] != FAR) continue;
            a = map_attr(nx, ny);
            if ((a & A_SOLID) || (avoid && (a & A_PUKE))) continue;
            /* the first step: this square if we came from the start, else its parent's */
            first[n] = at == player_sy * map_w + player_sx ? n : first[at];
            if (wanted(a, map_get(nx, ny))) {
                goal_x = first[n] % map_w;
                goal_y = first[n] / map_w;
                return 1;
            }
            queue[tail++] = n;
        }
    }
    return 0;
}

unsigned char ai_read(void) {
    signed int px = player_x(), py = player_y(), dx, dy;
    unsigned char c = 0;
    if (wander) {
        wander--;
        return wander_ctl;
    }
    if (player_bumped()) {                   /* someone in the way: step aside */
        wander = 16;
        wander_ctl = (last_ctl & (C_LEFT | C_RIGHT)) ? ((rng() & 1) ? C_UP : C_DOWN)
                                                     : ((rng() & 1) ? C_LEFT : C_RIGHT);
        return wander_ctl;
    }
    if (px == last_x && py == last_y && !player_dizzy() && !player_puking()) {
        if (++stuck > 20) {                  /* stuck: a random way for a moment */
            stuck = 0;
            wander = 20;
            wander_ctl = (unsigned char)(1 << (rng() & 3));
            return wander_ctl;
        }
    } else stuck = 0;
    last_x = px;
    last_y = py;
    if (!think-- || goal_x == FAR) {
        think = AI_THINK;
        if (!search(1) && !search(0)) goal_x = FAR;
    }
    if (goal_x == FAR) return 0;
    dx = (signed int)goal_x * SQ + SQ / 2 - px;
    dy = (signed int)goal_y * SQ + SQ * 3 / 4 - py;
    if (dx < -2) c |= C_LEFT;
    if (dx > 2) c |= C_RIGHT;
    if (dy < -2) c |= C_UP;
    if (dy > 2) c |= C_DOWN;
    if (!c) think = 0;                       /* there: the next step now */
    if (player_bag == BAG_FULL && !(rng() & 31)) c |= C_FIRE;   /* now and then: throw it */
    last_ctl = c;
    return c;
}
