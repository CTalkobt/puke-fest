/* nausea.c — the nausea rules
 *
 * At -1 and -2 the player gets better by himself, +1 every RECOVER_FRAMES
 * up to 0. Below that only puking (or items) helps: at PUKE_AT or below he
 * pukes, after a warning, at most once every PUKE_COOLDOWN frames. Puking
 * gives +PUKE_GAIN at once; STINK_DELAY later the stench costs PUKE_COST
 * if he is still within PUKE_RADIUS squares, so he can walk away from it.
 * Everyone's pukes stink the same, for him and for the crowd (npc.c).
 */
#include "tuning.h"
#include "nausea.h"
#include "player.h"
#include "npc.h"
#include "ticks.h"

#define NSTINK 16                    /* stenches on the way at once */

#define SK_NPC  1                    /* stink_how: a crowd member's puke */
#define SK_NEAR 2                    /*   the player was near it then */

signed char nausea;
unsigned char nausea_pukes;
static unsigned int t_recover, t_cooldown, t_warn, t_stink;
static unsigned int recover_left, cooldown_left, warn_left;
static unsigned char stink_x[NSTINK], stink_y[NSTINK], stink_how[NSTINK];
static unsigned int stink_left[NSTINK];

void nausea_timing(void) {
    t_recover = t_of(RECOVER_FRAMES);
    t_cooldown = t_of(PUKE_COOLDOWN);
    t_warn = t_of(PUKE_WARN);
    t_stink = t_of(STINK_DELAY);
}

void nausea_reset(void) {
    unsigned char i;
    nausea = NAUSEA_START;
    nausea_pukes = 0;
    recover_left = t_recover;
    cooldown_left = warn_left = 0;
    for (i = 0; i < NSTINK; i++) stink_left[i] = 0;
}

void nausea_add(signed char amount) {
    signed int n = (signed int)nausea + amount;
    if (n > NAUSEA_MAX) n = NAUSEA_MAX;
    if (n < NAUSEA_PASSOUT) n = NAUSEA_PASSOUT;
    nausea = (signed char)n;
}

static unsigned char dist(unsigned char a, unsigned char b) {
    return a > b ? a - b : b - a;
}

static unsigned char near(unsigned char x, unsigned char y, unsigned char radius) {
    unsigned char dx = dist(x, player_sx), dy = dist(y, player_sy);
    return (dx > dy ? dx : dy) <= radius;
}

void nausea_event(unsigned char x, unsigned char y, signed char amount, unsigned char radius) {
    if (near(x, y, radius)) nausea_add(amount);
}

void nausea_stink(unsigned char x, unsigned char y, unsigned char npc) {
    unsigned char i;
    for (i = 0; i < NSTINK; i++)
        if (!stink_left[i]) {
            stink_x[i] = x;
            stink_y[i] = y;
            stink_how[i] = (npc ? SK_NPC : 0) | (near(x, y, PUKE_RADIUS) ? SK_NEAR : 0);
            stink_left[i] = t_stink;
            return;
        }
}

unsigned char nausea_tick(void) {
    unsigned char i, e = 0;
    for (i = 0; i < NSTINK; i++)
        if (stink_left[i] && --stink_left[i] == 0) {
            npc_smell(stink_x[i], stink_y[i]);
            if (near(stink_x[i], stink_y[i], PUKE_RADIUS)) {
                nausea_add(PUKE_COST);
                e = NZ_STINK;
            } else if (stink_how[i] == (SK_NPC | SK_NEAR)) e = NZ_DODGE;
        }
    if (cooldown_left) cooldown_left--;

    if (nausea < 0 && nausea >= RECOVER_FROM) {
        if (--recover_left == 0) {
            recover_left = t_recover;
            nausea_add(1);
            e = NZ_RECOVER;
        }
    } else recover_left = t_recover;

    if (nausea <= PUKE_AT && !cooldown_left) {
        if (!warn_left) {
            warn_left = t_warn;
            return NZ_WARN;
        }
        if (--warn_left == 0) {
            cooldown_left = t_cooldown - t_warn;   /* the next warning starts inside the minute */
            nausea_pukes++;
            return NZ_PUKE;
        }
    } else warn_left = 0;
    return e;
}

unsigned char nausea_out(void) {
    return nausea <= NAUSEA_PASSOUT;
}

unsigned int nausea_puke_in(void) {
    if (warn_left) return warn_left;
    return cooldown_left + t_warn;
}

unsigned char nausea_warning(void) {
    return warn_left != 0;
}
