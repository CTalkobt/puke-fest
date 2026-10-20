/* hazard.c — corn dog stands, teacups, fireworks */
#include "hw.h"
#include "tuning.h"
#include "assets.h"
#include "map.h"
#include "nausea.h"
#include "npc.h"
#include "rng.h"
#include "hazard.h"
#include "hazard_int.h"


const unsigned char hz_tile[HZ_KINDS] = {T_CORNDOG, T_TEACUP, T_LAUNCHER};
static const unsigned char k_warn[HZ_KINDS] = {T_CORNDOG_PUFF, T_TEACUP_SPIN, T_LAUNCHER_LIT};
static const unsigned char k_radius[HZ_KINDS] = {CORNDOG_RADIUS, TEACUP_RADIUS, FIREWORK_RADIUS};
static const signed char k_cost[HZ_KINDS] = {CORNDOG_COST, TEACUP_COST, FIREWORK_COST};

unsigned char hz_n, hz_x[HZ_MAX], hz_y[HZ_MAX], hz_kind[HZ_MAX], hz_phase[HZ_MAX];
unsigned int hz_left[HZ_MAX];
unsigned int hz_t_every, hz_t_rnd, hz_t_warn, hz_t_show;
unsigned char hz_fire_x, hz_fire_y;

unsigned char hazard_update(void) {
    unsigned char i, k, fired = 0;
    for (i = 0; i < hz_n; i++) {
        if (--hz_left[i]) continue;
        k = hz_kind[i];
        switch (hz_phase[i]) {
        case HZ_IDLE:
            map_set(hz_x[i], hz_y[i], k_warn[k]);
            if (k == 2) {                    /* a firework lit */
                fired |= HZ_LIT;
                hz_fire_x = hz_x[i];
                hz_fire_y = hz_y[i];
            } else fired |= HZ_WARNING;
            hz_phase[i] = HZ_WARN;
            hz_left[i] = hz_t_warn;
            break;
        case HZ_WARN:
            nausea_event(hz_x[i], hz_y[i], k_cost[k], k_radius[k]);
            npc_whiff(hz_x[i], hz_y[i], k_radius[k]);
            hz_phase[i] = HZ_SHOW;
            hz_left[i] = hz_t_show;
            fired |= (unsigned char)(1 << k);
            hz_fire_x = hz_x[i];
            hz_fire_y = hz_y[i];
            break;
        default:
            map_set(hz_x[i], hz_y[i], hz_tile[k]);
            hz_phase[i] = HZ_IDLE;
            hz_left[i] = hz_t_every + rng() % hz_t_rnd;
            break;
        }
    }
    return fired;
}
