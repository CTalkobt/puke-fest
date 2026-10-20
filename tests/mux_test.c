/* mux_test.c — the multiplexer's scheduler (src/mux.c), on the host: make test
 *
 * Plays the interrupt's part (takes each list as it would) and checks every
 * list: the player is in it, no raster line has more than 4 characters,
 * a pair's characters do not overlap and each is placed at its line or
 * the top of the frame, the list is in order of lines; and that when too
 * many crowd together, everyone gets their turn.
 */
#include <stdio.h>
#include <stdlib.h>
#define __asm__(x)
#include "../src/mux.c"

unsigned char mx_line_lo[32], mx_line_hi[32], mx_reg[32], mx_xl0[32], mx_xl1[32], mx_y[32], mx_mask[32],
    mx_x8[32], mx_x9[32], mx_y8[32], mx_y9[32], mx_p0l[32], mx_p0h[32], mx_p1l[32], mx_p1h[32], mx_n[2];
volatile unsigned char mx_cur, mx_pending, mx_top;
unsigned char mx_done_lo[32], mx_done_hi[32];
unsigned int spr_ptr[8];
void mx_irq(void) {}

static int fails;
#define CHECK(c, ...) do { if (!(c)) { printf("FAIL line %d: ", __LINE__); printf(__VA_ARGS__); printf("\n"); fails++; } } while (0)

/* the list the interrupt would show next: check it, return who is in it */
static unsigned int take(void) {
    unsigned char b, cnt, k, j;
    unsigned int shown = 0, ys[MXMAX], lines[MXMAX], prev = 0;
    CHECK(mx_pending, "a list is ready");
    mx_cur ^= MXMAX;                         /* what the interrupt does */
    mx_pending = 0;
    b = mx_cur;
    cnt = mx_n[b ? 1 : 0];
    for (k = 0; k < cnt; k++) {
        unsigned char at = b + k;
        ys[k] = (mx_y9[at] ? 0x200 : 0) | (mx_y8[at] ? 0x100 : 0) | mx_y[at];
        lines[k] = (unsigned int)mx_line_hi[at] << 8 | mx_line_lo[at];
        CHECK(lines[k] >= prev, "in order of lines");
        prev = lines[k];
        CHECK(lines[k] == 0 || lines[k] == ys[k] / 2 - MARGIN_TOP, "placed at its line or the top");
        for (j = 0; j < k; j++)
            if (mx_reg[b + j] == mx_reg[at]) {
                CHECK(lines[k] != 0, "a pair's second character waits for its line");
                CHECK((ys[j] + PIC_H) / 2 + MARGIN_END < lines[k] || ys[j] > ys[k],
                      "pair %d: %u..%u then %u", mx_reg[at] / 4, ys[j], ys[j] + PIC_H, ys[k]);
            }
    }
    /* no line in more than 4 */
    for (k = 0; k < cnt; k++) {
        unsigned int l = ys[k] / 2, n4 = 0;
        for (j = 0; j < cnt; j++)
            if (ys[j] / 2 - MARGIN_TOP <= l && l <= (ys[j] + PIC_H) / 2 + MARGIN_END) n4++;
        CHECK(n4 <= 4, "%u characters on line %u", n4, l);
    }
    /* who: the pointers say which picture; the tests give each id its own frame */
    for (k = 0; k < cnt; k++) {
        unsigned int ptr = mx_p0l[b + k] | mx_p0h[b + k] << 8;
        unsigned int f = (ptr - (unsigned int)(SPR_LIN / 64)) / 12;
        shown |= 1u << f;
    }
    return shown;
}

int main(void) {
    int frame, i, t;
    unsigned int shown, last[MUX_IDS];

    /* 1: the player and 7 more on one line: 4 at a time, everyone in turn */
    for (i = 0; i < MUX_IDS; i++) last[i] = 0;
    for (frame = 1; frame <= 30; frame++) {
        mux_begin();
        for (i = 0; i < 8; i++) mux_add(i, i, 100 + i * 40, 300);
        mux_commit();
        shown = take();
        CHECK(shown & 1, "the player is always shown");
        CHECK(__builtin_popcount(shown) == 4, "4 shown, not %d", __builtin_popcount(shown));
        for (i = 0; i < 8; i++) if (shown & (1u << i)) last[i] = frame;
        if (frame >= 3)
            for (i = 0; i < 8; i++)
                CHECK(frame - last[i] < 3, "frame %d: %d not shown since %u", frame, i, last[i]);
    }

    /* 2: random crowds, 12 characters anywhere on the play area */
    srand(65);
    for (t = 0; t < 20000; t++) {
        mux_begin();
        for (i = 0; i < 12; i++) mux_add(i, i, 48 + rand() % 600, 108 + rand() % 360);
        mux_commit();
        shown = take();
        CHECK(shown & 1, "the player is always shown");
        CHECK(__builtin_popcount(shown) + mux_dropped == 12, "everyone shown or dropped");
    }

    /* 3: spread out down the screen, all 12 fit */
    mux_begin();
    for (i = 0; i < 12; i++) mux_add(i, i, 60 + i * 30, 108 + i * 30);
    mux_commit();
    shown = take();
    CHECK(shown == 0xFFF && mux_dropped == 0, "12 in a diagonal: all shown (%x)", shown);

    /* 4: nothing to show */
    mux_hide();
    CHECK(take() == 0 && mx_n[mx_cur ? 1 : 0] == 0, "an empty list");

    printf(fails ? "%d FAILED\n" : "all passed\n", fails);
    return fails != 0;
}
