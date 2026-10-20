/* mux.c — the sprite multiplexer: who is shown, on which pair, from when
 *
 * A character covers raster lines y / 2 .. (y + 42) / 2 (VIC-II lines;
 * sprite Y is in physical lines). With a margin before (for the interrupt
 * to move the pair) and after, that is its interval [s, e]. Two characters
 * can share a pair when one's interval ends before the other's starts; four
 * pairs show any set in which no line is in more than four intervals.
 *
 * mux_commit takes the characters in order of need - the player first, then
 * whoever has been left out longest - and keeps each one that fits that
 * rule, then hands out the pairs in order of s (greedy interval colouring:
 * it never fails once the rule holds). A pair's first character is placed
 * at the top of the frame (line 0); later ones at their s. The list goes to
 * the interrupt (mux.s45) sorted by line.
 */
#include "hw.h"
#include "assets.h"
#include "mux.h"

#define NPAIRS     4
#define MXMAX      16
#define PIC_H      42
#define MARGIN_TOP 2                 /* lines before a character for moving its pair */
#define MARGIN_END 1
#define FRAME_LINE 16                /* mux.s45: the top-of-frame interrupt (above NTSC's first sprites) */

extern unsigned char mx_line_lo[], mx_line_hi[], mx_reg[], mx_xl0[], mx_xl1[], mx_y[], mx_mask[],
    mx_x8[], mx_x9[], mx_y8[], mx_y9[], mx_p0l[], mx_p0h[], mx_p1l[], mx_p1h[], mx_n[];
extern volatile unsigned char mx_cur, mx_pending, mx_top;
extern unsigned char mx_done_lo[], mx_done_hi[];
void mx_irq(void);

unsigned char mux_dropped;
signed char mux_sway;

/* this frame's characters */
static unsigned char n, id[MXMAX], frame[MXMAX];
static unsigned int x[MXMAX], y[MXMAX], s[MXMAX], e[MXMAX];
static unsigned char waited[MUX_IDS];        /* frames each id has been left out */

/* the chosen, in order of s, and their pairs and lines */
static unsigned char nchosen, chosen[MXMAX], pair[MXMAX];
static unsigned int line[MXMAX];

static unsigned char saved_0314, saved_0315, saved_d01a;

void mux_begin(void) {
    n = 0;
}

void mux_add(unsigned char who, unsigned char f, unsigned int px, unsigned int py) {
    if (n == MXMAX) return;
    id[n] = who;
    frame[n] = f;
    x[n] = px + mux_sway;
    y[n] = py;
    s[n] = py / 2 - MARGIN_TOP;
    e[n] = (py + PIC_H) / 2 + MARGIN_END;
    n++;
}

/* would character c fit with the chosen: no line in more than NPAIRS intervals? */
static unsigned char fits(unsigned char c) {
    unsigned char i, j, k;
    /* the most intervals share a line at the start of one of them */
    for (i = 0; i <= nchosen; i++) {
        unsigned int p = i < nchosen ? s[chosen[i]] : s[c];
        if (p < s[c] || p > e[c]) continue;
        k = 1;                                 /* c itself */
        for (j = 0; j < nchosen; j++)
            if (s[chosen[j]] <= p && p <= e[chosen[j]]) k++;
        if (k > NPAIRS) return 0;
    }
    return 1;
}

static void choose(void) {
    unsigned char order[MXMAX], i, j, t;
    /* the order of need: the player, then the longest left out */
    for (i = 0; i < n; i++) order[i] = i;
    for (i = 1; i < n; i++)
        for (j = i; j > 0; j--) {
            unsigned char a = order[j - 1], b = order[j];
            unsigned char na = id[a] == 0 ? 255 : waited[id[a]], nb = id[b] == 0 ? 255 : waited[id[b]];
            if (na >= nb) break;
            order[j - 1] = b;
            order[j] = a;
        }
    nchosen = 0;
    mux_dropped = 0;
    for (i = 0; i < n; i++) {
        unsigned char c = order[i];
        if (fits(c)) {
            /* insert in order of s */
            for (j = nchosen; j > 0 && s[chosen[j - 1]] > s[c]; j--) chosen[j] = chosen[j - 1];
            chosen[j] = c;
            nchosen++;
            waited[id[c] & (MUX_IDS - 1)] = 0;
        } else {
            t = id[c] & (MUX_IDS - 1);
            if (waited[t] < 254) waited[t]++;
            mux_dropped++;
        }
    }
}

static void assign(void) {
    unsigned int free_after[NPAIRS];
    unsigned char used[NPAIRS], i, p;
    for (p = 0; p < NPAIRS; p++) used[p] = 0;
    for (i = 0; i < nchosen; i++) {
        unsigned char c = chosen[i];
        /* a pair not used yet, else the one free longest */
        unsigned char best = 0xFF;
        for (p = 0; p < NPAIRS; p++) {
            if (!used[p]) { best = p; break; }
            if (free_after[p] < s[c] && (best == 0xFF || free_after[p] < free_after[best])) best = p;
        }
        pair[i] = best;                      /* (fits() makes sure there is one) */
        line[i] = used[best] ? s[c] : 0;
        used[best] = 1;
        free_after[best] = e[c];
    }
}

#ifdef MUX_TRACE
/* check the list in buffer b as the interrupt carried it out: every entry
 * moved, not before its line, before its character's first line, after
 * the pair's previous character ended (read from a memory dump) */
