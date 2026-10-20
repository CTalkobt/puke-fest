/* hazard_int.h — the hazards' state, shared by hazard.c and its set-up in level.c */
#pragma once

#define HZ_MAX   12
#define HZ_KINDS 3
#define HZ_IDLE  0
#define HZ_WARN  1                   /* the warning tile is up, the effect comes */
#define HZ_SHOW  2                   /* it went off, the warning tile stays a little */

extern const unsigned char hz_tile[HZ_KINDS];
extern unsigned char hz_n, hz_x[HZ_MAX], hz_y[HZ_MAX], hz_kind[HZ_MAX], hz_phase[HZ_MAX];
extern unsigned int hz_left[HZ_MAX];
extern unsigned int hz_t_every, hz_t_rnd, hz_t_warn, hz_t_show;
