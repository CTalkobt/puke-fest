/* tuning.h — every gameplay constant, in one place
 *
 * Times are in video frames at 50 Hz (PAL); main.c scales the ones marked
 * (s) for 60 Hz.
 */
#pragma once

/* nausea */
#define NAUSEA_START      5          /* at the start of a screen */
#define NAUSEA_MAX        9
#define NAUSEA_PASSOUT  (-10)        /* down to this: lose a life */
#define RECOVER_FROM    (-2)         /* from here up to -1 he gets better by himself... */
#define RECOVER_FRAMES  500          /* (s) ...+1 every 10 s, up to 0 */
#define PUKE_AT         (-2)         /* at this or below he pukes... */
#define PUKE_COOLDOWN  3000          /* (s) ...at most once a minute */
#define PUKE_WARN        50          /* (s) the warning before it */
#define PUKE_FREEZE      30          /* frames he stands still, puking */
#define PUKE_GAIN         1          /* puking makes him feel better... */
#define STINK_DELAY     100          /* (s) ...but this much later, every puke... */
#define PUKE_RADIUS       3          /* squares: ...this near him... */
#define PUKE_COST       (-1)         /* ...costs this (his own included) */
#define PUDDLE_COST     (-1)         /* stepping into a puddle */
#define SLIP_FRAMES      14          /* sliding on after stepping into one */

/* recovery */
#define MINT_GAIN         1
#define ALE_GAIN          3
#define BENCH_FRAMES     75          /* (s) sitting still this long: +1 */
#define BENCH_GAIN        1

/* the player: 1/16 pixels per frame */
#define SPEED            40          /* 2.5 pixels a frame */
#define SPEED_DIAG       28          /* each axis, moving diagonally */
#define NUDGE            12          /* pixels: slide around a corner this near */
#define ANIM_FRAMES       8          /* frames per walking image */

/* people's bodies: two are too close when their feet are less than BODY_X
 * across and BODY_Y up or down apart; walking into someone bumps the
 * player back (BUMP_FRAMES at SPEED), at most once every BUMP_GAP frames */
#define BODY_X           18
#define BODY_Y           10
#define BUMP_FRAMES       4
#define BUMP_GAP         20

/* the player's hitbox, around his feet (pixels) */
#define BOX_L            10
#define BOX_R             9
#define BOX_T            10
#define BOX_B             2

/* the crowd */
#define NPC_MAX          11          /* on a screen (ids 1-11 for the multiplexer) */
#define NPC_SPEED        24          /* 1/16 pixels a frame: 1.5 pixels */
#define NPC_TURN_MIN     40          /* frames before choosing a new way... */
#define NPC_TURN_RND    127          /* ...plus up to this */
#define NPC_NAUSEA_MIN    1          /* nausea at the start: 1... */
#define NPC_NAUSEA_RND    3          /* ...to 3 */
#define NPC_DRIFT      1500          /* (s) the fair gets to them: -1 every 15-45 s */
#define NPC_PUKE_AT     (-1)         /* they puke at -1 down to... */
#define NPC_TOLERANCE     3          /* ...-3 (each their own, new after each puke) */
#define NPC_WARN_MIN     50          /* (s) green and standing still 1 s... */
#define NPC_WARN_RND    100          /* (s) ...to 3 s, then they puke */
#define NPC_RELIEF_MIN    1          /* their nausea after puking: 1... */
#define NPC_RELIEF_RND    3          /* ...to 3 */
#define NPC_SMELL_IN      2          /* a stench in reach costs them 1, */
#define NPC_SMELL_OF      3          /*   2 times in 3 */
#define NPC_FREEZE       40          /* frames they stand still, puking */

/* the cotton-candy kid: a crowd member, faster and sicker */
#define KID_SPEED        32          /* 2 pixels a frame */
#define KID_TURN_MIN     20
#define KID_TURN_RND     60
#define KID_DRIFT       600          /* (s) -1 every 6-18 s */

