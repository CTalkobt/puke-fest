/* hiscore.c — the high scores
 *
 * The table is one block (a version, the scores, the initials as screen
 * codes, the round and screen reached, the date) saved as is in the file
 * PFHI on the program's disk. The disk is
 * the KERNAL's: the raster interrupt is taken out meanwhile (mux_off), and
 * the VIC-IV's registers unlocked again after (the DOS locks them).
 */
#pragma cc45 overlay(scores)
#include <dma.h>
#include "hw.h"
#include "mux.h"
#include "video.h"
#include "hiscore.h"

#define HS_VERSION 3                 /* the file's layout: older ones are replaced */

static struct {
    unsigned char version;
    unsigned long score[HS_COUNT];
    unsigned char name[HS_COUNT][3];     /* screen codes */
    unsigned char round[HS_COUNT];       /* the round reached (1-) */
    unsigned char screen[HS_COUNT];      /* and the screen in it (1-7) */
    unsigned char date[HS_COUNT][3];     /* day, month, year (00-99), from the RTC; month 0: none */
} hs;

extern const char *k_name;
extern unsigned char k_len;
extern void *k_addr, *k_end;
unsigned char k_file(unsigned char save);

static void file(const char *name, unsigned char len) {
    k_name = name;
    k_len = len;
    k_addr = &hs;
    k_end = (unsigned char *)&hs + sizeof hs;
}

static void defaults(void) {
    static const char names[] = s"bobsueannkidpuk";
    unsigned char i, k;
    hs.version = HS_VERSION;
    for (i = 0; i < HS_COUNT; i++) {
        hs.score[i] = (unsigned long)(HS_COUNT - i) * 2000;
        for (k = 0; k < 3; k++) {
            hs.name[i][k] = (unsigned char)names[i * 3 + k];
            hs.date[i][k] = 0;
        }
        hs.round[i] = 1;
        hs.screen[i] = (unsigned char)(HS_COUNT - i + 1);
    }
}

void hs_load(void) {
    file("PFHI", 4);
    if (!k_file(0) || hs.version != HS_VERSION) defaults();
}

void hs_save(void) {
    mux_off();
    file("@0:PFHI", 7);
    k_file(1);
    REG(0xD02F) = 0x47;                      /* the VIC-IV again */
    REG(0xD02F) = 0x53;
    mux_on();
}

unsigned char hs_rank(unsigned long score) {
    unsigned char i;
    for (i = 0; i < HS_COUNT; i++)
        if (score > hs.score[i]) return i;
    return HS_COUNT;
}

/* an RTC register's value: BCD, tens * 10 + units (xemu's month is not
 * BCD: $0A for October - read the same way, it is 10 too) */
static unsigned char number(unsigned char v) {
    return (unsigned char)((v >> 4) * 10 + (v & 15));
}

void hs_insert(unsigned char rank, unsigned long score, const unsigned char *initials,
               unsigned char round, unsigned char screen) {
    unsigned char i, k, rtc[7];
    for (i = HS_COUNT - 1; i > rank; i--) {
        hs.score[i] = hs.score[i - 1];
        for (k = 0; k < 3; k++) {
            hs.name[i][k] = hs.name[i - 1][k];
            hs.date[i][k] = hs.date[i - 1][k];
        }
        hs.round[i] = hs.round[i - 1];
        hs.screen[i] = hs.screen[i - 1];
    }
    hs.score[rank] = score;
    hs.round[rank] = round;
    hs.screen[rank] = screen;
    dma_copy_far((unsigned long)(unsigned int)rtc, RTC_LIN, 7);   /* the date from the RTC */
    hs.date[rank][0] = number(rtc[3] & 0x3F);
    hs.date[rank][1] = number(rtc[4] & 0x1F);
    hs.date[rank][2] = number(rtc[5]);
    for (k = 0; k < 3; k++) hs.name[rank][k] = initials[k];
}

/* two digits at p (or "--") */
static void digits(char *p, unsigned char v, unsigned char ok) {
    p[0] = ok ? (char)('0' + v / 10) : '-';
    p[1] = ok ? (char)('0' + v % 10) : '-';
}

/* a line: "1. bob  010000  r1 s7  19.10.26" */
void hs_show(unsigned char row) {
    unsigned char i, k, ok;
    char line[32];
    unsigned long v;
    for (i = 0; i < HS_COUNT; i++, row += 2) {
        for (k = 0; k < 31; k++) line[k] = ' ';
        line[0] = (char)('1' + i);
        line[1] = '.';
        for (k = 0; k < 3; k++) line[3 + k] = (char)hs.name[i][k];
        v = hs.score[i];
        for (k = 14; k > 8; k--) {
            line[k - 1] = (char)('0' + (unsigned char)(v % 10));
            v /= 10;
        }
        line[15] = 18;                       /* r */
        line[16] = (char)('0' + hs.round[i]);
        line[18] = 19;                       /* s */
        line[19] = (char)('0' + hs.screen[i]);
        ok = hs.date[i][1] != 0;
        digits(line + 22, hs.date[i][0], ok);
        line[24] = '.';
        digits(line + 25, hs.date[i][1], ok);
        line[27] = '.';
        digits(line + 28, hs.date[i][2], ok);
        line[30] = 0;
        video_text(25, row, line, i ? C_WHITE : C_YELLOW);
    }
}
