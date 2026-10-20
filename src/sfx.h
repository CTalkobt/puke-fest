/* sfx.h — sound effects: SID 2, and the samples (BLARGH, SPLAT) */
#pragma once

void sfx_on(void);
void sfx_off(void);
void sfx_update(void);               /* once a frame */
void sfx_mute(unsigned char on);     /* (pause) */
void sfx_puke(void);                 /* the player's: BLARGH */
void sfx_crowd_puke(unsigned char kid);   /* someone else's (higher for the kid) */
void sfx_splat(void);                /* into a puddle */
void sfx_eww(void);                  /* the crowd sees a puke */
void sfx_oof(void);                  /* bumped into someone */
void sfx_whoosh(void);               /* a bag thrown */
void sfx_thud(void);                 /* a bag hits */
void sfx_whistle(void);              /* a firework going up */
void sfx_warn(void);
void sfx_pickup(void);
void sfx_good(void);                 /* recovered a little */
void sfx_bad(void);                  /* lost a little */
void sfx_gate(void);
void sfx_passout(void);
void sfx_hug(void);                  /* the aunt got you */
void sfx_puff(void);                 /* a corn dog stand puffed */
void sfx_spin(void);                 /* a teacup spun */
void sfx_alert(void);                /* a hazard is about to go */
void sfx_heart(void);                /* a heartbeat (very sick) */
void sfx_bang(void);                 /* a firework went off */
