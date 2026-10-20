"""The art, drawn from shapes and ASCII: 32 x 32 squares and the 32 x 42 player

Every picture is a list of rows of palette letters: TILE_PAL for the
squares, SPR_PAL for the sprites ('.' is transparent). gen_assets.py packs
them. Placeholder-quality art made to show the resolution; hand-drawn art
can replace any of it later.
"""
import math
import random

N = 32                         # a square is N x N pixels
PUKE_ANIM = 4                  # stench frames

TILE_PAL = [
    ('g', (46, 120, 46)),      # grass
    ('G', (70, 156, 58)),      # grass, light
    ('h', (30, 92, 34)),       # grass, dark
    ('y', (176, 148, 100)),    # path
    ('Y', (198, 172, 124)),    # path, light
    ('z', (150, 122, 80)),     # path, dark
    ('b', (128, 82, 36)),      # wood
    ('B', (76, 44, 20)),       # wood, dark
    ('w', (240, 240, 236)),    # white
    ('r', (204, 40, 40)),      # red
    ('R', (128, 20, 24)),      # red, dark
    ('k', (20, 20, 24)),       # black
    ('s', (150, 150, 158)),    # metal
    ('S', (84, 84, 92)),       # metal, dark
    ('p', (176, 192, 44)),     # puke
    ('P', (118, 136, 22)),     # puke, dark
    ('q', (214, 214, 96)),     # puke, light
    ('x', (214, 136, 60)),     # puke, chunks
    ('v', (226, 250, 176)),    # stench
    ('V', (168, 214, 112)),    # stench, faint
    ('o', (236, 144, 32)),     # ticket
    ('O', (150, 80, 10)),      # ticket, dark
    ('m', (236, 236, 236)),    # mint
    ('e', (40, 140, 80)),      # bottle
    ('E', (20, 80, 44)),       # bottle, dark
    ('l', (240, 220, 60)),     # yellow
    ('f', (240, 120, 180)),    # pink
    ('t', (34, 100, 40)),      # leaves
    ('T', (20, 64, 26)),       # leaves, dark
    ('u', (60, 96, 200)),      # blue
]
TILE_PAL += [
    ('U', (34, 54, 140)),      # blue, dark
    ('A', (40, 90, 180)),      # water
    ('Q', (80, 140, 220)),     # water, light
    ('Z', (20, 50, 120)),      # water, deep
    ('X', (110, 56, 150)),     # purple
    ('H', (170, 110, 210)),    # purple, light
    ('M', (64, 64, 72)),       # asphalt
    ('j', (96, 186, 76)),      # grass, highlight
    ('K', (12, 52, 18)),       # grass, deepest
    ('n', (226, 200, 152)),    # path, highlight
    ('N', (120, 96, 60)),      # path, deepest
    ('a', (168, 112, 56)),     # wood, light
    ('i', (60, 130, 50)),      # leaves, light
    ('I', (110, 176, 70)),     # leaves, highlight
    ('c', (236, 236, 244)),    # metal, highlight
    ('d', (160, 30, 30)),      # red, mid
    ('D', (250, 250, 250)),    # white, bright
    ('W', (196, 196, 200)),    # white, shade
    ('F', (250, 200, 220)),    # pink, light
]



SPR_PAL = [
    ('.', (0, 0, 0)),
    ('k', (16, 16, 24)),       # outline
    ('s', (240, 190, 150)),    # skin
    ('S', (200, 140, 110)),    # skin, shade
    ('h', (110, 60, 20)),      # hair
    ('r', (210, 40, 40)),      # shirt
    ('R', (140, 20, 30)),      # shirt, shade
    ('u', (50, 80, 190)),      # trousers
    ('U', (30, 50, 130)),      # trousers, shade
    ('w', (250, 250, 250)),    # white
    ('n', (80, 50, 30)),       # shoes
    ('g', (150, 210, 80)),     # sick skin
    ('G', (100, 160, 50)),     # sick skin, shade
    ('p', (190, 200, 50)),     # puke
    ('P', (130, 140, 30)),     # puke, dark
    ('y', (250, 230, 80)),
]

def canvas(fill):
    return [[fill] * N for _ in range(N)]


def speckle(base, specks, seed):
    r = random.Random(seed)
    g = canvas(base)
    for y in range(N):
        for x in range(N):
            v = r.random()
            for c, p in specks:
                if v < p:
                    g[y][x] = c
                    break
                v -= p
    return g


def grass(seed=11):
    r = random.Random(seed)
    g = speckle('g', [('h', 0.10), ('G', 0.08)], seed)
    for _ in range(26):                      # blades: a light tip over a dark root
        x, y = r.randrange(N), r.randrange(2, N)
        g[y][x] = 'h'
        g[y - 1][x] = 'G'
        if r.random() < 0.5:
            g[y - 2][x] = 'j'
    return g


def path(seed=12):
    r = random.Random(seed)
    g = speckle('y', [('Y', 0.10), ('z', 0.10)], seed)
    for _ in range(14):                      # pebbles
        x, y = r.randrange(1, N - 2), r.randrange(1, N - 2)
        g[y][x] = g[y][x + 1] = 'Y'
        g[y + 1][x] = 'z'
        g[y + 1][x + 1] = 'N'
        g[y - 1][x] = 'n'
    return g


def rect(g, x0, y0, x1, y1, c):
    for y in range(max(0, y0), min(N, y1 + 1)):
        for x in range(max(0, x0), min(N, x1 + 1)):
            g[y][x] = c


def wood(g, x0, y0, x1, y1, horizontal, seed):
    """a plank: light top/left edge, dark bottom/right, grain"""
    r = random.Random(seed)
    rect(g, x0, y0, x1, y1, 'b')
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            if (horizontal and y == y0) or (not horizontal and x == x0):
                g[y][x] = 'a'
            elif (horizontal and y == y1) or (not horizontal and x == x1):
                g[y][x] = 'B'
            elif r.random() < 0.10:
                g[y][x] = 'B'


