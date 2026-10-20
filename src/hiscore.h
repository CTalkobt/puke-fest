/* hiscore.h — the five strongest stomachs, kept on the disk (PFHI) */
#pragma once

#define HS_COUNT 5

void hs_load(void);                  /* from the disk, or the defaults */
void hs_save(void);
unsigned char hs_rank(unsigned long score);   /* its place 0-4, or HS_COUNT */
void hs_insert(unsigned char rank, unsigned long score, const unsigned char *initials,
               unsigned char round, unsigned char screen);   /* (1-) */
void hs_show(unsigned char row);     /* the table, a line each from row */
