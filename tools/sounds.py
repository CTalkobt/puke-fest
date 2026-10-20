"""The sampled sounds, synthesized: 8-bit unsigned at RATE Hz

blargh(): a retch - a growl falling in pitch, mixed with noise, through
two vowel formants ("aaa" into "uhh"), a gurgling tremolo, a wet splash
at the end. splat(): a puddle stepped in.
"""
import math
import random

RATE = 8000                    # the first two; the others are SAMPLE_RATE
SAMPLE_RATE = 6000


class Biquad:
    """a band-pass (constant peak gain) or low/high-pass filter (RBJ cookbook)"""

    def __init__(self, kind, f, q):
        w = 2 * math.pi * f / RATE
        a = math.sin(w) / (2 * q)
        c = math.cos(w)
        if kind == 'band':
            b = (a, 0, -a)
        elif kind == 'low':
            b = ((1 - c) / 2, 1 - c, (1 - c) / 2)
        else:
            b = ((1 + c) / 2, -(1 + c), (1 + c) / 2)
        a0 = 1 + a
        self.b = [x / a0 for x in b]
        self.a = [-2 * c / a0, (1 - a) / a0]
        self.x1 = self.x2 = self.y1 = self.y2 = 0.0

    def __call__(self, x):
        y = self.b[0] * x + self.b[1] * self.x1 + self.b[2] * self.x2 - self.a[0] * self.y1 - self.a[1] * self.y2
        self.x2, self.x1, self.y2, self.y1 = self.x1, x, self.y1, y
        return y


def to_bytes(v):
    peak = max(abs(x) for x in v) or 1
    return [max(0, min(255, int(round(128 + 120 * x / peak)))) for x in v]


def blargh():
    r = random.Random(3)
    n = int(0.55 * RATE)
    f1, f2 = Biquad('band', 700, 2.5), Biquad('band', 1150, 3.5)
    splash = Biquad('high', 1800, 0.7)
    out, ph = [], 0.0
    for i in range(n):
        t = i / RATE
        ph += (150 - 80 * t / 0.55) / RATE                  # the growl falls
        src = 0.8 * (2 * (ph % 1) - 1) + 0.6 * r.uniform(-1, 1)
        gurgle = 0.55 + 0.45 * math.sin(2 * math.pi * 23 * t)
        env = min(1, t / 0.03) * (1 if t < 0.38 else max(0, 1 - (t - 0.38) / 0.17))
        k = min(1, t / 0.4)                                 # "aaa" turning to "uhh"
        v = (f1(src) * (1 - 0.5 * k) + f2(src) * (1 - k) + 0.3 * src * k) * gurgle * env
        if t > 0.36:                                        # the splash
            v += 0.9 * splash(r.uniform(-1, 1)) * math.exp(-(t - 0.36) * 18)
        out.append(v)
    return to_bytes(out)


def splat():
    r = random.Random(5)
    n = int(0.2 * RATE)
    out = []
    lp = Biquad('low', 2400, 0.9)
    for i in range(n):
        t = i / RATE
        if i % 200 == 0:                                    # the filter sweeps down
            lp = Biquad('low', 2400 - 9000 * t, 0.9) if t < 0.2 else lp
        v = lp(r.uniform(-1, 1)) * math.exp(-t * 22)
        for d in (0.09, 0.15):                              # drips
            if d < t < d + 0.01:
                v += 0.5 * math.sin(2 * math.pi * 1400 * (t - d)) * (1 - (t - d) / 0.01)
        out.append(v)
    return to_bytes(out)


def at_rate(rate):
    """synthesize at another rate: RATE is what the Biquads and loops read"""
    def wrap(f):
        def g():
            global RATE
            old, RATE = RATE, rate
            try:
                return f()
            finally:
                RATE = old
        return g
    return wrap


@at_rate(SAMPLE_RATE)
def ewww():
    """the crowd's disgust: a nasal "eeew", the pitch falling"""
    r = random.Random(9)
    n = int(0.42 * RATE)
    f1, f2 = Biquad('band', 320, 3.0), Biquad('band', 2200, 4.0)
    out, ph = [], 0.0
    for i in range(n):
        t = i / RATE
        ph += (260 - 120 * t / 0.42) / RATE
        src = 2 * (ph % 1) - 1 + 0.15 * r.uniform(-1, 1)
        k = min(1, t / 0.3)                          # "ee" opening to "oo"
        env = min(1, t / 0.04) * max(0, 1 - max(0, t - 0.3) / 0.12)
        out.append((f1(src) + f2(src) * (1 - k)) * env)
    return to_bytes(out)


@at_rate(SAMPLE_RATE)
def oof():
    """bumped into: a short grunt"""
    r = random.Random(10)
    n = int(0.16 * RATE)
    f1 = Biquad('band', 500, 2.5)
    out, ph = [], 0.0
    for i in range(n):
        t = i / RATE
        ph += (150 - 60 * t / 0.16) / RATE
        src = 2 * (ph % 1) - 1 + 0.3 * r.uniform(-1, 1)
        out.append(f1(src) * min(1, t / 0.01) * max(0, 1 - t / 0.16))
    return to_bytes(out)


@at_rate(SAMPLE_RATE)
def whoosh():
    """the bag thrown: noise swept up and down"""
    r = random.Random(11)
    n = int(0.22 * RATE)
    out = []
    for i in range(n):
        t = i / RATE
        f = Biquad('band', 400 + 2200 * math.sin(math.pi * t / 0.22), 2.0) if i % 60 == 0 else f
        out.append(f(r.uniform(-1, 1)) * math.sin(math.pi * t / 0.22))
    return to_bytes(out)


@at_rate(SAMPLE_RATE)
def thud():
    """the bag hits: a low thump and a wet slap"""
    r = random.Random(12)
    n = int(0.14 * RATE)
    lp = Biquad('low', 900, 0.8)
    out = []
    for i in range(n):
        t = i / RATE
        v = math.sin(2 * math.pi * (110 - 300 * t) * t) * math.exp(-t * 30)
        v += 0.7 * lp(r.uniform(-1, 1)) * math.exp(-t * 18)
        out.append(v)
    return to_bytes(out)


@at_rate(SAMPLE_RATE)
def whistle():
    """a firework going up: a falling whistle"""
    r = random.Random(13)
    n = int(0.32 * RATE)
    out, ph = [], 0.0
    for i in range(n):
        t = i / RATE
        ph += (2400 - 1500 * t / 0.32) / RATE
        out.append((math.sin(2 * math.pi * ph) + 0.1 * r.uniform(-1, 1)) * min(1, t / 0.03) * (1 - t / 0.32))
    return to_bytes(out)


# name, its maker, its rate: SOUNDS in the data (bank 1 at $12000), NEAR_SOUNDS
# in the art (banks 4-5). (Not $10000-$11FFF of bank 1: the DOS uses it -
# saving a file fails once something is loaded there.)
SOUNDS = [('BLARGH', blargh, RATE), ('SPLAT', splat, RATE)]
NEAR_SOUNDS = [('EWWW', ewww, SAMPLE_RATE), ('OOF', oof, SAMPLE_RATE), ('WHOOSH', whoosh, SAMPLE_RATE),
               ('THUD', thud, SAMPLE_RATE), ('WHISTLE', whistle, SAMPLE_RATE)]
