# Puke Fest — design plan

An arcade game for the MEGA65, written in C with cc45 (ca45 for hot code),
linked with ln45. Top-down, one or two players (taking turns). Priorities in order: playability,
graphics, sound and music.

## The game

You are at the county fair, feeling a bit queasy. Each of the seven
screens is an area of the fair (the Parking Lot, the Midway, the Food
Court, the Teacups, the Funhouse, the Boat Ride, the Fireworks), each
harder than the last. Collect every ticket on the screen; the exit gate
then opens. Walk out through it to reach the next screen. After the
seventh: a bonus, and round 2, harder again. The five best scores are kept
on the disk (PFHI) with their initials.

### Nausea rules

| Rule | Value (all in `src/tuning.h`) |
|---|---|
| Scale | -10 .. +9, start of a screen +5 |
| -1 and -2 | he gets better by himself: +1 every 10 s, up to 0 |
| -2 and below | he pukes (automatically), at most once a minute, after a 1 s warning (flashing, gurgle) |
| His puke | +1 at once; 2 s later its stench costs -1 if he is still within 3 squares of it |
| Others' pukes | the same stench: -1 if he is within 3 squares 2 s later |
| -3 and below | no recovery by himself: only puking, items and benches bring him back |
| Distance | Chebyshev on the square grid: `max(abs(dx), abs(dy)) <= 3` |
| Puddles | every puke leaves a puddle with animated stench lines; stepping into one costs -1 and makes you slip |
| Pass out | at -10: lose a life (3 lives), restart the screen |
| Recovery | mint +1, ginger ale +3, sitting still on a bench +1 every 1.5 s |

Puking is a relief valve with a catch: the player gains +1 only if he
walks away from it in time (4 squares in 2 s is easy, unless he is boxed in
or slips in a puddle). Below -2 he is stuck until the next puke, so the
minute between pukes is the danger.

