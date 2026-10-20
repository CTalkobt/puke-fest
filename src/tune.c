/* tune.c — the music player
 *
 * Each voice reads its byte stream (tools/gen_music.py; the tune is copied
 * from the data in bank 1 into mus_data when it starts) a tick at a time;
 * a tick is mus_fpt[tune] frames, or fewer when the player is very sick.
 * A note restarts the envelope; an instrument's gap releases it that many
 * ticks before the next one. Vibrato is a triangle wave over 8 frames,
 * the instrument's depth plus the sickness'.
 */
#include <audio_sid.h>
#include <dma.h>
#include "hw.h"
#include "assets.h"
#include "tuning.h"
#include "tune.h"
#include "ticks.h"

#define VOICES 3

static unsigned char tune, active;
unsigned char tune_off;
static unsigned char mus_data[TUNE_MAX];     /* the tune playing (from the data, bank 1) */
static unsigned int pos[VOICES], start[VOICES];
#ifdef AUDIO_LOG
unsigned int log_pos[VOICES];               /* (tests: read from a memory dump) */
unsigned char log_tune, log_active;
unsigned int log_ticks;
#endif
static unsigned char left[VOICES], inst[VOICES], gated[VOICES], done[VOICES];
static unsigned int freq[VOICES];
static unsigned int acc, rate = 256;         /* 256: a tick every mus_fpt frames (PAL) */
static unsigned char sick_vib, detune, phase;
static const signed char tri[8] = {0, 1, 2, 1, 0, -1, -2, -1};

static void set_instrument(unsigned char v, unsigned char i) {
    sid_instrument_t s;
    s.wave = ins_wave[i];
    s.ad = ins_ad[i];
    s.sr = ins_sr[i];
    s.pw = ins_pw[i];
    sid_set_instrument(v, &s);
}

void tune_play(unsigned char t) {
    unsigned char v;
    if (tune_off) return;
    tune = t;
    dma_copy_far((unsigned long)(unsigned int)mus_data, DATA_LIN + MUSIC_OFS + mus_voice[t * 3], mus_size[t]);
    for (v = 0; v < VOICES; v++) {
        sid_gate(v, 0);
        start[v] = pos[v] = mus_voice[t * 3 + v] - mus_voice[t * 3];
        left[v] = 1;                         /* the first event on the next tick */
        inst[v] = 0;
        gated[v] = 0;
        done[v] = 0;
    }
    acc = (unsigned int)mus_fpt[t] << 8;     /* start at once */
    tune_sick(0);                            /* not sick yet (play keeps it up to date); PAL / NTSC tempo */
    active = 1;
}

void tune_stop(void) {
    unsigned char v;
    active = 0;
    for (v = 0; v < VOICES; v++) sid_gate(v, 0);
}

unsigned char tune_playing(void) {
    return active;
}

void tune_sick(signed char n) {
    unsigned int base = ticks_hz == 60 ? 213 : 256;   /* NTSC: as fast in time */
    sick_vib = n >= 0 ? 0 : (n > -4 ? 1 : 2);
    if (n <= SWAY_FROM) {
        rate = base + base / 4;              /* 1.25 times as fast... */
        detune = 1;                          /* ...and voice 2 a little off */
    } else {
        rate = base;
        detune = 0;
    }
}

/* the next event of voice v */
static void next(unsigned char v) {
    unsigned char b, guard = 0;
    for (;;) {
        b = mus_data[pos[v]++];
        if (b < 0x80) {
            freq[v] = sid_note_to_reg(b);
            if (v == 1 && detune) freq[v] += freq[v] >> 6;
            sid_gate(v, 0);
            set_instrument(v, inst[v]);
            sid_set_freq_reg(v, freq[v]);
            sid_gate(v, 1);
            gated[v] = 1;
            left[v] = mus_data[pos[v]++];
            return;
        }
        if (b == 0x80) {                     /* a rest */
            sid_gate(v, 0);
            gated[v] = 0;
            left[v] = mus_data[pos[v]++];
            return;
        }
        if (b == 0x81) {
            inst[v] = mus_data[pos[v]++];
            continue;
        }
        if (b == 0xFE && ++guard < 2) {      /* round again */
            pos[v] = start[v];
            continue;
        }
        sid_gate(v, 0);                      /* the end */
        gated[v] = 0;
        done[v] = 1;
        return;
    }
}

static void tick(void) {
    unsigned char v, d = 0;
#ifdef AUDIO_LOG
    log_ticks++;
    log_tune = tune;
    for (v = 0; v < VOICES; v++) log_pos[v] = pos[v];
#endif
    for (v = 0; v < VOICES; v++) {
        if (done[v]) {
            d++;
            continue;
        }
        if (--left[v] == 0) next(v);
        else if (gated[v] && left[v] == ins_gap[inst[v]]) {
            sid_gate(v, 0);                  /* staccato */
            gated[v] = 0;
        }
    }
    if (d == VOICES) active = 0;
#ifdef AUDIO_LOG
    log_active = active;
#endif
}

void tune_update(void) {
    unsigned char v, depth;
    unsigned int whole;
    if (!active) return;
    whole = (unsigned int)mus_fpt[tune] << 8;
    acc += rate;
    while (acc >= whole) {
        acc -= whole;
        tick();
    }
    /* vibrato */
    phase = (phase + 1) & 7;
    for (v = 0; v < VOICES; v++) {
        if (!gated[v]) continue;
        depth = ins_vib[inst[v]] + sick_vib;
        if (depth) {
            signed int dv = (signed int)(freq[v] >> 8) * depth * tri[phase] / 2;
            sid_set_freq_reg(v, freq[v] + dv);
        }
    }
}