def shadow(g, x0, y0, x1, y1):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            if 0 <= x < N and 0 <= y < N and g[y][x] in 'gGjh':
                g[y][x] = 'K' if g[y][x] == 'h' else 'h'


def fence():
    g = grass(21)
    shadow(g, 0, 26, 31, 29)
    for x0 in (3, 25):
        wood(g, x0, 3, x0 + 4, 28, False, x0)
        g[3][x0 + 1] = g[3][x0 + 2] = 'a'
    for y0 in (7, 19):
        wood(g, 0, y0, 31, y0 + 3, True, y0)
        for x0 in (5, 27):
            g[y0 + 1][x0] = 'S'                  # nails
            g[y0 + 1][x0 - 1] = 'c'
    return g


def tent():
    g = canvas('r')
    for y in range(N):
        for x in range(N):
            stripe = (x // 8) % 2
            shade = y / N
            if stripe:
                g[y][x] = 'D' if shade < 0.35 else ('w' if shade < 0.75 else 'W')
            else:
                g[y][x] = 'r' if shade < 0.55 else ('d' if shade < 0.85 else 'R')
            if x % 8 == 7:
                g[y][x] = 'R' if not stripe else 'W'
    for x in range(N):                           # a scalloped valance
        d = int(2 + 2 * abs(math.sin((x + 0.5) * math.pi / 8)))
        for y in range(d):
            g[y][x] = 'R' if y == d - 1 else ('l' if (x // 8) % 2 else 'r')
    for x in range(N):
        g[N - 2][x] = 'R'
        g[N - 1][x] = 'K'
    return g


def tree():
    g = grass(31)
    shadow(g, 6, 26, 28, 31)
    wood(g, 13, 20, 18, 29, False, 5)
    r = random.Random(7)
    for y in range(N):
        for x in range(N):
            dx, dy = (x - 15.5) / 14.5, (y - 12.5) / 12.5
            d = dx * dx + dy * dy
            if d < 1.0 + r.uniform(-0.08, 0.08):
                light = -dx * 0.6 - dy * 0.8 + r.uniform(-0.35, 0.35)
                g[y][x] = 'I' if light > 0.75 else 'i' if light > 0.25 else 't' if light > -0.3 else 'T'
                if d > 0.86:
                    g[y][x] = 'T'
    return g


def gate(open_):
    g = path(41)
    for x0 in (0, 27):
        rect(g, x0, 0, x0 + 4, 31, 's')
        rect(g, x0 + 3, 0, x0 + 4, 31, 'S')
        rect(g, x0, 0, x0, 31, 'c')
        rect(g, x0 - 1, 0, x0 + 5, 2, 'S')
    if not open_:
        for y0 in (5, 22):
            rect(g, 5, y0, 26, y0 + 1, 's')
            rect(g, 5, y0 + 1, 26, y0 + 1, 'S')
        for x in range(7, 26, 4):
            rect(g, x, 3, x + 1, 27, 's')
            rect(g, x + 1, 3, x + 1, 27, 'S')
            g[3][x] = g[2][x] = 'c'
        shadow(g, 5, 28, 26, 30)
    return g


def bench():
    g = grass(51)
    shadow(g, 2, 25, 29, 29)
    for y0 in (3, 8):
        wood(g, 4, y0, 27, y0 + 3, True, y0)
    for y0 in (14, 18):
        wood(g, 2, y0, 29, y0 + 3, True, y0)
    for x0 in (5, 25):
        rect(g, x0, 22, x0 + 1, 27, 'S')
        g[22][x0] = 's'
    return g


def item(draw):
    g = grass(61)
    draw(g)
    return g


def ticket(g):
    shadow(g, 6, 22, 28, 24)
    rect(g, 4, 9, 27, 21, 'o')
    rect(g, 4, 9, 27, 9, 'l')
    rect(g, 4, 21, 27, 21, 'O')
    rect(g, 27, 9, 27, 21, 'O')
    for y in range(10, 21, 2):
        g[y][19] = 'O'                           # the perforation
    for x, y in [(4, 12), (4, 13), (4, 17), (4, 18), (27, 12), (27, 13), (27, 17), (27, 18)]:
        g[y][x] = 'g'                            # notches
    for x, y in [(7, 12), (8, 12), (9, 12), (7, 13), (7, 14), (8, 14), (9, 14), (9, 15),
                 (7, 16), (8, 16), (9, 16), (11, 12), (12, 12), (13, 12), (12, 13), (12, 14), (12, 15), (12, 16)]:
        g[y][x] = 'k'                            # "ST"
    for x in range(21, 26):
        g[13][x] = g[17][x] = 'O'


def mint(g):
    shadow(g, 8, 25, 26, 27)
    for y in range(N):
        for x in range(N):
            dx, dy = x - 15.5, y - 15.5
            d = math.hypot(dx, dy)
            if d < 10.5:
                a = (math.atan2(dy, dx) + d * 0.18) % (2 * math.pi)
                g[y][x] = 'r' if int(a / (math.pi / 4)) % 2 else 'D'
                if d > 9.5:
                    g[y][x] = 'W' if dx + dy > 0 else 'w'
                if dx + dy < -9 and d > 7.5:
                    g[y][x] = 'D'


def ale(g):
    shadow(g, 9, 27, 24, 29)
    for y in range(2, 28):
        half = 2 if y < 9 else (2 + (y - 9)) if y < 13 else 6
        for x in range(16 - half, 16 + half):
            g[y][x] = 'e' if x < 16 + half - 2 else 'E'
            if x == 16 - half + 1:
                g[y][x] = 'i'
    rect(g, 14, 2, 17, 3, 'S')
    rect(g, 14, 2, 15, 2, 'c')
    rect(g, 10, 15, 21, 22, 'l')
    rect(g, 10, 15, 21, 15, 'D')
    for x in range(12, 20):
        g[18][x] = 'O'
    for x in range(13, 19, 2):
        g[20][x] = 'r'


def flowers(g):
    r = random.Random(71)
    for _ in range(6):
        cx, cy = r.randrange(3, 29), r.randrange(4, 27)
        petal = 'f' if r.random() < 0.6 else 'l'
        g[cy + 1][cx] = g[cy + 2][cx] = 't'
        for dx, dy in [(0, -2), (-2, 0), (2, 0), (0, 2), (-1, -1), (1, -1), (-1, 1), (1, 1)]:
            g[cy + dy][cx + dx] = petal
        g[cy - 1][cx] = 'F' if petal == 'f' else 'D'
        g[cy][cx] = 'l' if petal == 'f' else 'o'


def puddle(bg, seed):
    r = random.Random(seed)
    g = [row[:] for row in bg]
    for y in range(N):
        for x in range(N):
            dx, dy = (x - 15.5) / 14.0, (y - 23.5) / 7.0
            d = dx * dx + dy * dy + r.uniform(-0.12, 0.12)
            if d < 1.0:
                if d > 0.78:
                    g[y][x] = 'P'
                else:
                    v = r.random()
                    g[y][x] = 'x' if v < 0.07 else ('q' if v < 0.17 else 'p')
                    if dx < -0.2 and dy < -0.2 and v > 0.9:
                        g[y][x] = 'v'           # a shine
    return g


def stench(pool, frame):
    """the pool with its stench, frame 0-3: lines in rows 0-15 only, so an
    animation step changes just the top 8 characters"""
    g = [row[:] for row in pool]
    for i, (x0, start) in enumerate([(7, 0), (15, 6), (23, 3)]):
        for k in range(9):
            y = (start + 16 - (frame * 4 + k)) % 16      # the top half only (animated)
            x = x0 + round(1.6 * math.sin((y + i * 2) * math.pi / 5))
            g[y][x] = 'v' if k < 5 else 'V'
    return g


TILES = {
    'GRASS': grass(), 'PATH': path(), 'FENCE': fence(), 'TENT': tent(), 'TREE': tree(),
    'GATE': gate(False), 'GATE_OPEN': gate(True), 'BENCH': bench(),
    'TICKET': item(ticket), 'MINT': item(mint), 'ALE': item(ale), 'FLOWERS': item(flowers),
}
PUKE_POOL = {'GRASS_PUKE': puddle(grass(), 3), 'PATH_PUKE': puddle(path(), 4)}
for name, pool in PUKE_POOL.items():
    TILES[name] = stench(pool, 0)



# --- the player: 32 x 42, his feet on the bottom row ---------------------

PW, PH = 32, 42
DIRS = ('down', 'up', 'left', 'right')


class Pic:
    def __init__(self):
        self.g = [['.'] * PW for _ in range(PH)]

    def put(self, x, y, c):
        if 0 <= x < PW and 0 <= y < PH:
            self.g[y][x] = c

    def rect(self, x0, y0, x1, y1, c):
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                self.put(x, y, c)

    def ellipse(self, cx, cy, rx, ry, pick):
        """pick(dx, dy) -> a letter or None, dx and dy from -1 to 1"""
        for y in range(PH):
            for x in range(PW):
                dx, dy = (x + 0.5 - cx) / rx, (y + 0.5 - cy) / ry
                if dx * dx + dy * dy < 1.0:
                    c = pick(dx, dy)
                    if c:
                        self.put(x, y, c)

    def outlined(self):
        out = [row[:] for row in self.g]
        for y in range(PH):
            for x in range(PW):
                if self.g[y][x] == '.':
                    for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1)):
                        xx, yy = x + dx, y + dy
                        if 0 <= xx < PW and 0 <= yy < PH and self.g[yy][xx] not in '.k':
                            out[y][x] = 'k'
                            break
        return out


def legs_front(p, step):
    for side, x0 in ((0, 10), (1, 17)):
        lift = 2 if step == 1 + side else 0
        p.rect(x0, 32, x0 + 5, 38 - lift, 'u')
        p.rect(x0 + (5 if side == 0 else 0), 32, x0 + (5 if side == 0 else 0), 38 - lift, 'U')
        p.rect(x0 - (1 - side), 39 - lift, x0 + 5 + side, 40 - lift, 'n')
        p.rect(x0 - (1 - side), 41 - lift, x0 + 5 + side, 41 - lift, 'k')


def torso_front(p, step, back=False):
    p.rect(9, 20, 22, 31, 'r')
    p.rect(11, 19, 20, 19, 'r')
    p.rect(20, 20, 22, 31, 'R')
    if not back:
        p.rect(13, 19, 18, 19, 'w')
        p.put(14, 20, 'w'); p.put(17, 20, 'w')
        p.rect(16, 21, 16, 30, 'R')
    else:
        p.rect(13, 19, 18, 19, 'R')
    p.rect(9, 31, 22, 31, 'n')
    if not back:
        p.put(15, 31, 'y'); p.put(16, 31, 'y')
    for side, x0 in ((0, 6), (1, 23)):
        swing = 0 if not step else (1 if (step == 1) == (side == 0) else -1)
        p.rect(x0, 20 + max(0, swing), x0 + 2, 29 + swing, 'r')
        p.rect(x0 + (2 if side else 0), 20 + max(0, swing), x0 + (2 if side else 0), 29 + swing, 'R')
        p.rect(x0, 30 + swing, x0 + 2, 31 + swing, 's')
        p.rect(x0, 32 + swing, x0 + 2, 32 + swing, 'S')


def head_front(p, face=True):
    p.rect(14, 16, 17, 18, 'S')
    p.ellipse(16, 10, 8.5, 8.5, lambda dx, dy: 'S' if dx > 0.55 or dy > 0.75 else 's')
    p.put(7, 10, 's'); p.put(7, 11, 'S'); p.put(24, 10, 'S'); p.put(24, 11, 'S')
    if face:
        p.ellipse(16, 8, 9, 8, lambda dx, dy: 'h' if dy < -0.35 or abs(dx) > 0.82 else None)
        for x in range(10, 22, 3):
            p.put(x, 5, 'h'); p.put(x + 1, 6, 'h')
        for x, y in ((11, 2), (12, 2), (13, 1), (10, 3), (18, 2)):
            p.put(x, y, 'y')
        for ex in (11, 18):
            p.rect(ex, 10, ex + 2, 10, 'w')
            p.put(ex + 1, 10, 'k'); p.put(ex + 1, 11, 'k'); p.put(ex + 2, 11, 'w')
            p.rect(ex, 8, ex + 2, 8, 'h')
        p.put(15, 13, 'S'); p.put(16, 13, 'S')
        p.rect(13, 15, 18, 15, 'R')
        p.put(12, 14, 'R'); p.put(19, 14, 'R')
    else:
        p.ellipse(16, 9, 9, 9, lambda dx, dy: 'h' if dy < 0.7 else None)
        for x, y in ((12, 3), (13, 2), (14, 2), (19, 4), (20, 5)):
            p.put(x, y, 'y')
        for y in range(6, 15, 3):
            p.put(12, y, 'k'); p.put(19, y + 1, 'k')   # strands


def side(p, step):
    """facing right"""
    # the back arm, behind
    swing = (0, 3, -3)[step]
    p.rect(13 - swing // 2, 20, 15 - swing // 2, 29, 'R')
    # legs: apart when walking
    spread = (0, 3, -3)[step]
    for leg, (x0, s) in enumerate(((13, -spread), (14, spread))):
        for y in range(32, 39):
            off = s * (y - 31) // 7
            p.rect(x0 + off, y, x0 + off + 4, y, 'U' if leg == 0 else 'u')
        off = s
        p.rect(x0 + off, 39, x0 + off + 6, 40, 'n')
        p.rect(x0 + off, 41, x0 + off + 6, 41, 'k')
    # torso
    p.rect(11, 20, 21, 31, 'r')
    p.rect(11, 20, 12, 31, 'R')
    p.rect(12, 19, 20, 19, 'r')
    p.rect(11, 31, 21, 31, 'n')
    # neck, head
    p.rect(14, 16, 18, 18, 'S')
    p.ellipse(17, 10, 7.5, 8.5, lambda dx, dy: 'S' if dy > 0.75 else 's')
    p.ellipse(16, 8, 8, 8, lambda dx, dy: 'h' if dy < -0.3 or dx < -0.15 else None)
    p.rect(24, 11, 24, 13, 's')                    # nose
    p.put(25, 12, 's')
    p.rect(20, 10, 21, 10, 'w'); p.put(21, 10, 'k'); p.put(21, 11, 'k')
    p.rect(20, 8, 22, 8, 'h')
    p.rect(21, 15, 23, 15, 'R')
    p.rect(14, 10, 15, 12, 'S')                    # ear
    for x, y in ((13, 2), (14, 2), (15, 1), (12, 3)):
        p.put(x, y, 'y')
    # the front arm
    p.rect(16 + swing, 20, 18 + swing, 29, 'r')
    p.rect(18 + swing, 20, 18 + swing, 29, 'R')
    p.rect(16 + swing, 30, 18 + swing, 32, 's')


def player(direction, step):
    p = Pic()
    if direction in ('down', 'up'):
        legs_front(p, step)
        torso_front(p, step, back=direction == 'up')
        head_front(p, face=direction == 'down')
        g = p.outlined()
    else:
        side(p, step)
        g = p.outlined()
        if direction == 'left':
            g = [row[::-1] for row in g]
    return g


def sick(g):
    return [[{'s': 'g', 'S': 'G'}.get(c, c) for c in row] for row in g]


def puking(n):
    """facing down, green, bent a little; n 0: a stream, 1: a splash"""
    p = Pic()
    legs_front(p, 0)
    torso_front(p, 0)
    head_front(p)
    p.rect(13, 14, 18, 16, 'k')                   # the open mouth
    p.rect(14, 15, 17, 15, 'P')
    g = sick(p.outlined())
    rnd = random.Random(n)
    for y in range(16, 42):
        w = 2 + (y - 16) // (6 if n == 0 else 3)
        for x in range(16 - w // 2, 16 + (w + 1) // 2):
            if 0 <= x < PW:
                g[y][x] = 'P' if rnd.random() < 0.25 else ('y' if rnd.random() < 0.08 else 'p')
    if n == 1:
        for x in range(4, 28):
            for y in (39, 40, 41):
                if rnd.random() < 0.7 - abs(x - 16) / 24:
                    g[y][x] = 'p' if rnd.random() < 0.8 else 'P'
    return g


def player_frames():
    """down, up, left, right x stand, step, step; the same sick; 2 puking"""
    normal = [player(d, s) for d in DIRS for s in range(3)]
    return normal + [sick(f) for f in normal] + [puking(0), puking(1)]


# --- the crowd: the player's pictures recoloured -------------------------
# All sprites share one palette (the multiplexer moves characters between
# hardware sprites, and each sprite's colours are its own palette entries),
# so the variety is in the pictures. Nobody else wears red: the player is
# easy to find.

NPC_VARIANTS = [
    {'r': 'u', 'R': 'U', 'u': 'n', 'U': 'k'},                        # blue shirt, brown trousers
    {'r': 'y', 'R': 'h', 'h': 'k', 'y': 'w'},                        # yellow shirt, black hair
    {'r': 'w', 'R': 'S', 'h': 'y', 'y': 'w', 'u': 'n', 'U': 'k'},    # white shirt, blond, brown trousers
]
NPC_FRAMES = 18                # 12 walking, 4 sick (standing), 2 puking


def recolour(g, m):
    return [[m.get(c, c) for c in row] for row in g]


def npc_frames(variant):
    """down, up, left, right x stand, step, step; the 4 standing ones sick;
    2 puking"""
    m = NPC_VARIANTS[variant]
    walk = [player(d, s) for d in DIRS for s in range(3)]
    sick_stand = [sick(player(d, 0)) for d in DIRS]
    return [recolour(g, m) for g in walk + sick_stand + [puking(0), puking(1)]]


# --- the Hugging Aunt: white hair in a bun, a blue dress, arms out --------

def aunt(direction, step):
    p = Pic()
    # shoes under the dress
    for side, x0 in ((0, 11), (1, 17)):
        lift = 2 if step == 1 + side else 0
        p.rect(x0, 39 - lift, x0 + 4, 40 - lift, 'n')
        p.rect(x0, 41 - lift, x0 + 4, 41 - lift, 'k')
    # the dress, wider down to the hem
    for y in range(19, 39):
        half = 6 + (y - 19) // 3
        for x in range(16 - half, 16 + half):
            p.put(x, y, 'U' if x >= 16 + half - 2 else 'u')
    p.rect(13, 19, 18, 19, 'w')                     # a lace collar
    p.rect(10, 38, 21, 38, 'w')                     # and hem
    swing = (0, 1, -1)[step]
    if direction in ('down', 'up'):
        for side in (0, 1):                         # both arms out to hug
            x0, x1 = (1, 9) if side == 0 else (23, 30)
            y0 = 21 + (swing if side == 0 else -swing)
            p.rect(x0, y0, x1, y0 + 2, 'u')
            p.rect(x0, y0 + 2, x1, y0 + 2, 'U')
            hx = 0 if side == 0 else 30
            p.rect(hx, y0 - 1, hx + 1, y0 + 2, 's')
        p.rect(14, 16, 17, 18, 'S')
        p.ellipse(16, 10, 8.5, 8.5, lambda dx, dy: 'S' if dx > 0.55 or dy > 0.75 else 's')
        if direction == 'down':
            p.ellipse(16, 8, 9, 8, lambda dx, dy: 'h' if dy < -0.35 or abs(dx) > 0.82 else None)
            for ex in (11, 18):                     # eyes behind round glasses
                p.rect(ex - 1, 9, ex + 3, 9, 'k')
                p.rect(ex - 1, 12, ex + 3, 12, 'k')
                p.rect(ex - 1, 9, ex - 1, 12, 'k')
                p.rect(ex + 3, 9, ex + 3, 12, 'k')
                p.put(ex + 1, 10, 'k'); p.put(ex + 1, 11, 'k')
            p.rect(13, 15, 18, 15, 'r')             # lipstick
            p.rect(14, 16, 17, 16, 'r')
            p.put(10, 13, 'r'); p.put(21, 13, 'r')  # rouge
        else:
            p.ellipse(16, 9, 9, 9, lambda dx, dy: 'h' if dy < 0.7 else None)
        p.ellipse(16, 1.5, 4, 3, lambda dx, dy: 'h')   # the bun
    else:
        p.rect(17, 21 + swing, 30, 23 + swing, 'u')    # one arm out ahead
        p.rect(17, 23 + swing, 30, 23 + swing, 'U')
        p.rect(29, 20 + swing, 31, 23 + swing, 's')
        p.rect(14, 16, 18, 18, 'S')
        p.ellipse(17, 10, 7.5, 8.5, lambda dx, dy: 'S' if dy > 0.75 else 's')
        p.ellipse(16, 8, 8, 8, lambda dx, dy: 'h' if dy < -0.3 or dx < -0.15 else None)
        p.ellipse(11, 3, 3.5, 3, lambda dx, dy: 'h')   # the bun, at the back
        p.rect(24, 11, 24, 13, 's')
        p.put(25, 12, 's')
        p.rect(19, 9, 23, 9, 'k'); p.rect(19, 12, 23, 12, 'k'); p.rect(19, 9, 19, 12, 'k')
        p.put(21, 10, 'k'); p.put(21, 11, 'k')
        p.rect(21, 15, 23, 15, 'r')
    g = p.outlined()
    g = recolour(g, {'h': 'w'})
    if direction == 'left':
        g = [row[::-1] for row in g]
    return g


# --- the cotton-candy kid: small, a cone of blue candy floss --------------

def kid(direction, step, ill=False, puke=None):
    p = Pic()
    skin, shade = ('g', 'G') if ill else ('s', 'S')
    # legs (short), shorts, shoes
    for side, x0 in ((0, 12), (1, 17)):
        lift = 2 if step == 1 + side else 0
        p.rect(x0, 34, x0 + 3, 37 - lift, skin)
        p.rect(x0 - (1 - side), 38 - lift, x0 + 3 + side, 39 - lift, 'n')
        p.rect(x0 - (1 - side), 40 - lift, x0 + 3 + side, 40 - lift, 'k')
    p.rect(11, 31, 21, 34, 'U')
    # a T-shirt
    p.rect(10, 24, 21, 31, 'y')
    p.rect(19, 24, 21, 31, 'h')
    p.rect(8, 24, 9, 28, 'y'); p.rect(22, 24, 23, 28, 'y')
    p.rect(8, 29, 9, 30, skin); p.rect(22, 29, 23, 30, skin)
    # head
    side_view = direction in ('left', 'right')
    cx = 17 if side_view else 16
    p.ellipse(cx, 18, 6.5, 6.5, lambda dx, dy: shade if dy > 0.7 else skin)
    if direction == 'up':
        p.ellipse(16, 17, 7, 7, lambda dx, dy: 'n' if dy < 0.6 else None)
    elif side_view:
        p.ellipse(16, 16, 7, 6, lambda dx, dy: 'n' if dy < -0.3 or dx < -0.2 else None)
        p.put(22, 18, 'k')
        p.rect(23, 19, 23, 20, skin)
        if puke is None:
            p.rect(20, 22, 21, 22, 'R')
    else:
        p.ellipse(16, 16, 7, 6, lambda dx, dy: 'n' if dy < -0.35 else None)
        p.put(13, 18, 'k'); p.put(19, 18, 'k')
        if puke is None:
            p.rect(15, 21, 17, 21, 'R')
    # the candy floss on its stick, held up at the side
    if direction != 'up':
        sx = 25 if direction != 'left' else 6
        p.rect(sx, 10, sx, 28, 'w')
        p.ellipse(sx + 0.5, 6, 5, 6, lambda dx, dy: 'U' if dx > 0.5 and dy > 0.2 else 'u' if (int(dx * 7) + int(dy * 9)) % 3 else 'w')
    g = p.outlined()
    if puke is not None:                            # mouth open, the stream
        g[21][14:18] = list('kkkk')
        rnd = random.Random(puke)
        for y in range(22, 42):
            w = 2 + (y - 22) // (5 if puke == 0 else 2)
            for x in range(16 - w // 2, 16 + (w + 1) // 2):
                if 0 <= x < PW:
                    g[y][x] = 'P' if rnd.random() < 0.25 else 'p'
    if direction == 'left':
        g = [row[::-1] for row in g]
        if puke is None:
            pass
    return g


def aunt_frames():
    """18 like the crowd's (she never gets sick: those are her standing ones)"""
    walk = [aunt(d, s) for d in DIRS for s in range(3)]
    return walk + [walk[d * 3] for d in range(4)] + [walk[0], walk[0]]


def kid_frames():
    walk = [kid(d, s) for d in DIRS for s in range(3)]
    ill = [kid(d, 0, ill=True) for d in DIRS]
    return walk + ill + [kid('down', 0, ill=True, puke=0), kid('down', 0, ill=True, puke=1)]


# --- the corn dog stand ----------------------------------------------------

def corndog(puff):
    g = path(81)
    shadow_ = lambda: None
    for x in range(2, 30):                          # the cart's shadow on the path
        for y in (30, 31):
            g[y][x] = 'N'
    rect(g, 3, 13, 28, 27, 'o')                     # the cart
    rect(g, 26, 13, 28, 27, 'O')
    rect(g, 3, 27, 28, 27, 'O')
    rect(g, 2, 12, 29, 13, 'b')                     # its counter
    rect(g, 2, 13, 29, 13, 'B')
    for x in range(2, 30):                          # a striped awning, scalloped
        for y in range(2, 9):
            g[y][x] = 'r' if (x // 4) % 2 else 'w'
        d = 9 + (1 if (x % 4) in (1, 2) else 0)
        for y in range(9, d + 1):
            g[y][x] = 'R' if (x // 4) % 2 else 'W'
    rect(g, 3, 9, 3, 12, 'S'); rect(g, 28, 9, 28, 12, 'S')   # poles
    # a corn dog painted on the side: batter on a stick
    for y in range(16, 25):
        for x in range(7, 25):
            dx, dy = (x - 16) / 7.5, (y - 20) / 3.5
            if dx * dx + dy * dy < 1:
                g[y][x] = 'l' if dy < 0 else 'x'
    rect(g, 23, 19, 26, 20, 'B')
    for x, y in ((28, 28), (5, 28)):                # wheels
        rect(g, x - 2, y, x + 1, y + 3, 'k')
        g[y + 1][x - 1] = 'S'
    if puff:                                        # a stinking cloud over it
        r = random.Random(91)
        for cx, cy, rr in ((9, 5, 5), (16, 3, 6), (23, 6, 5), (13, 9, 4), (20, 10, 4)):
            for y in range(N):
                for x in range(N):
                    d = math.hypot(x - cx, y - cy)
                    if d < rr:
                        g[y][x] = 'P' if d > rr - 1 else ('V' if r.random() < 0.6 else 'q')
        for x0 in (6, 15, 24):                      # wavy lines rising
            for k in range(6):
                y = 14 - k
                g[y + 6][x0 + [0, 1, 1, 0, -1, -1][k]] = 'v'
    return g


TILES['CORNDOG'] = corndog(False)
TILES['CORNDOG_PUFF'] = corndog(True)


# --- the title logo: PUKE FEST in big slime letters ------------------------
# Each pixel of a 5 x 7 font is one 8 x 8 character cell. A filled cell
# takes one of 16 characters by which of its sides are open (no neighbour
# there): those get a dark outline, the top a highlight, the bottom a
# shadow. Drips hang under some bottom edges. '_' is the background.

FONT = {
    'P': ["####.", "#...#", "#...#", "####.", "#....", "#....", "#...."],
    'U': ["#...#", "#...#", "#...#", "#...#", "#...#", "#...#", ".###."],
    'K': ["#...#", "#..#.", "#.#..", "##...", "#.#..", "#..#.", "#...#"],
    'E': ["#####", "#....", "#....", "####.", "#....", "#....", "#####"],
    'F': ["#####", "#....", "#....", "####.", "#....", "#....", "#...."],
    'S': [".####", "#....", "#....", ".###.", "....#", "....#", "####."],
    'T': ["#####", "..#..", "..#..", "..#..", "..#..", "..#..", "..#.."],
}
LOGO_TEXT = "PUKE FEST"
LOGO_DRIPS = 3                 # rows of drips under the letters (and one for falling drops)
OPEN_T, OPEN_B, OPEN_L, OPEN_R = 1, 2, 4, 8
DRIP_MID, DRIP_END = 16, 17    # the drips' characters, after the 16 blocks
DROP_A, DROP_B = 18, 19        # under a drip: a falling drop (two sets, out of step)
# Palette cycling (video.c animates these entries; gen_assets.py reserves
# them): a drip's slime is painted in 4 entries, 2-pixel bands down the
# stream, '0'-'3' - rotating their colours makes it flow; a falling drop is
# 4 blobs one above the other, each its own entry, '4'-'7' (set A) or
# '8', '9', '<', '>' (set B) - lighting one at a time makes it fall. (Not
# letters: those are TILE_PAL's colours.)
LOGO_FLOW = '0123'
LOGO_DROP = ('4567', '89<>')


def logo_block(sides):
    r = random.Random(100 + sides)
    g = [['p'] * 8 for _ in range(8)]
    for y in range(8):
        for x in range(8):
            if r.random() < 0.06:
                g[y][x] = 'x'                        # chunks in an even fill
    if sides & OPEN_T:
        for x in range(8):
            g[0][x] = 'k'
            g[1][x] = 'v'
            g[2][x] = 'q'
    if sides & OPEN_B:
        for x in range(8):
            g[7][x] = 'k'
            g[6][x] = 'P'
            g[5][x] = 'P' if x % 3 else g[5][x]
    if sides & OPEN_L:
        for y in range(8):
            g[y][0] = 'k'
    if sides & OPEN_R:
        for y in range(8):
            g[y][7] = 'k'
    for cond, (x, y) in (((OPEN_T | OPEN_L), (0, 0)), ((OPEN_T | OPEN_R), (7, 0)),
                         ((OPEN_B | OPEN_L), (0, 7)), ((OPEN_B | OPEN_R), (7, 7))):
        if sides & cond == cond:
            g[y][x] = '_'                            # rounded corners
    return g


def logo_drip(end):
    g = [['_'] * 8 for _ in range(8)]
    for y in range(8):
        w = 2 if not end or y < 3 else (3 if y < 7 else 2)
        x0 = 4 - w // 2
        for x in range(x0 - 1, x0 + w + 1):
            if not end or y < 7:
                g[y][x] = 'k' if x in (x0 - 1, x0 + w) else LOGO_FLOW[(y // 2) % 4]
    if end:
        for x in range(3, 6):
            g[7][x] = 'k'
    return g


def logo_drop(letters):
    """4 blobs under a drip end, one per palette entry, top to bottom"""
    g = [['_'] * 8 for _ in range(8)]
    for k, c in enumerate(letters):
        for y in (2 * k, 2 * k + 1):
            for x in (3, 4):
                g[y][x] = c
    return g


def logo():
    """(the cell map: rows of character numbers or None, the characters)"""
    cols = []
    for ch in LOGO_TEXT:
        if ch == ' ':
            cols += [[0] * 7] * 3
            continue
        f = FONT[ch]
        for x in range(5):
            cols.append([1 if f[y][x] == '#' else 0 for y in range(7)])
        cols.append([0] * 7)
    cols.pop()
    w, h = len(cols), 7 + LOGO_DRIPS + 1
    filled = lambda x, y: 0 <= x < w and 0 <= y < 7 and cols[x][y]
    cells = [[None] * w for _ in range(h)]
    r = random.Random(7)
    for y in range(7):
        for x in range(w):
            if filled(x, y):
                sides = ((not filled(x, y - 1)) * OPEN_T | (not filled(x, y + 1)) * OPEN_B |
                         (not filled(x - 1, y)) * OPEN_L | (not filled(x + 1, y)) * OPEN_R)
                cells[y][x] = sides
                below = any(filled(x, yy) for yy in range(y + 1, 7))
                if sides & OPEN_B and not below and r.random() < 0.5:
                    n = r.randint(1, LOGO_DRIPS)
                    for k in range(n):
                        cells[y + 1 + k][x] = DRIP_END if k == n - 1 else DRIP_MID
                    cells[y + 1 + n][x] = DROP_A if r.random() < 0.5 else DROP_B
    chars = [logo_block(s) for s in range(16)] + [logo_drip(False), logo_drip(True),
                                                  logo_drop(LOGO_DROP[0]), logo_drop(LOGO_DROP[1])]
    return cells, chars


# --- the other screens' squares --------------------------------------------

def asphalt(seed=111):
    return speckle('M', [('S', 0.16), ('k', 0.06), ('s', 0.04)], seed)


def parkline():
    g = asphalt(112)
    for y in range(N):
        for x in (1, 2):
            g[y][x] = 'w' if (y // 2) % 8 else 'W'
    return g


def car(body, shade):
    g = asphalt(113)
    for y in range(3, 30):
        for x in range(6, 26):
            edge = y in (3, 29) or x in (6, 25)
            g[y][x] = shade if edge or x > 22 else body
    rect(g, 9, 7, 22, 11, 'u'); rect(g, 10, 8, 14, 9, 'c')     # windscreen
    rect(g, 9, 22, 22, 25, 'u')                                 # back window
    rect(g, 9, 13, 22, 20, shade)                               # roof
    rect(g, 10, 14, 21, 19, body)
    for x0, y0 in ((4, 6), (26, 6), (4, 22), (26, 22)):        # wheels
        rect(g, x0, y0, x0 + 1, y0 + 4, 'k')
    rect(g, 8, 3, 10, 3, 'l'); rect(g, 21, 3, 23, 3, 'l')       # headlights
    return g


def lamp():
    g = asphalt(114)
    for y in range(N):
        for x in range(N):
            if math.hypot(x - 15.5, y - 15.5) < 12 and g[y][x] in 'MSk':
                g[y][x] = 'S' if g[y][x] == 'M' else g[y][x]      # light on the ground
    rect(g, 14, 6, 17, 28, 'S'); rect(g, 14, 6, 14, 28, 's')    # the post
    rect(g, 10, 2, 21, 7, 'k'); rect(g, 11, 3, 20, 6, 'l')      # the lamp
    rect(g, 12, 3, 15, 4, 'D')
    return g


def ride_floor(seed=121):
    g = canvas('X')
    for y in range(N):
        for x in range(N):
            if (x + y) % 16 < 2 or (x - y) % 16 < 2:
                g[y][x] = 'H'
    return g


def teacup(spin):
    g = ride_floor()
    for y in range(N):
        for x in range(N):
            d = math.hypot(x - 15.5, y - 16.5)
            if d < 13:
                g[y][x] = 'w' if d > 11.5 else ('f' if d > 8 else ('F' if d > 3 else 'O'))
    rect(g, 27, 13, 30, 19, 'w'); rect(g, 28, 15, 29, 17, 'X')  # the handle
    for x in (8, 15, 22):                                       # dots on the cup
        g[16][x] = 'w'
    if spin:                                                    # whirling lines around it
        for k in range(48):
            a = k * math.pi / 24
            for r in (13.5, 14.5):
                x, y = int(15.5 + r * math.cos(a)), int(16.5 + r * math.sin(a))
                if 0 <= x < N and 0 <= y < N and k % 6 < 3:
                    g[y][x] = 'v'
        for k in range(10):
            a = k * 0.6
            x, y = int(15.5 + 6 * math.cos(a)), int(16.5 + 6 * math.sin(a))
            g[y][x] = 'v'
    return g


def checker():
    g = canvas('w')
    for y in range(N):
        for x in range(N):
            if (x // 8 + y // 8) % 2:
                g[y][x] = 'k'
            elif x % 8 == 7 or y % 8 == 7:
                g[y][x] = 'W'
    return g


def mirror():
    g = canvas('X')
    rect(g, 2, 1, 29, 28, 'H')                                  # the frame
    rect(g, 4, 3, 27, 26, 'c')                                  # the glass
    for y in range(3, 27):
        for x in range(4, 28):
            if (x + y) % 11 in (0, 1):
                g[y][x] = 'D'                                   # a shine
            elif x > 20:
                g[y][x] = 's'
    rect(g, 0, 29, 31, 31, 'k')
    return g


def water(seed=131):
    r = random.Random(seed)
    g = canvas('A')
    for y in range(N):
        for x in range(N):
            w = math.sin((x + 2 * y) * 0.4) + r.uniform(-0.4, 0.4)
            g[y][x] = 'Q' if w > 0.9 else ('Z' if w < -0.9 else 'A')
    return g


def bridge():
    g = water(132)
    for y in range(N):
        for x in range(2, 30):
            g[y][x] = 'B' if y % 6 in (0, 5) else ('a' if y % 6 == 1 else 'b')
    rect(g, 2, 0, 3, 31, 'B'); rect(g, 28, 0, 29, 31, 'B')     # rails
    for y in range(2, N, 6):
        g[y][5] = g[y][26] = 'k'                                # nails
    return g


def launcher(lit):
    g = grass(141)
    rect(g, 11, 8, 20, 27, 'R'); rect(g, 12, 8, 19, 26, 'r')   # the tube
    rect(g, 13, 8, 14, 26, 'l')
    rect(g, 10, 6, 21, 8, 'k')                                  # its mouth
    rect(g, 10, 27, 21, 29, 'B')                                # a crate under it
    if lit:                                                     # a fizzing fuse, sparks
        rect(g, 21, 18, 26, 19, 'k')
        r = random.Random(142)
        for _ in range(22):
            x, y = 26 + r.randint(-4, 4), 18 + r.randint(-5, 4)
            if 0 <= x < N and 0 <= y < N:
                g[y][x] = r.choice('lDo')
        for x in range(12, 20, 2):
            g[4][x] = g[2][x + 1] = 'v'
    return g


TILES.update({
    'ASPHALT': asphalt(), 'PARKLINE': parkline(), 'CAR_RED': car('r', 'R'), 'CAR_BLUE': car('u', 'U'),
    'LAMP': lamp(), 'RIDE': ride_floor(), 'TEACUP': teacup(False), 'TEACUP_SPIN': teacup(True),
    'CHECKER': checker(), 'MIRROR': mirror(), 'WATER': water(), 'BRIDGE': bridge(),
    'LAUNCHER': launcher(False), 'LAUNCHER_LIT': launcher(True),
})
for base, seed in (('ASPHALT', 5), ('RIDE', 6), ('CHECKER', 7), ('BRIDGE', 8)):
    PUKE_POOL[base + '_PUKE'] = puddle(TILES[base], seed)
    TILES[base + '_PUKE'] = stench(PUKE_POOL[base + '_PUKE'], 0)


# --- the paper bag ----------------------------------------------------------

def bag_tile():
    g = grass(151)
    shadow(g, 9, 26, 25, 28)
    for y in range(8, 26):
        w = 6 + (y - 8) // 6
        for x in range(16 - w, 16 + w):
            g[y][x] = 'z' if x >= 16 + w - 2 else ('n' if x <= 16 - w + 1 else 'Y')
    for x in range(10, 22):                        # the folded top
        g[8 + (x % 3)][x] = 'n'
        g[7][x] = 'B' if x % 2 else 'z'
    rect(g, 14, 15, 17, 18, 'z')                   # a crease
    for y in range(8, 26):
        w = 6 + (y - 8) // 6
        g[y][16 - w - 1] = g[y][16 + w] = 'B'
    rect(g, 9, 26, 22, 26, 'B')
    return g


TILES['BAG'] = bag_tile()


def flying_bag(tilt):
    """the full bag in the air (sprite colours), upright or on its side"""
    p = Pic()
    if tilt:
        p.rect(9, 18, 22, 25, 'S')
        p.rect(9, 18, 10, 25, 'h')
        p.rect(21, 20, 23, 23, 'h')                # the crumpled top
        p.put(24, 21, 'p'); p.put(25, 22, 'p')     # a drip
    else:
        p.rect(11, 14, 20, 28, 'S')
        p.rect(19, 14, 20, 28, 'h')
        for x in range(11, 21, 2):
            p.put(x, 13, 'h')                      # the crumpled top
        p.put(15, 29, 'p'); p.put(16, 30, 'p')
    return p.outlined()


def burst(big):
    """a firework bursting: rays and sparks"""
    p = Pic()
    r = random.Random(17 + big)
    n = 12
    for k in range(n):
        a = k * 2 * math.pi / n
        length = 13 if big else 8
        for d in range(2 if big else 1, length):
            x, y = int(16 + d * math.cos(a)), int(20 + d * math.sin(a) * 1.1)
            p.put(x, y, 'y' if d < length * 0.5 else ('w' if k % 2 else 'r'))
    for _ in range(14 if big else 6):
        p.put(r.randrange(2, 30), r.randrange(2, 40), r.choice('yrwu'))
    p.put(16, 20, 'w')
    return p.g


def extra_frames():
    """the bag in the air (2), a firework burst (2)"""
    return [flying_bag(0), flying_bag(1), burst(0), burst(1)]