Every nausea change goes through `nausea_event(x, y, amount, radius)` or
`nausea_stink(x, y)` (a puke's delayed stench). New hazards (smell clouds, hugging aunts, teacups, seagulls...)
only emit events; the rules do not change.

People and hazards: the crowd wanders with their own hidden nausea and
turns green before puking (chain reactions: a stench makes them sicker);
the cotton-candy kid is faster and sicker; the Hugging Aunt chases you
when you come within 5 squares (a hug: -2 and 1 s dizzy); corn dog stands
show a stink cloud for 1 s, then puff (-1 within 2 squares). Every screen
starts with at least 2 puddles. Nobody walks through anybody: walking into
someone bumps you back. A paper bag catches your next puke (or one within 2
squares of you); full, FIRE throws it at the nearest person within 8
squares: -4 to them, the aunt stunned.

The HUD counts down to the next puke ("puke in 12") at -2 and below; a
tone warns when a hazard is about to go; at -7 and below the heart pounds.
Walking into someone makes them step aside. Screens fade in and out.

Between screens, a story chapter (with its own music) warns about the next
one. The title shows the rules and the high scores (with the round and
screen reached and the date) in turn, then a demo; left / right chooses the
starting screen (and plays its music), up / down 1 or 2 players, O the
options (music on / off; practice mode: the crowd and the hazards 1.5 times
as slow, the aunt slower, no high scores). The title says v1.0. Two
players take turns: a turn lasts until a pass-out, each has their own
score, lives, screen and round, and each enters initials at the end. Each screen has a mood: the teacups' floor spins, the
mirrors flash, the water shimmers, the fireworks are at night (with bursts).

Score: tickets, pickups, time bonus, bonus for a screen cleared without
puking, bonus for NPC pukes you dodged.

## Technical design

- **Display**: VIC-IV 80 x 50 (640 x 400), 16-bit character numbers.
  Characters above $FF are full-colour (FCM, 64 bytes, a byte per pixel);
  below $FF the normal character set, for the HUD and messages. Row 0 is
  the HUD, rows 1-48 the play area: 20 x 12 squares of 32 x 32 pixels (4 x 4
  characters each). The screen (8000 bytes) is in bank 5.
- **Art and data on disk**, loaded at the start by the KERNAL
  (`src/load.s45`; no C stdio): `PUKEGFX1`/`PUKEGFX2` (banks 4-5: tiles, the
  stench frames, the player's, the kid's and the aunt's pictures), `PUKEGFX3`
  (bank 1 at $12000, free while the game runs: the crowd's pictures, the
  samples, the maps, the logo's layout, the text pages). Bank 0 keeps the
  code and small tables.
- **Palettes**: the screen keeps the ROM's palette bank (entries 16+ for the
  tiles, 0-15 the system colours for text); the sprites get the next bank.
  Below 0 the tiles turn toward sick green by -nausea / 16 (sprites stay
  vivid); at -5 and below the whole screen sways (TEXTXPOS and every
  sprite's X, the same units).
- **Maps**: a byte per square (240 bytes a screen). Each square type has
  attributes (solid, item, bench, gate, exit, puddle) and a puddle variant.
  Maps carry their width and height. A map larger than the screen will
  scroll later; for now maps fit the screen.
- **Sprites**: 16-colour at the native resolution, 42 rows high; a
  character is 32 x 42, two sprites side by side: 4 pairs. The multiplexer
  (`mux.c` chooses, `mux.s45` is the raster interrupt) reuses a pair
  further down the screen once its character has ended, so any number fit
  as long as no raster line has more than 4; beyond that they take turns
  (flicker), the player always shown. All sprites share one palette (a
  character moves between pairs), so the crowd's variety is recoloured art,
  and nobody else wears red.
- **Sound**: SID 1 for music, SID 2 for effects (mirrored on SIDs 3 and 4:
  both speakers), the audio DMA channels for samples (BLARGH, splat,
  synthesized by `tools/sounds.py`, loaded with the art). The music player
  (`tune.c`) runs once a frame from the main loop; the tunes are text
  (`assets/music.txt`), compiled by `tools/gen_music.py`: the title's
  waltz, the story's music box, a tune for each screen (a stroll, the
  circus march, a polka, a dizzy waltz, a spooky funhouse tune, a
  barcarolle, a brass fanfare), jingles. Below 0 the screen's tune
  wobbles; from -5 it is faster and out of tune.
- **Game logic knows nothing about drawing**, so a first-person view could
  be added later as another view module over the same grid.

### Source layout

```
Makefile              cc45 -O2, ln45 -basic, disk45 -> D81 (as in ../tank)
src/main.c            state machine: title, play, passed out, clear, game over
src/video.c           FCM screen, palettes, tiles, HUD and messages
src/mux.c, mux.s45    the sprite multiplexer: scheduling, raster interrupt
src/npc.c             the crowd, the cotton-candy kid, the Hugging Aunt
src/hazard.c          corn dog stands
src/rng.c             random numbers
src/ticks.c           times: NTSC, harder screens
src/hiscore.c         the high scores (PFHI)
src/load.s45          loading the art, files, printing (KERNAL); samples (audio DMA)
src/ai.c              the machine plays: the demo and attract mode (a search to the nearest ticket)
src/game.h            the game's state, shared with the overlays
src/bag.c             the paper bag in the air
src/front.c           the title, level select, high score entry (overlay "front")
src/boot.c            start-up with the loading screen, shut-down (overlay "boot")
src/level.c           starting a screen: the map, puddles, HUD (overlay "level")
src/people.c          a screen's crowd and hazards (overlay "people")
src/story.c           the story between screens (overlay "story")
src/ai.c              the machine plays (overlay "ai")
src/turns.c           a new game, two players' turns, drawing the HUD (overlay "turns")
src/options.c         the options screen (overlay "options")
src/map.c             the current map: squares, attributes, collision
src/player.c          the player: movement, collision, animation
src/nausea.c          the nausea rules
src/sfx.c             sound effects (SID 2) and samples
src/tune.c            the music player
assets/music.txt      the tunes and instruments
tools/gen_music.py    compiles them: build/music.c/.h
tools/sounds.py       the samples, synthesized
src/ctl.c             joystick and keys (scripted in the DEMO build)
src/tuning.h          every gameplay constant
tools/art.py          the art (shapes and ASCII; placeholder quality) and the logo
assets/tiles/*.png    optional hand-drawn tiles: NAME.png, 32 x 32 (animated: frames side by side)
assets/sprites/*.png  optional hand-drawn sprite sheets: player, crowd0-2, kid, aunt;
                      32 x 42 pictures side by side in art.py's frame order
tools/gen_assets.py   packs it: build/assets.c/.h (tables), build/pukegfx1-3.bin (art, data)
tests/nausea_test.c   the nausea rules, stepped on the host (make test)
tests/mux_test.c      the multiplexer's scheduler, on the host (make test)
tools/screenshot.py   headless xemu run -> PNG
assets/level*.txt     the seven maps (a name line, people start on the floor around them)
assets/text.txt       the title's text pages
```

## Milestones

| # | Milestone | Done when |
|---|---|---|
| M0 | Skeleton | `make run` boots in xemu from the D81 |
| M1 | Grey-box | one screen at 80 x 50, player moves and collides, nausea rules, puddles, stench, tickets/exit, recovery, lives (done) |
| M2 | Game loop | multiplexer, the crowd with chain reactions, Hugging Aunt, cotton-candy kid, corn dog stands, 2 puddles at least on every screen, the Food Court (all done); tune until fun |
| M3 | Graphics | PNG overrides (done), green palette tint (done), screen sway (done), title screen with logo and cast (done); hand-drawn art to come |
| M4 | Sound | SFX on SID 2, samples, music player, 5 tunes, sick music (done) |
| M5 | Content | 7 screens, difficulty curve, rounds, high scores on disk (done) |
| M6 | Polish | attract mode with an AI player, pause, NTSC, hardening for real hardware (done; not yet run on one) |

## Risks

- cc45 is still in development: keep interrupt and per-frame hot code in
  ca45 when C is not up to it.
- Bank 0: the program ends at $AF00 (about 768 bytes free); nine copied
  code overlays (scores, front, boot, level, people, story, ai, turns,
  options: 16.8 KB, kept at $59600 in bank 5, file PUKEFEST.OV1) take turns
  in a 2.9 KB region at $B200, below a 512-byte stack. The music is in bank 1; the tune playing
  is copied into a buffer (TUNE_MAX). More code means more (or smaller)
  overlays: the region is as large as the largest. An overlay is one
  object file. `__far` (mapped) overlays need the program below $4000.
  `__far` (mapped) overlays need the program below $4000, so when it fills
  up, cold code (title, messages, set-up) goes to copied overlays sharing
  one region.
- Bank 1's $10000-$11FFF is the DOS's: loading anything there makes saving
  a file (the high scores) fail. Free far memory left: about 1.5 KB in
  bank 1's data, about 2 KB in the overlay store.
- xemu's `-screenshot` redraws the last frame without running the CPU, so
  multiplexed sprites show only each pair's last character in it; the
  `MUX_TRACE` build checks the interrupt's timing from a memory dump.
- Sickness effects must not make the game unfair: each has its own tuning.

## Real hardware: what to check first

Only run in xemu so far. Done for the hardware: the VIC-IV registers are
unlocked explicitly after the art loads (the DOS locks them), 40 MHz is
set (FAST + VFAST; the audio DMA needs it), the sprite Y origin comes from
TEXTYPOS (PAL 104, NTSC 55), the music keeps its tempo at 60 Hz. To check:

1. Loading: `PUKEFEST`, `PUKEFEST.OV1`, `PUKEGFX1-3` from the SD card's D81 (about
   125 KB: several seconds).
2. The title: the logo's drips run, the cast walks, the music plays.
3. Play: sprites line up with the squares (both PAL and NTSC), no flicker
   with few people, turns taken when more than 4 share a band.
4. Samples (BLARGH, splat) on both sides; SID music and effects.
5. High scores: a new one saved, still there after a reset.
6. RUN/STOP back to BASIC with the screen as it was.
7. RESTORE: not handled (the KERNAL's NMI); see what it does.

## Balance notes (the machine playing each screen 2.5 minutes, from +5)

It clears a screen in 15-22 s (the Funhouse ~36 s, once people stepped
aside when bumped: before, they blocked its one-square corridors for a
minute), and never passed out: from +5 a screen is over before nausea
builds. A person is slower, so this says little about how hard nausea
feels; the aunt hugged it every 15-20 s (it does not dodge her). Tune by
playing: NPC_DRIFT, HAZARD_EVERY, AUNT_SPEED, NAUSEA_START in tuning.h.