unsigned int mux_lists, mux_entries, mux_missed, mux_early, mux_late, mux_overlap;
unsigned char mux_most;

static void check(unsigned char b) {
    unsigned char k, j, cnt = mx_n[b ? 1 : 0];
    if (!cnt) return;
    mux_lists++;
    if (cnt > mux_most) mux_most = cnt;
    for (k = 0; k < cnt; k++) {
        unsigned char at = b + k;
        unsigned int done = (unsigned int)mx_done_hi[at] << 8 | mx_done_lo[at];
        unsigned int want = (unsigned int)mx_line_hi[at] << 8 | mx_line_lo[at];
        unsigned int top = ((unsigned int)((mx_y8[at] ? 0x100 : 0) | (mx_y9[at] ? 0x200 : 0) | mx_y[at])) / 2;
        mux_entries++;
        if (done == 0xFFFF) { mux_missed++; continue; }
        if (done < want) mux_early++;
        if (done >= top) mux_late++;
        for (j = 0; j < k; j++) {                /* the pair's previous character */
            unsigned char pj = b + j;
            unsigned int ptop, pend;
            if (mx_reg[pj] != mx_reg[at]) continue;
            ptop = (unsigned int)((mx_y8[pj] ? 0x100 : 0) | (mx_y9[pj] ? 0x200 : 0) | mx_y[pj]);
            pend = (ptop + PIC_H) / 2;
            if (done <= pend && want != 0) mux_overlap++;
        }
    }
}
#endif

void mux_commit(void) {
    unsigned char i, j, b, k, c;
    choose();
    assign();
    while (mx_pending) {}                    /* the interrupt has not taken the last one yet */
    b = mx_cur ^ MXMAX;
#ifdef MUX_TRACE
    check(b);
    for (k = 0; k < MXMAX; k++) mx_done_lo[b + k] = mx_done_hi[b + k] = 0xFF;
#endif
    /* sorted by line (the first characters of the pairs, line 0, first) */
    k = 0;
    for (j = 0; j < 2; j++)
        for (i = 0; i < nchosen; i++) {
            unsigned int ptr;
            unsigned char m, at = b + k;
            if ((j == 0) != (line[i] == 0)) continue;
            c = chosen[i];
            m = (unsigned char)(3 << (pair[i] * 2));
            mx_line_lo[at] = (unsigned char)line[i];
            mx_line_hi[at] = (unsigned char)(line[i] >> 8);
            mx_reg[at] = pair[i] * 4;
            mx_xl0[at] = (unsigned char)x[c];
            mx_xl1[at] = (unsigned char)(x[c] + 16);
            mx_y[at] = (unsigned char)y[c];
            mx_mask[at] = m;
            mx_x8[at] = (unsigned char)(((x[c] & 0x100) ? 1 : 0) | (((x[c] + 16) & 0x100) ? 2 : 0)) << (pair[i] * 2);
            mx_x9[at] = (unsigned char)(((x[c] & 0x200) ? 1 : 0) | (((x[c] + 16) & 0x200) ? 2 : 0)) << (pair[i] * 2);
            mx_y8[at] = (y[c] & 0x100) ? m : 0;
            mx_y9[at] = (y[c] & 0x200) ? m : 0;
            /* an image is 384 bytes: 6 pointer units of 64; a picture two */
            if (frame[c] < FRAME_FAR) ptr = (unsigned int)(SPR_LIN / 64) + (unsigned int)frame[c] * 12;
            else ptr = (unsigned int)(DATA_SPR_LIN / 64) + (unsigned int)(frame[c] - FRAME_FAR) * 12;
            mx_p0l[at] = (unsigned char)ptr;
            mx_p0h[at] = (unsigned char)(ptr >> 8);
            mx_p1l[at] = (unsigned char)(ptr + 6);
            mx_p1h[at] = (unsigned char)((ptr + 6) >> 8);
            k++;
        }
    mx_n[b ? 1 : 0] = k;
    mx_pending = 1;
}

void mux_hide(void) {
    n = 0;
    mux_commit();
}

void mux_on(void) {
    unsigned char i;
    __asm__("sei");
    saved_0314 = REG(0x0314);
    saved_0315 = REG(0x0315);
    saved_d01a = REG(0xD01A);
    mx_cur = 0;
    mx_pending = 0;
    mx_n[0] = mx_n[1] = 0;
    mx_top = 1;
    for (i = 0; i < 2 * MXMAX; i++) mx_done_lo[i] = mx_done_hi[i] = 0xFF;
    REG(0xDC0D) = 0x7F;                      /* no CIA 1 (KERNAL) interrupts... */
    (void)REG(0xDC0D);
    REG(0x0314) = (unsigned char)(unsigned int)mx_irq;
    REG(0x0315) = (unsigned char)((unsigned int)mx_irq >> 8);
    REG(0xD012) = FRAME_LINE;                /* ...only the raster */
    REG(0xD011) = REG(0xD011) & 0x7F;
    REG(0xD01A) = 0x01;
    REG(0xD019) = 0x0F;
    __asm__("cli");
}

void mux_off(void) {
    __asm__("sei");
    REG(0xD01A) = saved_d01a & 0x0F;
    REG(0xD019) = 0x0F;
    REG(0x0314) = saved_0314;
    REG(0x0315) = saved_0315;
    REG(0xDC0D) = 0x81;                      /* the KERNAL's timer interrupt again */
    REG(0xD015) = 0;
    __asm__("cli");
}
