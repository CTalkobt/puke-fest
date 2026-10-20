/* sfx.c — sound effects: SID 2 (mirrored on SID 4), and samples
 *
 * Voice 3: pukes, passing out, puffs (noise); voice 4: pickups, gains,
 * hugs; voice 5: the warning gurgle, losses, the gate. Each is swept once
 * a frame. SID 1 is the music's (tune.c).
 *
 * The samples (in the art and the data loaded from disk) play on the
 * audio DMA channels, in the middle (both sides): channel 0 the player's,
 * 1 the crowd's, 2 splats.
 */
#include <audio_sid.h>
#include "hw.h"
#include "assets.h"
#include "sfx.h"

#define V_NOISE 3
#define V_BLIP  4
#define V_TONE  5

static const sid_instrument_t i_noise = { SID_NOISE, 0x09, 0xA9, 0 };
static const sid_instrument_t i_blip  = { SID_PULSE, 0x05, 0x00, 0x0800 };
static const sid_instrument_t i_gurgle = { SID_TRIANGLE, 0x22, 0x80, 0 };
static const sid_instrument_t i_tone  = { SID_SAWTOOTH, 0x08, 0x00, 0 };

/* a voice's sweep: frames left, frequency, change per frame, (wobble) */
static unsigned char t[6], wob[6];
static unsigned int f[6];
static signed int df[6];

static void start(unsigned char v, const sid_instrument_t *inst, unsigned int freq,
                  signed int step, unsigned char frames, unsigned char wobble) {
    sid_gate(v, 0);
    sid_set_instrument(v, inst);
    sid_set_freq_reg(v, freq);
    sid_gate(v, 1);
    f[v] = freq;
    df[v] = step;
    t[v] = frames;
    wob[v] = wobble;
}

/* load.s45: a sample on an audio DMA channel */
extern unsigned char snd_ch, snd_vol, snd_addr[3], snd_rate[3];
extern unsigned int snd_len;
void snd_on(void);
void snd_off(void);
void snd_play(void);

/* a sample at linear address at */
static void sample(unsigned char ch, unsigned long at, unsigned int len, unsigned long rate, unsigned char vol) {
    snd_ch = ch;
    snd_addr[0] = (unsigned char)at;
    snd_addr[1] = (unsigned char)(at >> 8);
    snd_addr[2] = (unsigned char)(at >> 16);
    snd_len = len;
    snd_rate[0] = (unsigned char)rate;
    snd_rate[1] = (unsigned char)(rate >> 8);
    snd_rate[2] = (unsigned char)(rate >> 16);
    snd_vol = vol;
    snd_play();
}

void sfx_on(void) {
    sid_init();
    sid_set_mirror(1);                       /* SIDs 1-2 (left) on 3-4 (right) too */
    sid_set_volume(15);
    snd_on();
}

void sfx_mute(unsigned char on) {
    sid_set_volume(on ? 0 : 15);
}

void sfx_off(void) {
    snd_off();
    sid_init();
}

void sfx_update(void) {
    unsigned char v;
    for (v = V_NOISE; v <= V_TONE; v++)
        if (t[v]) {
            f[v] += (unsigned int)df[v];
            if (wob[v]) sid_set_freq_reg(v, f[v] + ((t[v] & 2) ? wob[v] * 16 : 0));
            else sid_set_freq_reg(v, f[v]);
            if (--t[v] == 0) sid_gate(v, 0);
        }
}

void sfx_puke(void) {
    sample(0, SND_BLARGH, SND_BLARGH_LEN, SND_BLARGH_RATE, 255);
    start(V_NOISE, &i_noise, 0x1800, -0x60, 32, 0);
}

void sfx_crowd_puke(unsigned char kid) {
    sample(1, SND_BLARGH, SND_BLARGH_LEN, kid ? SND_BLARGH_HIGH : SND_BLARGH_LOW, 150);
}

void sfx_splat(void) { sample(2, SND_SPLAT, SND_SPLAT_LEN, SND_SPLAT_RATE, 220); }
void sfx_eww(void) { sample(3, SND_EWWW, SND_EWWW_LEN, SND_EWWW_RATE, 140); }
void sfx_oof(void) { sample(2, SND_OOF, SND_OOF_LEN, SND_OOF_RATE, 200); }
void sfx_whoosh(void) { sample(2, SND_WHOOSH, SND_WHOOSH_LEN, SND_WHOOSH_RATE, 200); }
void sfx_thud(void) { sample(3, SND_THUD, SND_THUD_LEN, SND_THUD_RATE, 255); }
void sfx_whistle(void) { sample(3, SND_WHISTLE, SND_WHISTLE_LEN, SND_WHISTLE_RATE, 160); }
void sfx_passout(void) { start(V_NOISE, &i_noise, 0x0C00, -0x20, 60, 0); }
void sfx_warn(void) { start(V_TONE, &i_gurgle, 0x0300, 0x08, 40, 40); }
void sfx_pickup(void) { start(V_BLIP, &i_blip, 0x2000, 0x300, 8, 0); }
void sfx_good(void) { start(V_BLIP, &i_blip, 0x1400, 0x180, 14, 0); }
void sfx_bad(void) { start(V_TONE, &i_tone, 0x0A00, -0x40, 14, 0); }
void sfx_gate(void) { start(V_TONE, &i_blip, 0x1000, 0x100, 30, 6); }
void sfx_hug(void) { start(V_BLIP, &i_blip, 0x0E00, 0x120, 24, 20); }
void sfx_puff(void) { start(V_NOISE, &i_noise, 0x0500, 0x18, 28, 0); }
void sfx_alert(void) { start(V_TONE, &i_blip, 0x1800, 0x280, 10, 0); }
void sfx_heart(void) { start(V_NOISE, &i_tone, 0x0280, -0x08, 6, 0); }
void sfx_spin(void) { start(V_TONE, &i_blip, 0x0800, 0x200, 30, 30); }
void sfx_bang(void) { start(V_NOISE, &i_noise, 0x2800, -0x100, 30, 0); }
