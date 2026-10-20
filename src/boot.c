/* boot.c — starting up and shutting down (the overlay "boot": it is only
 * in bank 0 while it runs)
 */
#pragma cc45 overlay(boot)
#include <dma.h>
#include <input.h>
#include "hw.h"
#include "tuning.h"
#include "assets.h"
#include "ai.h"
#include "ctl.h"
#include "hazard.h"
#include "hiscore.h"
#include "map.h"
#include "mux.h"
#include "nausea.h"
#include "npc.h"
#include "player.h"
#include "rng.h"
#include "sfx.h"
#include "spr.h"
#include "ticks.h"
#include "tune.h"
#include "video.h"
#include "game.h"

/* the hardware sprites: 16-colour (16 pixels wide, 8 bytes a row), 42
 * rows high, at the native resolution (SPR640, SPRENV400), with 16-bit
 * pointers to the images (SPR_LIN, DATA_SPR_LIN); coordinates are
 * physical pixels: X has its 9th and 10th bits in $D010 and $D05F, Y in
 * $D077 and $D078. Sprite n's colours are entries n * 16 + 0-15 of the
 * sprite palette bank (the same for every sprite); 0 is transparent. The
 * multiplexer (mux.c, mux.s45) places them. */
#define HEIGHT 42

_Alignas(16) unsigned int spr_ptr[8];        /* the pointers; mux.s45 writes them */
static unsigned char spr_saved[10];

void spr_on(void) {
    unsigned char n;
    spr_saved[0] = REG(0xD06C); spr_saved[1] = REG(0xD06D); spr_saved[2] = REG(0xD06E); spr_saved[3] = REG(0xD06B);
    spr_saved[4] = REG(0xD057); spr_saved[5] = REG(0xD01B); spr_saved[6] = REG(0xD055); spr_saved[7] = REG(0xD056);
    spr_saved[8] = REG(0xD076); spr_saved[9] = REG(0xD05F);
    REG(0xD015) = 0;
    for (n = 0; n < 8; n++) REG(0xD027 + n) = 0;     /* transparent colour 0 */
    REG(0xD06C) = (unsigned char)(unsigned int)spr_ptr;
    REG(0xD06D) = (unsigned char)((unsigned int)spr_ptr >> 8);
    REG(0xD06E) = 0x80;                              /* bank 0, 16-bit pointers */
    REG(0xD06B) = 0xFF;                              /* all 16-colour... */
    REG(0xD057) = 0xFF;                              /* ...8 bytes a row */
    REG(0xD055) = 0xFF;                              /* their own height */
    REG(0xD056) = HEIGHT;
    REG(0xD076) = 0xFF;                              /* a row is a raster line */
    REG(0xD01B) = 0;                                 /* in front of the screen */
    REG(0xD017) = 0;
    REG(0xD01D) = 0;
}

void spr_off(void) {
    REG(0xD015) = 0;
    REG(0xD06C) = spr_saved[0]; REG(0xD06D) = spr_saved[1]; REG(0xD06E) = spr_saved[2]; REG(0xD06B) = spr_saved[3];
    REG(0xD057) = spr_saved[4]; REG(0xD01B) = spr_saved[5]; REG(0xD055) = spr_saved[6]; REG(0xD056) = spr_saved[7];
    REG(0xD076) = spr_saved[8]; REG(0xD05F) = spr_saved[9];
}

/* the display set up, and put back as it was at the end */
#define PAL_R(i) REG(0xD100 + (i))
#define PAL_G(i) REG(0xD200 + (i))
#define PAL_B(i) REG(0xD300 + (i))
static unsigned char saved[16];

static void palettes(void) {
    unsigned char i, bank = (saved[14] >> 4) & 3, spr = (bank + 1) & 3;
    v_pal_screen = (unsigned char)(bank << 6 | bank << 4 | spr << 2 | bank);
    REG(0xD070) = (unsigned char)(spr << 6 | bank << 4 | spr << 2 | bank);   /* edit the sprites' */
    for (i = 0; i < 128; i++) {                   /* every sprite: the same colours for now */
        PAL_R(i) = spr_pal_r[i & 15];
        PAL_G(i) = spr_pal_g[i & 15];
        PAL_B(i) = spr_pal_b[i & 15];
    }
    REG(0xD070) = v_pal_screen;
    video_tint(0);
}

