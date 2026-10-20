/* player.c — the player
 *
 * His position is the middle of his feet, in 1/16 pixels of the map. He
 * collides with a small box around his feet against the solid squares, and
 * slides around corners within NUDGE pixels. He changes square when his
 * feet do: that is when he picks things up and steps into puddles.
 */
#include "hw.h"
#include "tuning.h"
#include "ctl.h"
#include "map.h"
#include "mux.h"
#include "player.h"
#include "npc.h"
#include "ticks.h"

#define DIR_DOWN  0
#define DIR_UP    1
#define DIR_LEFT  2
#define DIR_RIGHT 3

unsigned char player_sx, player_sy, player_bag;
static signed int px, py;            /* 1/16 pixels */
static unsigned char dir, anim, moving;
static unsigned char freeze;         /* frames left puking */
static unsigned char stun;           /* frames left dizzy */
static unsigned char slip;           /* frames left sliding */
static signed char slip_dx, slip_dy;
static unsigned int bench, t_bench;
static unsigned char bumped, bump_gap;    /* walked into someone (this frame); frames to the next bump */

void player_timing(void) {
    t_bench = t_of(BENCH_FRAMES);
}

void player_reset(unsigned char sx, unsigned char sy) {
    px = ((signed int)sx * SQ + SQ / 2) * 16;
    py = ((signed int)sy * SQ + SQ * 3 / 4) * 16;
    player_sx = sx;
    player_sy = sy;
    dir = DIR_DOWN;
    anim = moving = freeze = slip = stun = bump_gap = 0;
    player_bag = BAG_NONE;
    bench = 0;
}

/* move by (dx, dy) 1/16 pixels, an axis at a time, sliding around corners */
/* is someone in the way of feet at (x, y)? */
static unsigned char person(signed int x, signed int y) {
    unsigned char i = npc_at(x, y, NPC_NONE);
    if (i == NPC_NONE) return 0;
    if (!bumped && !bump_gap) npc_make_way(i);
    bumped = 1;
    return 1;
}

static void move(signed int dx, signed int dy) {
    signed int x = px >> 4, y = py >> 4, k;
    if (dx) {
        signed int nx = (px + dx) >> 4;
        if (person(nx, y)) {}
        else if (map_clear(nx, y)) px += dx;
        else if (!dy)
            for (k = 1; k <= NUDGE; k++) {
                if (map_clear(nx, y - k) && map_clear(x, y - 1)) { py -= 16; break; }
                if (map_clear(nx, y + k) && map_clear(x, y + 1)) { py += 16; break; }
            }
    }
    x = px >> 4;
    y = py >> 4;
    if (dy) {
        signed int ny = (py + dy) >> 4;
        if (person(x, ny)) {}
        else if (map_clear(x, ny)) py += dy;
        else if (!dx)
            for (k = 1; k <= NUDGE; k++) {
                if (map_clear(x - k, ny) && map_clear(x - 1, y)) { px -= 16; break; }
                if (map_clear(x + k, ny) && map_clear(x + 1, y)) { px += 16; break; }
            }
    }
}

/* he has just stepped onto the square under his feet */
static unsigned char arrive(void) {
    unsigned char a = map_attr(player_sx, player_sy), e = 0;
    unsigned char t = map_get(player_sx, player_sy);
    if (a & A_TICKET) e = PE_TICKET;
    if (a & A_MINT) e = PE_MINT;
    if (a & A_ALE) e = PE_ALE;
    if (t == T_BAG && player_bag == BAG_NONE) {
        e = PE_BAG;
        player_bag = BAG_EMPTY;
    }
    if (e) map_set(player_sx, player_sy, tile_under[t]);
    if (a & A_EXIT) e |= PE_EXIT;
    if ((a & A_PUKE) && !slip) {
        e |= PE_PUDDLE;
        slip = SLIP_FRAMES;
    }
    bench = 0;
    return e;
}

unsigned char player_update(unsigned char c) {
    signed int dx = 0, dy = 0, s;
    unsigned char sx, sy, e = 0;
    if (freeze) {
        freeze--;
        return 0;
    }
    if (stun) {
        stun--;
        moving = 0;
        return 0;
    }
    if (slip) {
        slip--;
        dx = slip_dx * SPEED;
        dy = slip_dy * SPEED;
    } else {
        s = ((c & (C_LEFT | C_RIGHT)) && (c & (C_UP | C_DOWN))) ? SPEED_DIAG : SPEED;
        if (c & C_LEFT) { dx = -s; dir = DIR_LEFT; }
        if (c & C_RIGHT) { dx = s; dir = DIR_RIGHT; }
        if (c & C_UP) { dy = -s; dir = DIR_UP; }
        if (c & C_DOWN) { dy = s; dir = DIR_DOWN; }
        slip_dx = dx < 0 ? -1 : dx > 0;
        slip_dy = dy < 0 ? -1 : dy > 0;
    }
    moving = dx || dy;
    if (bump_gap) bump_gap--;
    if (moving) {
        anim++;
        bumped = 0;
        move(dx, dy);
        if (bumped && !slip && !bump_gap) {  /* knocked back the way he came */
            slip_dx = -slip_dx;
            slip_dy = -slip_dy;
            slip = BUMP_FRAMES;
            bump_gap = BUMP_GAP;
            e |= PE_BUMP;
        }
    }
    sx = (unsigned char)(px >> 9);      /* 1/16 pixels -> squares of 32 */
    sy = (unsigned char)(py >> 9);
    if (sx != player_sx || sy != player_sy) {
        player_sx = sx;
        player_sy = sy;
        e = arrive();
    }
    if ((map_attr(sx, sy) & A_BENCH) && !moving) {
        if (++bench >= t_bench) {
            bench = 0;
            e |= PE_BENCH;
        }
    } else bench = 0;
    return e;
}

void player_puke(unsigned char into_bag) {
    unsigned char t = tile_puke[map_get(player_sx, player_sy)];
    freeze = PUKE_FREEZE;
    slip = 0;
    if (!into_bag && t != NO_TILE) map_set(player_sx, player_sy, t);
}

void player_stun(unsigned char frames) {
    stun = frames;
    slip = 0;
}

unsigned char player_bumped(void) {
    return bump_gap == BUMP_GAP;
}

unsigned char player_dizzy(void) {
    return stun != 0;
}

signed int player_x(void) {
    return px >> 4;
}

signed int player_y(void) {
    return py >> 4;
}

unsigned char player_puking(void) {
    return freeze != 0;
}

void player_draw(unsigned char sick, unsigned char flash) {
    unsigned char f;
    if (freeze) f = FRAME_PUKE + (freeze < PUKE_FREEZE / 2);
    else {
        f = dir * 3;
        if (moving) f += 1 + ((anim / ANIM_FRAMES) & 1);
        if (flash) sick = (FRAMECOUNT & 4) != 0;   /* a puke is coming: flashing */
        if (stun) sick = (FRAMECOUNT & 8) != 0;    /* dizzy */
        if (sick) f += FRAME_SICK;
    }
    /* the picture is 32 x 42, his feet in the middle of its bottom row */
    mux_add(0, f, map_spr_x((px >> 4) - 16), map_spr_y((py >> 4) - 41));
}
