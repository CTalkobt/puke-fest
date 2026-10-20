# pfest — keep your lunch down at the county fair (MEGA65)
#
#   make              build build/pfest.d81: the program (PFEST), its code overlays
#                     (PFEST.OV1, loaded to $59600) and its art and data (PFGFX1-3)
#   make run          build and run it in xemu-xmega65
#   make demo         the self-playing build (build/demo/), run in xemu
#   make screenshot   run the self-playing build headless, save build/screenshot.png
#   make test         the nausea rules and the multiplexer's scheduler, on this machine
#   make wav          the samples as WAV files in build/wav, to listen to
#
# The art is loaded from the disk, so the program runs from build/pfest.d81.
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

TARGET  := $(BUILD)/pfest.prg
C_MODS  := main video mux map player npc hazard nausea rng ticks ctl sfx tune hiscore ai bag front boot level people story turns options
OBJS    := $(C_MODS:%=$(BUILD)/%.o45) $(BUILD)/mux_irq.o45 $(BUILD)/load.o45 $(BUILD)/assets.o45 $(BUILD)/music.o45
HEADERS := $(wildcard src/*.h) $(BUILD)/assets.h $(BUILD)/music.h
MAPS    := $(sort $(wildcard assets/level*.txt))

DISK    := $(BUILD)/pfest.d81

.PHONY: all run disk demo screenshot test wav clean

all: $(DISK)

$(BUILD):
	mkdir -p $@

$(BUILD)/assets.h $(BUILD)/assets.c $(BUILD)/pfgfx1.bin $(BUILD)/pfgfx2.bin $(BUILD)/pfgfx3.bin &: $(BUILD)/music.bin tools/gen_assets.py tools/art.py tools/sounds.py assets/text.txt $(MAPS) $(wildcard assets/tiles/*.png assets/sprites/*.png) | $(BUILD)
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

$(TARGET): $(OBJS) $(STDLIB)
	$(LN45) -basic --overlay-store=0x59600-0x5DFFF -M $(BUILD)/pfest.map -o $@ $(OBJS) $(STDLIB)

$(STDLIB):
	$(MAKE) -C $(M65) lib

run: $(DISK)
	$(XEMU) -prgmode 65 -8 $(abspath $(DISK)) -prg $(abspath $(TARGET))

disk: $(DISK)

$(DISK): $(TARGET) $(BUILD)/pfgfx1.bin $(BUILD)/pfgfx2.bin $(BUILD)/pfgfx3.bin
	rm -f $@
	$(DISK45) create $@ -n "PFEST" -i "PF"
	$(DISK45) add $@ $(TARGET) PFEST
	$(DISK45) add $@ $(BUILD)/pfest.ov1 PFEST.OV1
	$(DISK45) add $@ $(BUILD)/pfgfx1.bin PFGFX1
	$(DISK45) add $@ $(BUILD)/pfgfx2.bin PFGFX2
	$(DISK45) add $@ $(BUILD)/pfgfx3.bin PFGFX3

demo:
	$(MAKE) BUILD=build/demo DEFS=-DDEMO run

screenshot:
	$(MAKE) BUILD=build/demo DEFS=-DDEMO all
	$(PYTHON) tools/screenshot.py build/demo/pfest.prg build/screenshot.png "$(SHOT_DELAY)" build/demo/pfest.d81

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

