/* npc_int.h — the crowd's state, shared by npc.c and its set-up in level.c */
#pragma once
#include "tuning.h"

#define DIR_DOWN  0
#define DIR_UP    1
#define DIR_LEFT  2
#define DIR_RIGHT 3
#define STAND     4

extern unsigned char n_kind[NPC_MAX];
extern signed int n_px[NPC_MAX], n_py[NPC_MAX];          /* their feet, 1/16 pixels */
extern unsigned char n_sx[NPC_MAX], n_sy[NPC_MAX];       /* their squares */
extern unsigned char n_way[NPC_MAX], n_face[NPC_MAX], n_moving[NPC_MAX], n_anim[NPC_MAX];
extern unsigned char n_turn[NPC_MAX], n_look[NPC_MAX], n_freeze[NPC_MAX];   /* n_look: the first picture */
extern signed char n_nausea[NPC_MAX], n_puke_at[NPC_MAX];
extern unsigned int n_drift[NPC_MAX], n_warn[NPC_MAX];
extern unsigned int n_chase[NPC_MAX], n_rest[NPC_MAX];   /* the aunt */
extern unsigned int npc_t_drift, npc_t_kid_drift, npc_t_warn_min, npc_t_warn_rnd, npc_t_chase, npc_t_rest;
extern signed int npc_aunt_speed;
signed char npc_tolerance(void);
unsigned int npc_next_drift(unsigned char i);
