/* bag.c — the full paper bag in the air
 *
 * Thrown at the nearest person within BAG_RANGE squares, it flies at
 * BAG_SPEED pixels a frame toward where they are now (they cannot dodge),
 * and hits: BAG_COST to their nausea, or the aunt stands stunned.
 */
#include "hw.h"
#include "tuning.h"
#include "assets.h"
#include "map.h"
#include "mux.h"
#include "npc.h"
#include "npc_int.h"
#include "player.h"
#include "bag.h"

#define BAG_ID 15                    /* the multiplexer's id */

static unsigned char flying, target, spin;
static signed int bx, by;            /* pixels of the map */
#ifdef PUKE_LOG
unsigned int log_flight;             /* (tests) frames in the air */
signed int log_dx, log_dy;
#endif

void bag_reset(void) {
    flying = 0;
}

unsigned char bag_throw(void) {
    unsigned char i, d, dx, dy, best = BAG_RANGE + 1;
    if (flying || player_bag != BAG_FULL) return 0;
    for (i = 0; i < npc_count; i++) {
        dx = n_sx[i] > player_sx ? n_sx[i] - player_sx : player_sx - n_sx[i];
        dy = n_sy[i] > player_sy ? n_sy[i] - player_sy : player_sy - n_sy[i];
        d = dx > dy ? dx : dy;
        if (d < best) {
            best = d;
            target = i;
        }
    }
    if (best > BAG_RANGE) return 0;
    player_bag = BAG_NONE;
    flying = 1;
    bx = player_x();
    by = player_y() - 20;                /* from his hands */
    return 1;
}

unsigned char bag_update(void) {
    signed int tx, ty, dx, dy;
    if (!flying) return 0;
    spin++;
    tx = n_px[target] >> 4;
    ty = (n_py[target] >> 4) - 20;       /* at their middle */
    dx = tx - bx;
    dy = ty - by;
#ifdef PUKE_LOG
    log_flight++;
    log_dx = dx;
    log_dy = dy;
#endif
    if (dx > -BAG_SPEED && dx < BAG_SPEED && dy > -BAG_SPEED && dy < BAG_SPEED) {
        flying = 0;
        if (n_kind[target] == NPC_AUNT) {
            n_freeze[target] = AUNT_BAG_STUN;
            n_chase[target] = 0;
            n_rest[target] = npc_t_rest;
        } else if (n_nausea[target] > -9 - BAG_COST) n_nausea[target] += BAG_COST;
        return 1;
    }
    bx += dx > BAG_SPEED ? BAG_SPEED : dx < -BAG_SPEED ? -BAG_SPEED : dx;
    by += dy > BAG_SPEED ? BAG_SPEED : dy < -BAG_SPEED ? -BAG_SPEED : dy;
    return 0;
}

void bag_draw(void) {
    if (flying) mux_add(BAG_ID, FRAME_BAG + ((spin >> 3) & 1), map_spr_x(bx - 16), map_spr_y(by - 21));
}