void video_on(void) {
    static unsigned char again;
    /* the VIC-IV's registers unlocked (the DOS locks them when it loads a
     * file, as the art just was) */
    REG(0xD02F) = 0x47;
    REG(0xD02F) = 0x53;
    if (again) {                              /* set up already: the registers only */
        REG(0xD05D) = REG(0xD05D) & 0x7F;
        REG(0xD031) = REG(0xD031) | 0xC8;
        REG(0xD054) = REG(0xD054) | 0x55;
        return;
    }
    again = 1;
    saved[0] = REG(0xD031); saved[1] = REG(0xD054); saved[2] = REG(0xD058); saved[3] = REG(0xD059);
    saved[4] = REG(0xD05D); saved[5] = REG(0xD05E); saved[6] = REG(0xD060); saved[7] = REG(0xD061);
    saved[8] = REG(0xD062); saved[9] = REG(0xD063); saved[10] = REG(0xD064); saved[11] = REG(0xD065);
    saved[12] = REG(0xD020); saved[13] = REG(0xD021); saved[14] = REG(0xD070); saved[15] = REG(0xD07B);
    v_textx = REG(0xD04C) | (unsigned int)(REG(0xD04D) & 0x0F) << 8;

    video_clear();
    dma_fill_far(COLOR_LIN, COLS * ROWS * 2, 0);

    REG(0xD020) = 0;
    REG(0xD021) = 0;
    REG(0xD05D) = REG(0xD05D) & 0x7F;         /* hot registers off */
    REG(0xD031) = REG(0xD031) | 0xC8;         /* H640 + V400, FAST */
    REG(0xD054) = REG(0xD054) | 0x55;         /* CHR16 + FCLRHI: characters > $FF are FCM; SPR640; VFAST: 40 MHz */
    REG(0xD058) = (unsigned char)(COLS * 2);  /* bytes per row */
    REG(0xD059) = 0;
    REG(0xD05E) = COLS;
    REG(0xD07B) = ROWS;
    REG(0xD060) = (unsigned char)SCREEN_LIN;
    REG(0xD061) = (unsigned char)(SCREEN_LIN >> 8);
    REG(0xD062) = (unsigned char)(SCREEN_LIN >> 16);
    REG(0xD063) = REG(0xD063) & 0xF0;
    REG(0xD064) = 0;                          /* colour RAM from its start */
    spr_y0 = (REG(0xD04E) | (unsigned int)(REG(0xD04F) & 0x0F) << 8) + PLAY_ROW * 8;   /* TEXTYPOS: PAL or NTSC */
    REG(0xD065) = 0;
    palettes();
}

void video_off(void) {
    REG(0xD070) = v_pal_screen;
    video_sway(0);
    REG(0xD070) = saved[14];
    REG(0xD054) = saved[1];
    REG(0xD058) = saved[2]; REG(0xD059) = saved[3];
    REG(0xD05E) = saved[5];
    REG(0xD060) = saved[6]; REG(0xD061) = saved[7]; REG(0xD062) = saved[8]; REG(0xD063) = saved[9];
    REG(0xD064) = saved[10]; REG(0xD065) = saved[11];
    REG(0xD07B) = saved[15];
    REG(0xD05D) = saved[4];
    REG(0xD031) = saved[0];
    REG(0xD020) = saved[12]; REG(0xD021) = saved[13];
    k_text = "\x93";                         /* clear the (80-column) screen */
    k_print();
}


unsigned char boot(void) {
    ticks_hz = (REG(0xD06F) & 0x80) ? 60 : 50;   /* NTSC or PAL */
    t_pause = t_of(PAUSE_FRAMES);
    t_intro = t_of(INTRO_FRAMES);
    nausea_timing();
    player_timing();
    k_text = "\x93LOADING...\r";
    k_print();
    /* the first file has the tiles and the logo: a loading screen while
     * the rest comes in */
    gfx_first = 0;
    gfx_last = 1;
    if (!gfx_load()) goto missing;
    video_on();
    video_logo((COLS - LOGO_W) / 2, 12);
    centre(28, s"loading...", C_GREY);
#ifdef LOAD_TEST
    for (;;) {}                       /* (tests: stop on the loading screen) */
#endif
    gfx_first = 1;
    gfx_last = 3;
    if (!gfx_load()) {
        video_off();
        goto missing;
    }
    hs_load();
    video_on();                       /* again: the DOS may have locked the VIC-IV */
    spr_on();
    sfx_on();
    input_flush();
    mux_on();                         /* (the KERNAL's interrupt off: only the raster's) */
#ifdef HS_TEST
    {
        static const unsigned char t[3] = {20, 19, 20};   /* t s t (tests: a save) */
        hs_insert(0, 12345, t, 2, 5);
        hs_save();
    }
    title(1);
#elif defined(DEMO)
    new_game();
#elif defined(STORY_TEST)
    start_level = STORY_TEST;         /* (tests: as if chosen on the title, FIRE) */
    new_game();
#else
    title(0);
#endif
    return 1;
missing:
    k_text = "PUKEGFX1-3 MISSING: RUN PUKEFEST FROM ITS DISK\r";
    k_print();
    return 0;
}

void shutdown(void) {
    mux_off();
    sfx_off();
    spr_off();
    video_off();
}
