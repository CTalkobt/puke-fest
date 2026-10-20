#!/usr/bin/env python3
"""Generate the game's assets

Usage: gen_assets.py <build dir> <map.txt>...

Writes
- <build>/assets.h, assets.c: the tables the program keeps in bank 0 (tile
  attributes, palettes, the maps)
- <build>/pukegfx1.bin, pukegfx2.bin: the art, loaded from disk at the start
  to GFX_LIN (bank 4) and on into bank 5: the tiles' FCM characters, the
  stench frames, the sprite images (see the GFX_ offsets in assets.h).
  Each starts with its load address in its bank ($0000)
- <build>/preview.png: every tile and player image, 3x (if PIL is there)

Tiles are 32 x 32: 16 FCM characters of 64 bytes (a byte per pixel, the
palette entry), in rows. Sprites are 16-colour, 16 x 42 halves (8 bytes a
row, the high nibble the left pixel), 384 bytes each; a picture is two
halves side by side. The art itself is in art.py.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import art                                               # noqa: E402
import sounds                                            # noqa: E402

PAL_FIRST = 16                 # entries 0-15 stay the system colours (text)
PAL_MAX = 239                  # entries 16-254 for the tiles (255 is FCM's "colour RAM" value)
ART = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'assets')
LETTER_RGB = dict(art.TILE_PAL)
SP = {c: i for i, (c, _) in enumerate(art.SPR_PAL)}
SPR_SLOT = 384                 # bytes per sprite half (16 x 42 x 4 bits = 336)

# tile attributes (src/map.h's use)
A_SOLID, A_BENCH, A_TICKET, A_MINT, A_ALE, A_GATE, A_EXIT, A_PUKE = (1 << i for i in range(8))

T = art.TILES
POOL = art.PUKE_POOL
# name, attributes, picture, puddle variant (another tile's name), what an item leaves
TILES = [
    ('GRASS', 0, T['GRASS'], 'GRASS_PUKE', None),
    ('PATH', 0, T['PATH'], 'PATH_PUKE', None),
    ('FENCE', A_SOLID, T['FENCE'], None, None),
    ('TENT', A_SOLID, T['TENT'], None, None),
    ('TREE', A_SOLID, T['TREE'], None, None),
    ('GATE', A_SOLID | A_GATE, T['GATE'], None, None),
    ('GATE_OPEN', A_EXIT, T['GATE_OPEN'], None, None),
    ('BENCH', A_BENCH, T['BENCH'], None, None),
    ('TICKET', A_TICKET, T['TICKET'], None, 'GRASS'),
    ('MINT', A_MINT, T['MINT'], None, 'GRASS'),
    ('ALE', A_ALE, T['ALE'], None, 'GRASS'),
    ('FLOWERS', 0, T['FLOWERS'], 'GRASS_PUKE', None),
    ('GRASS_PUKE', A_PUKE, T['GRASS_PUKE'], None, None),
    ('PATH_PUKE', A_PUKE, T['PATH_PUKE'], None, None),
    ('CORNDOG', A_SOLID, T['CORNDOG'], None, None),
    ('CORNDOG_PUFF', A_SOLID, T['CORNDOG_PUFF'], None, None),
    ('ASPHALT', 0, T['ASPHALT'], 'ASPHALT_PUKE', None),
    ('PARKLINE', 0, T['PARKLINE'], 'ASPHALT_PUKE', None),
    ('CAR_RED', A_SOLID, T['CAR_RED'], None, None),
    ('CAR_BLUE', A_SOLID, T['CAR_BLUE'], None, None),
    ('LAMP', A_SOLID, T['LAMP'], None, None),
    ('RIDE', 0, T['RIDE'], 'RIDE_PUKE', None),
    ('TEACUP', A_SOLID, T['TEACUP'], None, None),
    ('TEACUP_SPIN', A_SOLID, T['TEACUP_SPIN'], None, None),
    ('CHECKER', 0, T['CHECKER'], 'CHECKER_PUKE', None),
    ('MIRROR', A_SOLID, T['MIRROR'], None, None),
    ('WATER', A_SOLID, T['WATER'], None, None),
    ('BRIDGE', 0, T['BRIDGE'], 'BRIDGE_PUKE', None),
    ('LAUNCHER', A_SOLID, T['LAUNCHER'], None, None),
    ('LAUNCHER_LIT', A_SOLID, T['LAUNCHER_LIT'], None, None),
    ('ASPHALT_PUKE', A_PUKE, T['ASPHALT_PUKE'], None, None),
    ('RIDE_PUKE', A_PUKE, T['RIDE_PUKE'], None, None),
    ('CHECKER_PUKE', A_PUKE, T['CHECKER_PUKE'], None, None),
    ('BRIDGE_PUKE', A_PUKE, T['BRIDGE_PUKE'], None, None),
    ('BAG', 0, T['BAG'], None, 'GRASS'),
]
TILE_ID = {t[0]: i for i, t in enumerate(TILES)}
# the animated tiles: every frame of each
ANIMATED = [(name, [art.stench(POOL[name], f) for f in range(art.PUKE_ANIM)]) for name in
            ('GRASS_PUKE', 'PATH_PUKE', 'ASPHALT_PUKE', 'RIDE_PUKE', 'CHECKER_PUKE', 'BRIDGE_PUKE')]

# map characters -> tiles; '@' is the start (on grass)
TILE_CHARS = {
    '.': 'GRASS', '=': 'PATH', '#': 'FENCE', 'R': 'TENT', 'Y': 'TREE',
    'G': 'GATE', 'b': 'BENCH', 'T': 'TICKET', 'M': 'MINT', 'A': 'ALE',
    '*': 'FLOWERS', '@': 'GRASS', 'c': 'GRASS', 'C': 'PATH', 'a': 'GRASS', 'k': 'GRASS',
    'p': 'GRASS_PUKE', 'P': 'PATH_PUKE', 'D': 'CORNDOG',
    '_': 'ASPHALT', '|': 'PARKLINE', 'X': 'CAR_RED', 'x': 'CAR_BLUE', 'L': 'LAMP',
    'o': 'RIDE', 'U': 'TEACUP', '%': 'CHECKER', 'H': 'MIRROR', '~': 'WATER', '-': 'BRIDGE',
    'F': 'LAUNCHER', 'B': 'BAG',
}
# who starts where on the other floors: on asphalt, the ride, the checkers, the bridge
for c, floor in (('d', 'ASPHALT'), ('e', 'RIDE'), ('f', 'CHECKER'), ('g', 'BRIDGE')):
    TILE_CHARS[c] = floor
# who starts where: the crowd (c on grass, C on the path), the Hugging Aunt,
# the cotton-candy kid (src/npc.h NPC_ types)
NPC_CHARS = {'c': 0, 'C': 0, 'k': 1, 'a': 2, 'd': 0, 'e': 0, 'f': 0, 'g': 0}
GFX_MAX = 0x19600              # pukegfx1/2.bin end below the code overlays ($59600, Makefile) and the screen
DATA_MAX = 0xD800              # pukegfx3.bin: $12000-$1F7FF of bank 1 (hw.h: DATA_LIN)
GFX_LIN, DATA_LIN = 0x40000, 0x12000   # (hw.h)


def nib(v):
    v = max(0, min(255, int(round(v))))
    return ((v << 4) | (v >> 4)) & 0xFF


# --- pictures as RGB: art.py's letters, or hand-drawn PNGs -----------------
# A PNG in assets/tiles/<NAME>.png (32 x 32) replaces that tile (its frames,
# for an animated one: <NAME>.png 32 x 32 * frames wide). A sheet in
# assets/sprites/<who>.png (32 x 42 a picture, left to right in the order
# of art.py's frames: player, crowd0-crowd2, kid, aunt) replaces those
# pictures; transparent pixels (alpha < 128) are see-through, the others go
# to the nearest sprite colour. Tile colours not in the palette yet are
# added to it.

palette = [rgb for _, rgb in art.TILE_PAL]      # the tile palette: RGB, entries from PAL_FIRST
# the logo's cycled entries (art.LOGO_FLOW, LOGO_DROP): placeholders, unique
# and never drawn as such - video.c sets their colours as it animates them
CYCLED = art.LOGO_FLOW + art.LOGO_DROP[0] + art.LOGO_DROP[1]
for k, c in enumerate(CYCLED):
    if c in LETTER_RGB:
        sys.exit("art.py: the logo's cycled entry '%s' is a tile colour's name too" % c)
    LETTER_RGB[c] = (1, 0, k)
    palette.append((1, 0, k))


def png(path):
    try:
        from PIL import Image
    except ImportError:
        sys.exit("%s: reading PNGs needs PIL (python3-pil)" % path)
    return Image.open(path).convert('RGBA')


def tile_png(name, frames=1):
    path = os.path.join(ART, 'tiles', name + '.png')
    if not os.path.exists(path):
        return None
    im = png(path)
    if im.size != (art.N * frames, art.N):
        sys.exit("%s is %dx%d, not %dx%d" % (path, im.size[0], im.size[1], art.N * frames, art.N))
    return [[[im.getpixel((f * art.N + x, y))[:3] for x in range(art.N)] for y in range(art.N)] for f in range(frames)]


def rgb_of(g):
    """a picture of letters (or already RGB) as RGB; '_' is the background"""
    return [[c if isinstance(c, tuple) else (None if c == '_' else LETTER_RGB[c]) for c in row] for row in g]


def entry(rgb):
    if rgb is None:
        return 0                                     # FCM 0: the background colour
    if rgb not in palette:
        if len(palette) == PAL_MAX:
            sys.exit("more than %d tile colours" % PAL_MAX)
        palette.append(rgb)
    return PAL_FIRST + palette.index(rgb)


def char_bytes(g, cx=0, cy=0):
    g = rgb_of(g)
    return [entry(g[cy * 8 + y][cx * 8 + x]) for y in range(8) for x in range(8)]


def tile_bytes(g):
    return [b for cy in range(4) for cx in range(4) for b in char_bytes(g, cx, cy)]


def sprite_sheet(who, frames):
    """frames (letter pictures), with those a PNG sheet has replaced"""
    path = os.path.join(ART, 'sprites', who + '.png')
    if not os.path.exists(path):
        return frames
    im = png(path)
    if im.size[1] != art.PH or im.size[0] % art.PW:
        sys.exit("%s: pictures are %dx%d, side by side" % (path, art.PW, art.PH))
    spal = [(c, rgb) for c, rgb in art.SPR_PAL if c != '.']
    out = list(frames)
    for f in range(min(len(frames), im.size[0] // art.PW)):
        g = []
        for y in range(art.PH):
            row = []
            for x in range(art.PW):
                r, gg, b, a = im.getpixel((f * art.PW + x, y))
                row.append('.' if a < 128 else min(spal, key=lambda e: (e[1][0] - r) ** 2 + (e[1][1] - gg) ** 2 + (e[1][2] - b) ** 2)[0])
            g.append(row)
        out[f] = g
    return out


def spr_half(g, half):
    out = []
    for row in g:
        r = row[half * 16:half * 16 + 16]
        for x in range(0, 16, 2):
            out.append(SP[r[x]] << 4 | SP[r[x + 1]])
    return out + [0] * (SPR_SLOT - len(out))


COLOURS = {'white': 1, 'red': 2, 'cyan': 3, 'green': 5, 'yellow': 7, 'orange': 8, 'lred': 10, 'grey': 12, 'lgreen': 13}


def screen_codes(text):
    """lower-case ASCII as screen codes (letters show as capitals)"""
    out = []
    for ch in text:
        o = ord(ch)
        if 'a' <= ch <= 'z':
            out.append(o - 96)
        elif 0x20 <= o <= 0x3F:
            out.append(o)
        else:
            sys.exit("text: no screen code for %r" % ch)
    return out


def text_record(row, colour, text):
    """a line: row, colour, column, length, the screen codes"""
    t = screen_codes(text)
    return [row, colour, (80 - len(t)) // 2, len(t)] + t


def read_text(path):
    pages, name = {}, None
    for i, line in enumerate(open(path), 1):
        line = line.rstrip('\n')
        if not line.strip() or line.startswith(';'):
            continue
        if line.startswith('['):
            name = line.strip('[] ')
            pages[name] = []
            continue
        row, colour, text = line.split(' ', 2)
        pages[name] += text_record(int(row), COLOURS[colour], text)
    return pages


# where someone starts, the floor is the one most common around it
AUTO_FLOOR = '@ckaC'
FLOORS = {'.': 'GRASS', '*': 'GRASS', '=': 'PATH', '_': 'ASPHALT', '|': 'ASPHALT', 'o': 'RIDE',
          '%': 'CHECKER', '-': 'BRIDGE', 'd': 'ASPHALT', 'e': 'RIDE', 'f': 'CHECKER', 'g': 'BRIDGE'}


def floor_around(rows, x, y):
    seen = {}
    for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1), (-1, -1), (1, -1), (-1, 1), (1, 1)):
        yy, xx = y + dy, x + dx
        if 0 <= yy < len(rows) and 0 <= xx < len(rows[yy]) and rows[yy][xx] in FLOORS:
            f = FLOORS[rows[yy][xx]]
            seen[f] = seen.get(f, 0) + 1
    return TILE_ID[max(seen, key=seen.get) if seen else 'GRASS']


def read_map(path):
    lines = open(path).read().split('\n')
    name = next((l.split(':', 1)[1].strip() for l in lines if l.startswith('; name:')), os.path.basename(path))
    rows = [l.rstrip('\n') for l in lines if l.strip() and not l.startswith(';')]
    w, h = len(rows[0]), len(rows)
    sx = sy = None
    data = []
    npcs = []
    for y, r in enumerate(rows):
        if len(r) != w:
            sys.exit("%s: row %d is %d wide, not %d" % (path, y + 1, len(r), w))
        for x, c in enumerate(r):
            if c not in TILE_CHARS:
                sys.exit("%s: unknown square '%s' at %d,%d" % (path, c, x, y))
            if c == '@':
                sx, sy = x, y
            if c in NPC_CHARS:
                npcs.append((x, y, NPC_CHARS[c]))
            if c in AUTO_FLOOR:
                data.append(floor_around(rows, x, y))
            else:
                data.append(TILE_ID[TILE_CHARS[c]])
    if sx is None:
        sys.exit("%s: no start square '@'" % path)
    # every ticket and the gate within reach of the start (the gate as if open)
    walk = lambda t: not (TILES[t][1] & A_SOLID) or TILES[t][1] & A_GATE
    seen, todo = {(sx, sy)}, [(sx, sy)]
    while todo:
        x, y = todo.pop()
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            xx, yy = x + dx, y + dy
            if 0 <= xx < w and 0 <= yy < h and (xx, yy) not in seen and walk(data[yy * w + xx]):
                seen.add((xx, yy))
                todo.append((xx, yy))
    for i, t in enumerate(data):
        if TILES[t][1] & (A_TICKET | A_GATE) and (i % w, i // w) not in seen:
            sys.exit("%s: %s at %d,%d cannot be reached from the start" % (path, TILES[t][0], i % w, i // w))
    if sum(1 for t in data if TILES[t][1] & A_TICKET) == 0:
        sys.exit("%s: no tickets" % path)
    return w, h, sx, sy, data, npcs, name


def carray(ctype, name, values, per_line=16):
    lines = []
    for i in range(0, len(values), per_line):
        lines.append("    " + ", ".join(str(v) for v in values[i:i + per_line]) + ",")
    return "const %s %s[%d] = {\n%s\n};\n" % (ctype, name, len(values), "\n".join(lines))


def pad(b, n=256):
    return b + [0] * (-len(b) % n)


def preview(path, frames):
    try:
        from PIL import Image
    except ImportError:
        return
    tp = dict(art.TILE_PAL)
    tp['_'] = (0, 0, 0)
    sp = dict(art.SPR_PAL)
    sp['.'] = (40, 40, 48)
    cols = 13
    trows = (len(TILES) + cols - 1) // cols
    top = 34 * trows
    im = Image.new('RGB', (cols * 34, top + 44 * ((len(frames) + cols - 1) // cols)), (0, 0, 0))
    for i, t in enumerate(TILES):
        g = rgb_of(t[2])
        for y in range(32):
            for x in range(32):
                im.putpixel(((i % cols) * 34 + x, (i // cols) * 34 + y), g[y][x] or (0, 0, 0))
    for i, g in enumerate(frames):
        for y in range(42):
            for x in range(32):
                im.putpixel(((i % cols) * 34 + x, top + (i // cols) * 44 + y), sp[g[y][x]])
    im.resize((im.width * 3, im.height * 3), Image.NEAREST).save(path)


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    out, maps = sys.argv[1], sys.argv[2:]
    tiles = []
    for t in TILES:
        p = tile_png(t[0])
        tiles += tile_bytes(p[0] if p else t[2])
    logo_cells, logo_chars = art.logo()
    for c in logo_chars:
        tiles += char_bytes(c)
    tiles = pad(tiles)
    # the logo's layout, with the tiles: in the first file, for the loading screen
    logo_at = len(tiles)
    tiles += pad([255 if c is None else c for row in logo_cells for c in row])
    # the animated tiles: only their top 8 characters change (art.stench)
    anim = []
    for name, fr in ANIMATED:
        p = tile_png(name, len(fr))
        for g in (p or fr):
            anim += tile_bytes(g)[:8 * 64]
    anim = pad(anim)
    # the pictures: the player, the kid and the aunt with the art (banks 4-5);
    # the crowd's in bank 1 (FRAME_FAR on)
    frames = sprite_sheet('player', art.player_frames())
    npc_first = len(frames)
    frames += sprite_sheet('kid', art.kid_frames()) + sprite_sheet('aunt', art.aunt_frames()[:12])
    frame_extra = len(frames)
    frames += sprite_sheet('extra', art.extra_frames())
    frame_far = len(frames)
    far_frames = []
    for v in range(len(art.NPC_VARIANTS)):
        far_frames += sprite_sheet('crowd%d' % v, art.npc_frames(v))
    sprites = [b for g in frames for half in (0, 1) for b in spr_half(g, half)]
    gfx = tiles + anim + sprites
    snd_at = {}                                             # name: (linear address, data, rate)
    for name, make, rate in sounds.NEAR_SOUNDS:
        data_ = make()
        snd_at[name] = (GFX_LIN + len(gfx), data_, rate)
        gfx += data_
    if len(gfx) > GFX_MAX:
        sys.exit("the art is %d bytes: more than the %d below the screen" % (len(gfx), GFX_MAX))
    for i, part in enumerate((gfx[:0x10000], gfx[0x10000:])):
        with open(os.path.join(out, "pukegfx%d.bin" % (i + 1)), "wb") as f:
            f.write(bytes([0, 0] + part))

    # the data (bank 1, DATA_LIN): the crowd's pictures, the samples, the
    # maps, the logo's layout
    data = [b for g in far_frames for half in (0, 1) for b in spr_half(g, half)]
    for name, make, rate in sounds.SOUNDS:
        data_ = make()
        snd_at[name] = (DATA_LIN + len(data), data_, rate)
        data += data_
    levels = [read_map(m) for m in maps]
    level_ofs = []
    for l in levels:
        level_ofs.append(len(data))
        data += l[4]
    # text: the pages, then each screen's name (a line ending in 255)
    pages = read_text(os.path.join(ART, 'text.txt'))
    page_ofs = {}
    for name, recs in pages.items():
        page_ofs[name] = len(data)
        data += recs + [255]
    name_ofs = []
    for l in levels:
        name_ofs.append(len(data))
        data += text_record(0, 1, l[6]) + [255]
    music_ofs = len(data)
    data += list(open(os.path.join(out, 'music.bin'), 'rb').read())   # (tools/gen_music.py)
    if len(data) > DATA_MAX:
        sys.exit("the data is %d bytes: more than the %d in bank 1" % (len(data), DATA_MAX))
    with open(os.path.join(out, "pukegfx3.bin"), "wb") as f:
        f.write(bytes([0x00, 0x20] + data))                     # to $2000 in bank 1

    with open(os.path.join(out, "assets.h"), "w") as f:
        f.write("/* generated by tools/gen_assets.py - do not edit */\n#pragma once\n\n")
        for i, t in enumerate(TILES):
            f.write("#define T_%s %d\n" % (t[0], i))
        f.write("#define NTILES %d\n\n" % len(TILES))
        for n, v in [('A_SOLID', A_SOLID), ('A_BENCH', A_BENCH), ('A_TICKET', A_TICKET), ('A_MINT', A_MINT),
                     ('A_ALE', A_ALE), ('A_GATE', A_GATE), ('A_EXIT', A_EXIT), ('A_PUKE', A_PUKE)]:
            f.write("#define %s 0x%02X\n" % (n, v))
        f.write("#define NO_TILE 0xFF\n\n")
        f.write("/* the art (pukegfx1.bin, pukegfx2.bin), loaded to GFX_LIN: offsets */\n")
        f.write("#define GFX_TILES 0x%04XUL\n#define GFX_ANIM 0x%04XUL\n#define GFX_SPRITES 0x%04XUL\n#define GFX_SIZE 0x%04XUL\n"
                % (0, len(tiles), len(tiles) + len(anim), len(gfx)))
        f.write("#define SPR_SLOT %d\n\n" % SPR_SLOT)
        f.write("/* the data (pukegfx3.bin), loaded to DATA_LIN: the crowd's pictures from 0 (DATA_SPRITES),\n"
                " * the samples (8-bit unsigned, SND_RATE Hz), the maps (level_ofs), the logo (LOGO_OFS) */\n")
        f.write("#define MUSIC_OFS 0x%04XUL   /* the tunes' streams (gen_music.py) */\n" % music_ofs)
        f.write("#define DATA_SPRITES 0\n#define DATA_SIZE 0x%04XUL\n#define LOGO_OFS 0x%04XUL   /* (in the art: GFX_LIN) */\n" % (len(data), logo_at))
        f.write("/* text pages (assets/text.txt): lines of row, colour, column, length, screen codes; 255 ends */\n")
        for name, at in page_ofs.items():
            f.write("#define PAGE_%s 0x%04X\n" % (name.upper(), at))
        f.write("/* the samples: where (linear), how long, the audio DMA rate register value (Hz * 2^24 / 40.5 MHz) */\n")
        rate_reg = lambda hz: hz * (1 << 24) // 40500000
        for name, (at, data_, rate) in snd_at.items():
            f.write("#define SND_%s 0x%05XUL\n#define SND_%s_LEN %d\n#define SND_%s_RATE 0x%06XUL\n"
                    % (name, at, name, len(data_), name, rate_reg(rate)))
        f.write("#define SND_BLARGH_LOW 0x%06XUL\n#define SND_BLARGH_HIGH 0x%06XUL   /* the crowd's, the kid's */\n"
                % (rate_reg(sounds.RATE * 7 // 8), rate_reg(sounds.RATE * 4 // 3)))
        f.write("\n")
        f.write("#define PAL_FIRST %d\n#define NPAL %d\n" % (PAL_FIRST, len(palette)))
        f.write("/* the title logo: LOGO_W x LOGO_H cells, each a character from\n"
                " * TILE_CHAR + LOGO_CHAR on, or 255 (empty) */\n")
        f.write("#define LOGO_CHAR %d\n#define LOGO_W %d\n#define LOGO_H %d\n"
                % (len(TILES) * 16, len(logo_cells[0]), len(logo_cells)))
        f.write("/* its cycled palette entries: 4 for the drips' flow, 4 + 4 for two sets of falling drops */\n")
        f.write("#define LOGO_FLOW %d\n#define LOGO_DROP_A %d\n#define LOGO_DROP_B %d\n"
                % (PAL_FIRST + len(art.TILE_PAL), PAL_FIRST + len(art.TILE_PAL) + 4, PAL_FIRST + len(art.TILE_PAL) + 8))

        f.write("#define NFRAMES %d\n#define FRAME_SICK 12\n#define FRAME_PUKE 24\n" % len(frames))
        f.write("/* everyone else's pictures: FRAME_NPC + look * NPC_FRAMES; from FRAME_FAR on in bank 1 */\n")
        f.write("#define FRAME_NPC %d\n#define NPC_FRAMES %d\n#define NPC_VARIANTS %d\n#define FRAME_FAR %d\n"
                "#define NPC_SICK 12\n#define NPC_PUKE 16\n" % (npc_first, art.NPC_FRAMES, len(art.NPC_VARIANTS), frame_far))
        f.write("/* each one's first picture: the kid, the aunt (with the art), the crowd's variants (FRAME_FAR on) */\n")
        f.write("#define FRAME_KID %d\n#define FRAME_AUNT %d    /* (her 12 walking ones only: she never gets sick) */\n#define FRAME_CROWD %d\n"
                % (npc_first, npc_first + art.NPC_FRAMES, frame_far))
        f.write("#define FRAME_BAG %d      /* the bag in the air (2) */\n#define FRAME_BURST %d    /* a firework burst (2) */\n"
                % (frame_extra, frame_extra + 2))
        f.write("/* the screen moods' palette entries: water (deep, mid, light), the ride, the mirrors */\n")
        for name, c in (('WATER0', 'Z'), ('WATER1', 'A'), ('WATER2', 'Q'), ('RIDE0', 'X'), ('RIDE1', 'H'),
                        ('GLASS0', 'c'), ('GLASS1', 'D')):
            f.write("#define PE_%s %d\n" % (name, PAL_FIRST + palette.index(LETTER_RGB[c])))
        f.write("#define NLEVELS %d\n" % len(levels))
        f.write("#define NANIM %d\n#define ANIM_STEPS %d\n\n" % (len(ANIMATED), art.PUKE_ANIM))
        f.write("extern const unsigned char tile_attr[NTILES], tile_puke[NTILES], tile_under[NTILES];\n")
        f.write("extern const unsigned char anim_tile[NANIM];\n")
        f.write("extern const unsigned char pal_r[NPAL], pal_g[NPAL], pal_b[NPAL];\n")
        f.write("extern const unsigned char spr_pal_r[16], spr_pal_g[16], spr_pal_b[16];\n")
        f.write("extern const unsigned char level_w[NLEVELS], level_h[NLEVELS], level_sx[NLEVELS], level_sy[NLEVELS];\n")
        f.write("extern const unsigned int level_ofs[NLEVELS];    /* each map in the data */\n")
        f.write("extern const unsigned int level_name[NLEVELS];   /* each screen's name: a text page */\n")
        f.write("/* the crowd's starting squares: level n's are entries npc_first[n] to npc_first[n + 1] - 1 */\n")
        nsp = max(1, sum(len(l[5]) for l in levels))
        f.write("extern const unsigned char npc_first[NLEVELS + 1], npc_sx[%d], npc_sy[%d], npc_type[%d];\n" % (nsp, nsp, nsp))

    with open(os.path.join(out, "assets.c"), "w") as f:
        f.write("/* generated by tools/gen_assets.py - do not edit */\n#include \"assets.h\"\n\n")
        f.write(carray("unsigned char", "tile_attr", [t[1] for t in TILES]))
        f.write(carray("unsigned char", "tile_puke", [TILE_ID[t[3]] if t[3] else 255 for t in TILES]))
        f.write(carray("unsigned char", "tile_under", [TILE_ID[t[4]] if t[4] else 255 for t in TILES]))
        f.write(carray("unsigned char", "anim_tile", [TILE_ID[n] for n, _ in ANIMATED]))
        f.write("/* the tile palette, 0-255 (not nibble-swapped: video.c tints it) */\n")
        f.write(carray("unsigned char", "pal_r", [c[0] for c in palette]))
        f.write(carray("unsigned char", "pal_g", [c[1] for c in palette]))
        f.write(carray("unsigned char", "pal_b", [c[2] for c in palette]))

        f.write(carray("unsigned char", "spr_pal_r", [nib(c[1][0]) for c in art.SPR_PAL]))
        f.write(carray("unsigned char", "spr_pal_g", [nib(c[1][1]) for c in art.SPR_PAL]))
        f.write(carray("unsigned char", "spr_pal_b", [nib(c[1][2]) for c in art.SPR_PAL]))
        f.write(carray("unsigned char", "level_w", [l[0] for l in levels]))
        f.write(carray("unsigned char", "level_h", [l[1] for l in levels]))
        f.write(carray("unsigned char", "level_sx", [l[2] for l in levels]))
        f.write(carray("unsigned char", "level_sy", [l[3] for l in levels]))
        f.write(carray("unsigned int", "level_ofs", level_ofs))
        f.write(carray("unsigned int", "level_name", name_ofs))
        first, n = [], 0
        for l in levels:
            first.append(n)
            n += len(l[5])
        first.append(n)
        spawns = [p for l in levels for p in l[5]] or [(0, 0, 0)]
        f.write(carray("unsigned char", "npc_first", first))
        f.write(carray("unsigned char", "npc_sx", [p[0] for p in spawns]))
        f.write(carray("unsigned char", "npc_sy", [p[1] for p in spawns]))
        f.write(carray("unsigned char", "npc_type", [p[2] for p in spawns]))

    preview(os.path.join(out, "preview.png"), frames)


if __name__ == "__main__":
    main()
