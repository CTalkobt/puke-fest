# Puke Fest — keep your lunch down at the county fair (MEGA65)
#
#   make              build build/pukefest.d81 with disk45: the program (PUKEFEST),
#                     its code overlays (PUKEFEST.OV1, loaded to $59600) and its art and
#                     data (PUKEGFX1-3)
#   make run          build the disk image and boot it in xemu-xmega65 (drive 8,
#                     PUKEFEST loaded and run from it, as on a MEGA65)
#   make demo         the self-playing build (build/demo/), run in xemu
#   make screenshot   run the self-playing build headless, save build/screenshot.png
#   make test         the nausea rules and the multiplexer's scheduler, on this machine
#   make wav          the samples as WAV files in build/wav, to listen to
#
# The art is loaded from the disk, so the program runs from build/pukefest.d81.
#   make clean        remove build/
#
# Variables: M65 (the M65Compiler tree, default ../m65compiler), XEMU,
# SHOT_DELAY (seconds).

M65     ?= ../m65compiler
CC45    := $(M65)/bin/cc45
CA45    := $(M65)/bin/ca45
LN45    := $(M65)/bin/ln45
DISK45  := $(M65)/bin/disk45
STDLIB  := $(M65)/lib/build/c45.lib
XEMU    ?= xemu-xmega65
PYTHON  ?= python3
HOSTCC  ?= cc

BUILD   ?= build
DEFS    ?=

CFLAGS  := -O2 -I$(M65)/lib/include -Isrc -I$(BUILD) $(DEFS)

TARGET  := $(BUILD)/pukefest.prg
# the code overlays, written by ln45 with the program (the program loads
# them as PUKEFEST.OV1: ln45 names them after it)
OVL     := $(BUILD)/pukefest.ov1
C_MODS  := main video mux map player npc hazard nausea rng ticks ctl sfx tune hiscore ai bag front boot level people story turns options
OBJS    := $(C_MODS:%=$(BUILD)/%.o45) $(BUILD)/mux_irq.o45 $(BUILD)/load.o45 $(BUILD)/assets.o45 $(BUILD)/music.o45
HEADERS := $(wildcard src/*.h) $(BUILD)/assets.h $(BUILD)/music.h
MAPS    := $(sort $(wildcard assets/level*.txt))

DISK    := $(BUILD)/pukefest.d81

.PHONY: all run disk demo screenshot test wav clean

all: $(DISK)

$(BUILD):
	mkdir -p $@

$(BUILD)/assets.h $(BUILD)/assets.c $(BUILD)/pukegfx1.bin $(BUILD)/pukegfx2.bin $(BUILD)/pukegfx3.bin &: $(BUILD)/music.bin tools/gen_assets.py tools/art.py tools/sounds.py assets/text.txt $(MAPS) $(wildcard assets/tiles/*.png assets/sprites/*.png) | $(BUILD)
	$(PYTHON) tools/gen_assets.py $(BUILD) $(MAPS)

$(BUILD)/%.o45: src/%.c $(HEADERS) | $(BUILD)
	$(CC45) $(CFLAGS) -c $< -o $@

$(BUILD)/mux_irq.o45: src/mux.s45 | $(BUILD)
	$(CA45) -c $< -o $@

$(BUILD)/load.o45: src/load.s45 | $(BUILD)
	$(CA45) -c $< -o $@

$(BUILD)/music.h $(BUILD)/music.c $(BUILD)/music.bin &: tools/gen_music.py assets/music.txt | $(BUILD)
	$(PYTHON) tools/gen_music.py $(BUILD) assets/music.txt

$(BUILD)/music.o45: $(BUILD)/music.c $(BUILD)/music.h
	$(CC45) $(CFLAGS) -c $< -o $@

$(BUILD)/assets.o45: $(BUILD)/assets.c $(BUILD)/assets.h
	$(CC45) $(CFLAGS) -c $< -o $@

$(TARGET) $(OVL) &: $(OBJS) $(STDLIB)
	$(LN45) -basic --overlay-store=0x59600-0x5DFFF -M $(BUILD)/pukefest.map -o $(TARGET) $(OBJS) $(STDLIB)

$(STDLIB):
	$(MAKE) -C $(M65) lib

run: $(DISK)
	$(XEMU) -8 $(abspath $(DISK)) -autoload

disk: $(DISK)

# the disk: the program first (-autoload runs the first program), then the
# overlays and the art, under the names the program loads them by
$(DISK): $(TARGET) $(OVL) $(BUILD)/pukegfx1.bin $(BUILD)/pukegfx2.bin $(BUILD)/pukegfx3.bin
	rm -f $@
	$(DISK45) create $@ -n "PUKE FEST" -i "PF"
	$(DISK45) add $@ $(TARGET) PUKEFEST
	$(DISK45) add $@ $(OVL) PUKEFEST.OV1
	$(DISK45) add $@ $(BUILD)/pukegfx1.bin PUKEGFX1
	$(DISK45) add $@ $(BUILD)/pukegfx2.bin PUKEGFX2
	$(DISK45) add $@ $(BUILD)/pukegfx3.bin PUKEGFX3

demo:
	$(MAKE) BUILD=build/demo DEFS=-DDEMO run

screenshot:
	$(MAKE) BUILD=build/demo DEFS=-DDEMO all
	$(PYTHON) tools/screenshot.py build/demo/pukefest.prg build/screenshot.png "$(SHOT_DELAY)" build/demo/pukefest.d81

test: $(BUILD)/assets.h
	$(HOSTCC) -Wall -Wno-unused-function -Itests -Isrc -o $(BUILD)/nausea_test tests/nausea_test.c
	$(BUILD)/nausea_test
	$(HOSTCC) -Wall -Wno-unused-function -Wno-unused-value -Wno-pointer-to-int-cast -Itests -Isrc -I$(BUILD) -o $(BUILD)/mux_test tests/mux_test.c
	$(BUILD)/mux_test

wav: | $(BUILD)
	mkdir -p $(BUILD)/wav
	$(PYTHON) -c "import sys, wave; sys.path.insert(0, 'tools'); import sounds; [(lambda w, d: (w.setnchannels(1), w.setsampwidth(1), w.setframerate(r), w.writeframes(bytes(d)), w.close()))(wave.open('$(BUILD)/wav/%s.wav' % n.lower(), 'wb'), m()) for n, m, r in sounds.SOUNDS + sounds.NEAR_SOUNDS]"

clean:
	rm -rf build

