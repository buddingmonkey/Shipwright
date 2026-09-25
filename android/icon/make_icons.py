import os
import sys
from PIL import Image, ImageDraw, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
SRC = os.path.join(ROOT, "soh", "macosx", "sohIcon.png")
RES = os.path.join(ROOT, "android", "app", "src", "main", "res")
PREVIEW = sys.argv[1] if len(sys.argv) > 1 else None

TOP, BOTTOM = 83, 941
INNER = BOTTOM - TOP
CANVAS = round(INNER * 108 / 72)
OFF = (CANVAS - INNER) // 2 - TOP
BG_X = 200
KEY = 4
SOLID = 36
LAST_ROW = 880
FIRST_ROW = 140

DENSITIES = {"mdpi": 1.0, "hdpi": 1.5, "xhdpi": 2.0, "xxhdpi": 3.0, "xxxhdpi": 4.0}

src = Image.open(SRC).convert("RGBA")
sp = src.load()
inner = src.getchannel("A").point(lambda v: 255 if v == 255 else 0).filter(ImageFilter.MinFilter(41)).load()

rows = {y: sp[BG_X, y][:3] for y in range(TOP + 20, LAST_ROW + 1)}
ys = sorted(rows)
n = len(ys)
mean_y = sum(ys) / n
coef = []
for c in range(3):
    mean_v = sum(rows[y][c] for y in ys) / n
    slope = sum((y - mean_y) * (rows[y][c] - mean_v) for y in ys) / sum((y - mean_y) ** 2 for y in ys)
    coef.append((mean_v, slope))


def bg_at(y):
    return tuple(max(0, min(255, round(m + s * (y - mean_y)))) for m, s in coef)


background = Image.new("RGBA", (CANVAS, CANVAS))
bp = background.load()
for cy in range(CANVAS):
    col = bg_at(cy - OFF) + (255,)
    for cx in range(CANVAS):
        bp[cx, cy] = col

foreground = Image.new("RGBA", (CANVAS, CANVAS), (0, 0, 0, 0))
fp = foreground.load()


def unmix(p, b):
    d = max(abs(p[c] - b[c]) for c in range(3))
    if d <= KEY:
        return (0, 0, 0, 0)
    a = min(1.0, (d - KEY) / (SOLID - KEY))
    f = tuple(max(0, min(255, round((p[c] - (1 - a) * b[c]) / a))) for c in range(3))
    return f + (round(a * 255),)


for y in range(FIRST_ROW, LAST_ROW + 1):
    b = rows.get(y, bg_at(y))
    for x in range(TOP, BOTTOM):
        p = sp[x, y]
        if not inner[x, y]:
            continue
        fp[x + OFF, y + OFF] = unmix(p[:3], b)

for cy in range(LAST_ROW + OFF + 1, CANVAS):
    for cx in range(CANVAS):
        fp[cx, cy] = fp[cx, LAST_ROW + OFF]

monochrome = Image.new("RGBA", (CANVAS, CANVAS), (0, 0, 0, 0))
monochrome.putalpha(foreground.getchannel("A"))
mp = monochrome.load()
for cy in range(CANVAS):
    for cx in range(CANVAS):
        mp[cx, cy] = (255, 255, 255, mp[cx, cy][3])


def save(img, path, size):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    img.resize((size, size), Image.LANCZOS).save(path, optimize=True)


viewport = round(INNER)
inset = (CANVAS - viewport) // 2
composite = background.copy()
composite.alpha_composite(foreground)
full = composite.crop((inset, inset, inset + viewport, inset + viewport))
mask = Image.new("L", (viewport * 4, viewport * 4), 0)
ImageDraw.Draw(mask).ellipse((0, 0, viewport * 4 - 1, viewport * 4 - 1), fill=255)
mask = mask.resize((viewport, viewport), Image.LANCZOS)
round_icon = Image.new("RGBA", (viewport, viewport), (0, 0, 0, 0))
round_icon.paste(full, (0, 0), mask)
legacy = src.crop((TOP - 3, TOP - 3, BOTTOM + 3, BOTTOM + 3))

for name, scale in DENSITIES.items():
    d = os.path.join(RES, "mipmap-" + name)
    save(background, os.path.join(d, "ic_launcher_background.png"), round(108 * scale))
    save(foreground, os.path.join(d, "ic_launcher_foreground.png"), round(108 * scale))
    save(monochrome, os.path.join(d, "ic_launcher_monochrome.png"), round(108 * scale))
    save(legacy, os.path.join(d, "ic_launcher.png"), round(48 * scale))
    save(round_icon, os.path.join(d, "ic_launcher_round.png"), round(48 * scale))

if PREVIEW:
    os.makedirs(PREVIEW, exist_ok=True)
    size = 432
    bgl = background.resize((size, size), Image.LANCZOS)
    fgl = foreground.resize((size, size), Image.LANCZOS)
    mol = monochrome.resize((size, size), Image.LANCZOS)
    comp = bgl.copy()
    comp.alpha_composite(fgl)
    themed = Image.new("RGBA", (size, size), (40, 52, 70, 255))
    tint = Image.new("RGBA", (size, size), (190, 215, 255, 255))
    themed.paste(tint, (0, 0), mol.getchannel("A"))
    v0, v1 = 72, 360
    shapes = {}
    for shape in ("square", "circle", "squircle", "teardrop"):
        m = Image.new("L", (size * 4, size * 4), 0)
        dr = ImageDraw.Draw(m)
        box = (v0 * 4, v0 * 4, v1 * 4, v1 * 4)
        if shape == "square":
            dr.rectangle(box, fill=255)
        elif shape == "circle":
            dr.ellipse(box, fill=255)
        elif shape == "squircle":
            dr.rounded_rectangle(box, radius=90 * 4, fill=255)
        else:
            dr.rounded_rectangle(box, radius=144 * 4, fill=255)
            dr.rectangle((size * 2, size * 2, v1 * 4, v1 * 4), fill=255)
        shapes[shape] = m.resize((size, size), Image.LANCZOS)
    sheet = Image.new("RGBA", (size * 5, size * 2), (235, 235, 235, 255))
    for i, (name, m) in enumerate(shapes.items()):
        tile = Image.new("RGBA", (size, size), (0, 0, 0, 0))
        tile.paste(comp, (0, 0), m)
        sheet.alpha_composite(tile, (i * size, 0))
        tile = Image.new("RGBA", (size, size), (0, 0, 0, 0))
        tile.paste(themed, (0, 0), m)
        sheet.alpha_composite(tile, (i * size, size))
    guide = comp.copy()
    g = ImageDraw.Draw(guide)
    g.rectangle((v0, v0, v1, v1), outline=(255, 0, 0, 255), width=2)
    r = 33 * 4
    g.ellipse((216 - r, 216 - r, 216 + r, 216 + r), outline=(0, 255, 0, 255), width=2)
    sheet.alpha_composite(guide, (size * 4, 0))
    chk = Image.new("RGBA", (size, size), (255, 0, 255, 255))
    chk.alpha_composite(fgl)
    sheet.alpha_composite(chk, (size * 4, size))
    sheet.convert("RGB").save(os.path.join(PREVIEW, "icon-preview.png"))
