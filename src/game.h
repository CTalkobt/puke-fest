/* game.h — the game's state, shared by main.c and the cold code in the
 * overlays (front.c: the title and the high scores; setup.c: starting up,
 * shutting down, starting a screen)
 */
#pragma once

#define ST_TITLE 0
#define ST_INTRO 1                   /* the screen's name, before it starts */
#define ST_PLAY  2
#define ST_OUT   3                   /* passed out */
#define ST_CLEAR 4                   /* the screen is done */
#define ST_WIN   5                   /* all seven: round over */
#define ST_OVER  6
#define ST_ENTRY 7                   /* initials for the high scores */
#define ST_PAUSE 8
#define ST_STORY 9                   /* between screens */
#define ST_TURN  10                  /* the other player's turn: get ready */
#define ST_OPTIONS 11

extern unsigned char state, level, round, lives, attract, prev_ctl, flash;
extern unsigned int attract_left, pause, t_pause, t_intro;
extern unsigned long score;
extern unsigned char story_win, start_level;

/* two players take turns (a turn lasts until a pass-out): each one's game;
 * score, lives, level and round are the current one's */
typedef struct {
    unsigned long score;
    unsigned char lives, level, round;
} player_t;
extern player_t pl[2];
extern unsigned char players, cur;   /* 1 or 2; whose turn */
extern unsigned char practice;       /* the options: an easier game, no high scores */
extern const unsigned char screen_tune[NLEVELS];   /* each screen's music */

/* main.c */
extern signed char shown_nausea;
extern unsigned char shown_tickets, shown_lives, shown_bag, shown_puke;
extern unsigned long shown_score;
void hud(void);
void box(unsigned char row, unsigned char rows);
void centre(unsigned char row, const char *s, unsigned char colour);
void message(const char *a, const char *b, unsigned char colour);

/* front.c (the overlay "front") */
extern unsigned char page;
extern unsigned int page_left;
void title(unsigned char scores);
void title_page(void);
void title_cast(void);
void title_select(void);             /* the starting screen's line */
void game_over_done(void);
void entry(unsigned char c, unsigned char pressed);

/* options.c (the overlay "options") */
void options(void);
void options_input(unsigned char pressed);

/* story.c (the overlay "story"): the next chapter, or (win) the win page */
void story(unsigned char win);

/* boot.c (the overlay "boot") */
unsigned char boot(void);            /* 0: the files are not on the disk */
void shutdown(void);

/* level.c (the overlay "level") */
void start_screen(void);
void hud_frame(void);

/* turns.c (the overlay "turns") */
void keep(void);                     /* the current player's game into pl[cur] */
void next_turn(void);                /* the other player's turn: a card (ST_TURN) */
void new_game(void);
void hud_nausea(void);               /* the HUD's parts, drawn again */
void hud_puke(unsigned char seconds);
void hud_tickets(void);
void hud_score(void);
void hud_bag(void);
void hud_lives(void);
