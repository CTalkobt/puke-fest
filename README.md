# Puke Fest

Keep your lunch down at the county fair. An arcade game for the MEGA65 in
80 x 50, written in C with the M65Compiler tool suite (cc45, ca45, ln45,
disk45): <https://github.com/CTalkobt/M65Compiler>, <http://cc45.ctalkobt.net>.

Seven screens of the fair - the Parking Lot, the Midway, the Food Court, the
Teacups, the Funhouse, the Boat Ride and the Fireworks - each harder than
the last. Collect every ticket to open the gate. Below 0 nausea you puke
(once a minute at most); walk away before the stench hits. The crowd pukes
too, Aunt Hilda wants a hug, the corn dog stands puff, the teacups spin, the
fireworks bang. Paper bags catch a puke - and a full one can be thrown.

One or two players (taking turns), a story between screens, a tune for each
screen, high scores on the disk.

## Build and run

Needs the M65Compiler tree next to this one (`../m65compiler`, or
`make M65=<path>`) - get it from <https://github.com/CTalkobt/M65Compiler>
(documentation: <http://cc45.ctalkobt.net>) - Python 3 (PIL for the art
previews and PNG art), and xemu (`xemu-xmega65`) to run it.

```sh
make            # build/pukefest.d81: PUKEFEST, PUKEFEST.OV1, PUKEGFX1-3
make run        # run it in xemu from the disk image
make demo       # the machine plays
make test       # the nausea rules and the sprite multiplexer, on the host
make wav        # the samples as WAV files in build/wav
```

On a MEGA65, copy `build/pukefest.d81` to the SD card, mount it and run PUKEFEST.

## Controls

| | |
|---|---|
| Move | joystick (either port), W A S D or the cursor keys |
| Start, throw the bag | FIRE, SPACE or RETURN |
| Title: starting screen | left / right |
| Title: 1 or 2 players | up / down |
| Title: options (music, practice) | O |
| Pause | P |
| Quit to BASIC | RUN/STOP |

## More

`PLAN.md` has the design: the rules, the technical layout (memory, the
sprite multiplexer, the overlays), the source files, the milestones and a
checklist for a first run on real hardware. The gameplay numbers are in
`src/tuning.h`; the maps, the story text and the tunes are plain text in
`assets/`.
