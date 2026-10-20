/* nausea_test.c — the nausea rules (src/nausea.c), on the host, frame by
 * frame at 50 Hz: make test */
#include <stdio.h>
#include <stdlib.h>
unsigned char player_sx, player_sy;
void npc_smell(unsigned char x, unsigned char y) { (void)x; (void)y; }
#include "../src/ticks.c"
#include "../src/nausea.c"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { printf("FAIL line %d: ", __LINE__); printf(__VA_ARGS__); printf("\n"); fails++; } } while (0)

/* run n frames; count events */
static int run(int n, int *pukes, int *warns) {
    int last = 0;
    while (n--) {
        int z = nausea_tick();
        if (z == NZ_PUKE) {
            (*pukes)++;
            nausea_add(PUKE_GAIN);          /* what main.c does */
            nausea_stink(player_sx, player_sy, 0);
        }
        if (z == NZ_WARN) (*warns)++;
        last = z;
    }
    return last;
}

int main(void) {
    int p = 0, w = 0, f;
    nausea_timing();

    /* 1: from -3 he pukes after the 1 s warning, +1 -> -2; walks away; recovers */
    nausea_reset(); nausea = -3; player_sx = 5; player_sy = 5;
    for (f = 1; f <= 51 && !p; f++) run(1, &p, &w);
    CHECK(p == 1 && w == 1, "puke after warning (p=%d w=%d)", p, w);
    CHECK(f - 1 == 51, "puke at frame 51, was %d", f - 1);
    CHECK(nausea == -2, "-2 after puking, was %d", nausea);
    player_sx = 9;                           /* 4 squares away: out of the stench */
    run(100, &p, &w);
    CHECK(nausea == -2, "walked away: still -2, was %d", nausea);
    run(400, &p, &w);                        /* 10 s at -2 (since the puke) */
    CHECK(nausea == -1, "recovered to -1 after 10 s, was %d", nausea);
    run(500, &p, &w);
    CHECK(nausea == 0, "recovered to 0, was %d", nausea);
    run(1000, &p, &w);
    CHECK(nausea == 0, "no recovery past 0, was %d", nausea);

    /* 2: stays put: the stench takes the +1 back 2 s later */
    nausea_reset(); nausea = -3; p = w = 0; player_sx = 5;
    run(51, &p, &w);
    CHECK(nausea == -2, "-2 after puking, was %d", nausea);
    player_sx = 8;                           /* 3 squares: still in it */
    run(100, &p, &w);
    CHECK(nausea == -3, "stench at 3 squares: -3, was %d", nausea);

    /* 3: below -2 no recovery; the next puke a minute after the last */
    run(500, &p, &w);
    CHECK(nausea == -3, "no recovery at -3, was %d", nausea);
    p = w = 0;
    run(3000 - 600 - 1, &p, &w);            /* 600 frames since the puke so far */
    CHECK(p == 0, "no puke within the minute (%d)", p);
    run(1, &p, &w);
    CHECK(p == 1, "puke once the minute is up (p=%d)", p);

    /* 4: at -1 he never pukes; at -2 he does */
    nausea_reset(); nausea = -1; p = w = 0;
    run(400, &p, &w);
    CHECK(p == 0 && w == 0, "-1: no puke");
    nausea_reset(); nausea = -2; p = w = 0;
    run(60, &p, &w);
    CHECK(p == 1, "-2: pukes (%d)", p);

    /* 5: a warning is called off when he gets better in time */
    nausea_reset(); nausea = -2; p = w = 0;
    run(10, &p, &w);
    CHECK(w == 1 && nausea_warning(), "warning on");
    nausea_add(1);
    run(60, &p, &w);
    CHECK(p == 0 && !nausea_warning(), "called off at -1 (p=%d)", p);

    /* 6: someone else's puke 2 squares away: -1 two seconds later, no +1 */
    nausea_reset(); nausea = 3; player_sx = 5; player_sy = 5;
    nausea_stink(7, 3, 1);
    run(99, &p, &w);
    CHECK(nausea == 3, "not yet");
    run(1, &p, &w);
    CHECK(nausea == 2, "others' stench -1, was %d", nausea);

    /* 7: passing out */
    nausea_reset(); nausea_add(-20);
    CHECK(nausea == NAUSEA_PASSOUT && nausea_out(), "clamped at -10, out");

    printf(fails ? "%d FAILED\n" : "all passed\n", fails);
    return fails != 0;
}