/* the Hugging Aunt: never sick, chases you when you come near */
#define AUNT_SPEED       30          /* 1.9 pixels a frame (you: 2.5) */
#define AUNT_SEE          5          /* squares: she sees you this near... */
#define AUNT_CHASE      300          /* (s) ...and chases you up to 6 s */
#define AUNT_REST       250          /* (s) then (or after a hug) leaves you alone 5 s */
#define AUNT_REACH_X     24          /* pixels between their feet for a hug (bodies keep */
#define AUNT_REACH_Y     14          /*   BODY_X x BODY_Y apart) */
#define HUG_COST        (-2)
#define HUG_STUN         50          /* frames you stand there, dizzy */

/* hazards: corn dog stands (a stink cloud, then a puff), teacups (they
 * spin), fireworks (lit, then bang): every 8-16 s a warning for 1 s, then
 * the effect on everyone near */
#define HAZARD_EVERY    400          /* (s) */
#define HAZARD_RND      400          /* (s) */
#define HAZARD_WARN      50          /* (s) the warning before the effect */
#define HAZARD_SHOW      40          /* (s) the warning tile stays a little after */
#define CORNDOG_RADIUS    2          /* squares */
#define CORNDOG_COST    (-1)
#define TEACUP_RADIUS     1
#define TEACUP_COST     (-1)
#define FIREWORK_RADIUS   2
#define FIREWORK_COST   (-1)

/* each screen (and each round after the seventh) is harder: the crowd gets
 * sick and the hazards go off 8 / (8 + d) as often (d: the screen number,
 * + 7 a round), the aunt is d pixels/16 a frame faster (up to AUNT_FASTEST) */
#define AUNT_FASTEST     38

/* practice mode (the options): the crowd gets sick and the hazards go off
 * 1.5 times as slowly, the aunt is this much slower (1/16 pixels a frame) */
#define PRACTICE_AUNT_SLOWER 6

/* every screen starts with at least this many puddles (the map's, and
 * more at random, not within PUKE_RADIUS of the start) */
#define MIN_PUDDLES       2

/* how sick it sounds: a heartbeat from HEART_FROM down, every
 * HEART_FRAMES (fewer the sicker) */
#define HEART_FROM      (-7)
#define HEART_FRAMES     45

/* how sick it looks */
#define SICK_TINT_MAX     9          /* the screen turns green by -nausea / 16, up to this / 16 */
#define SWAY_FROM       (-5)         /* from here down the screen sways... */
#define SWAY_GAIN         2          /* ...this many pixels more each step down */

/* the screens' moods: frames per palette step; how dark the night (/ 16) */
#define MOOD_RATE        10
#define NIGHT             7

/* the title logo's slime: frames per palette step */
#define LOGO_RATE         6

/* the paper bag: it catches the player's next puke, or one within
 * BAG_REACH squares of him; full, FIRE throws it at the nearest person
 * within BAG_RANGE squares: BAG_COST to them (the aunt: stunned) */
#define BAG_REACH         2
#define BAG_RANGE         8
#define BAG_SPEED         5          /* pixels a frame in the air */
#define BAG_COST        (-4)
#define AUNT_BAG_STUN   150          /* frames she stands there */

/* score */
#define SCORE_TICKET    100
#define SCORE_MINT       10
#define SCORE_ALE        25
#define SCORE_CLEAR     500
#define SCORE_NO_PUKE   1000         /* a screen cleared without puking */
#define SCORE_DODGE      50          /* out of a crowd puke's stench in time */
#define SCORE_WIN      5000          /* all seven screens */
#define SCORE_CATCH      50          /* a puke caught in the bag */
#define SCORE_HIT       100          /* someone hit with a full bag */

#define ANIM_RATE         8          /* frames per image of the puddles' stench */

#define LIVES             3
#define PAUSE_FRAMES    120          /* (s) passed out / screen clear messages */
#define INTRO_FRAMES     90          /* (s) the screen's name before it starts */
#define TITLE_PAGE      400          /* (s) the title's rules and high scores take turns */
#define ATTRACT_FRAMES 1500          /* (s) then a demo, this long at most */
#define STORY_FRAMES   1500          /* (s) a chapter waits this long for FIRE */
