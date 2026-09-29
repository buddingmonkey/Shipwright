import json
import math
import os
import sys
from PIL import Image, ImageDraw, ImageFilter, ImageOps

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
SRC = os.path.join(ROOT, "soh", "macosx", "sohIcon.png")
RES = os.path.join(ROOT, "android", "app", "src", "main", "res")
IOS = os.path.join(ROOT, "soh", "ios", "Assets.xcassets", "AppIcon.appiconset")
VISION = os.path.join(ROOT, "soh", "visionos", "Assets.xcassets", "AppIcon.solidimagestack")
PREVIEW = sys.argv[1] if len(sys.argv) > 1 else None

TOP, BOTTOM = 83, 941
INNER = BOTTOM - TOP
BG_X = 200
KEY = 4
SOLID = 36
LAST_ROW = 880
FIRST_ROW = 140
HULL = 0.78

ANDROID_SIZE = 432
ANDROID_SAFE = 33 / 108
VISION_SIZE = 1024
VISION_SAFE = 0.40
APPLE_SIZE = 1024

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


def unmix(p, b):
    if all(p[c] <= b[c] + KEY for c in range(3)) and p[2] - p[0] >= (b[2] - b[0]) / 2:
        return (0, 0, 0, 0), False
    d = max(abs(p[c] - b[c]) for c in range(3))
    if d <= KEY:
        return (0, 0, 0, 0), False
    a = min(1.0, (d - KEY) / (SOLID - KEY))
    f = tuple(max(0, min(255, round((p[c] - (1 - a) * b[c]) / a))) for c in range(3))
    return f + (round(a * 255),), True


CUT = LAST_ROW - TOP
ship = Image.new("RGBA", (INNER, INNER * 3), (0, 0, 0, 0))
shp = ship.load()
ink = Image.new("L", ship.size, 0)
inkp = ink.load()
for y in range(FIRST_ROW, LAST_ROW + 1):
    b = rows.get(y, bg_at(y))
    for x in range(TOP, BOTTOM):
        if inner[x, y]:
            shp[x - TOP, y - TOP], solid = unmix(sp[x, y][:3], b)
            if solid:
                inkp[x - TOP, y - TOP] = shp[x - TOP, y - TOP][3]
for y in range(CUT + 1, ship.height):
    for x in range(INNER):
        shp[x, y] = shp[x, CUT]
        inkp[x, y] = inkp[x, CUT]

alpha = ink.load()
points = [(x / INNER, y / INNER) for y in range(0, CUT, 4) for x in range(0, INNER, 4) if alpha[x, y] > 64 and y < HULL * INNER]


def fit(safe):
    for f in range(150, 49, -1):
        frame = f / 100
        feasible = []
        for o in range(-100, 101):
            oy = o / 200
            if all(((ux - 0.5) * frame) ** 2 + (oy + uy * frame - 0.5) ** 2 <= safe * safe for ux, uy in points):
                feasible.append(oy)
        if feasible:
            return frame, (feasible[0] + feasible[-1]) / 2
    return 1.0, 0.0


def layers(size, frame, oy):
    px = size * frame
    k = px / INNER
    top = size * oy
    left = (size - px) / 2
    background = Image.new("RGB", (size, size))
    bp = background.load()
    for cy in range(size):
        col = bg_at(TOP + (cy + 0.5 - top) / k)
        for cx in range(size):
            bp[cx, cy] = col
    scaled = ship.resize((round(px), round(px * 3)), Image.LANCZOS)
    foreground = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    foreground.paste(scaled, (round(left), round(top)))
    mask = Image.new("L", (size, size), 0)
    mask.paste(ink.resize(scaled.size, Image.LANCZOS), (round(left), round(top)))
    return background, foreground, mask


def composite(background, foreground):
    out = background.convert("RGBA")
    out.alpha_composite(foreground)
    return out.convert("RGB")


def save(img, path, size=None):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    if size is not None and size != img.width:
        img = img.resize((size, size), Image.LANCZOS)
    img.save(path, optimize=True)


def circle(size, radius, scale=4):
    m = Image.new("L", (size * scale, size * scale), 0)
    c = size * scale / 2
    r = radius * scale
    ImageDraw.Draw(m).ellipse((c - r, c - r, c + r, c + r), fill=255)
    return m.resize((size, size), Image.LANCZOS)


a_frame, a_oy = fit(ANDROID_SAFE)
a_bg, a_fg, a_ink = layers(ANDROID_SIZE, a_frame, a_oy)
a_mono = Image.new("RGBA", a_fg.size, (255, 255, 255, 0))
a_mono.putalpha(a_ink)
viewport = round(ANDROID_SIZE * 72 / 108)
inset = (ANDROID_SIZE - viewport) // 2
a_full = composite(a_bg, a_fg).crop((inset, inset, inset + viewport, inset + viewport))
a_round = Image.new("RGBA", (viewport, viewport), (0, 0, 0, 0))
a_round.paste(a_full, (0, 0), circle(viewport, viewport / 2))
legacy = src.crop((TOP - 3, TOP - 3, BOTTOM + 3, BOTTOM + 3))

for name, scale in DENSITIES.items():
    d = os.path.join(RES, "mipmap-" + name)
    save(a_bg, os.path.join(d, "ic_launcher_background.png"), round(108 * scale))
    save(a_fg, os.path.join(d, "ic_launcher_foreground.png"), round(108 * scale))
    save(a_mono, os.path.join(d, "ic_launcher_monochrome.png"), round(108 * scale))
    save(legacy, os.path.join(d, "ic_launcher.png"), round(48 * scale))
    save(a_round, os.path.join(d, "ic_launcher_round.png"), round(48 * scale))

i_bg, i_fg, i_ink = layers(APPLE_SIZE, 1.0, 0.0)
i_tint = ImageOps.autocontrast(i_fg.convert("L"), cutoff=1).convert("RGBA")
i_tint.putalpha(i_ink)
save(composite(i_bg, i_fg), os.path.join(IOS, "AppIcon1024.png"))
save(i_fg, os.path.join(IOS, "AppIcon1024Dark.png"))
save(i_tint, os.path.join(IOS, "AppIcon1024Tinted.png"))
ios_entry = {"idiom": "universal", "platform": "ios", "size": "1024x1024"}
with open(os.path.join(IOS, "Contents.json"), "w") as f:
    json.dump(
        {
            "images": [
                dict(ios_entry, filename="AppIcon1024.png"),
                dict(ios_entry, filename="AppIcon1024Dark.png", appearances=[{"appearance": "luminosity", "value": "dark"}]),
                dict(ios_entry, filename="AppIcon1024Tinted.png", appearances=[{"appearance": "luminosity", "value": "tinted"}]),
            ],
            "info": {"author": "xcode", "version": 1},
        },
        f,
        indent=2,
        sort_keys=True,
        separators=(",", " : "),
    )
    f.write("\n")

v_frame, v_oy = fit(VISION_SAFE)
v_bg, v_fg, _ = layers(VISION_SIZE, v_frame, v_oy)
save(v_bg, os.path.join(VISION, "Back.solidimagestacklayer", "Content.imageset", "Back.png"))
save(v_fg, os.path.join(VISION, "Front.solidimagestacklayer", "Content.imageset", "Front.png"))

print("android frame %.2f oy %.3f, visionOS frame %.2f oy %.3f" % (a_frame, a_oy, v_frame, v_oy))

if PREVIEW:
    os.makedirs(PREVIEW, exist_ok=True)
    size = 432
    tiles = []

    def masked(img, mask):
        tile = Image.new("RGBA", (size, size), (0, 0, 0, 0))
        tile.paste(img.convert("RGBA").resize((size, size), Image.LANCZOS), (0, 0), mask)
        return tile

    a_comp = composite(a_bg, a_fg)
    v0, v1 = 72, 360
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
        tiles.append(masked(a_comp, m.resize((size, size), Image.LANCZOS)))
    guide = a_comp.convert("RGBA").resize((size, size), Image.LANCZOS)
    g = ImageDraw.Draw(guide)
    g.rectangle((v0, v0, v1, v1), outline=(255, 0, 0, 255), width=2)
    r = 33 * 4
    g.ellipse((216 - r, 216 - r, 216 + r, 216 + r), outline=(0, 255, 0, 255), width=2)
    tiles.append(guide)
    themed = Image.new("RGBA", (size, size), (40, 52, 70, 255))
    tint = Image.new("RGBA", (size, size), (190, 215, 255, 255))
    themed.paste(tint, (0, 0), a_mono.resize((size, size), Image.LANCZOS).getchannel("A"))
    tiles.append(masked(themed, circle(size, 144)))

    sq = Image.new("L", (size * 4, size * 4), 0)
    ImageDraw.Draw(sq).rounded_rectangle((0, 0, size * 4 - 1, size * 4 - 1), radius=round(size * 4 * 0.2237), fill=255)
    sq = sq.resize((size, size), Image.LANCZOS)
    tiles.append(masked(composite(i_bg, i_fg), sq))
    dark = Image.new("RGBA", (size, size), (28, 28, 30, 255))
    dark.alpha_composite(i_fg.resize((size, size), Image.LANCZOS))
    tiles.append(masked(dark, sq))
    tinted = Image.new("RGBA", (size, size), (20, 20, 20, 255))
    tl = i_tint.resize((size, size), Image.LANCZOS)
    tc = Image.new("RGBA", (size, size), (255, 170, 60, 255))
    tc.putalpha(Image.composite(ImageOps.grayscale(tl), Image.new("L", (size, size), 0), tl.getchannel("A")))
    tinted.alpha_composite(tc)
    tiles.append(masked(tinted, sq))

    vc = circle(size, size / 2)
    tiles.append(masked(composite(v_bg, v_fg), vc))
    shifted = v_bg.convert("RGBA")
    shifted.alpha_composite(v_fg.transform(v_fg.size, Image.AFFINE, (1, 0, -40, 0, 1, -30)))
    tiles.append(masked(shifted.convert("RGB"), vc))
    vg = composite(v_bg, v_fg).convert("RGBA").resize((size, size), Image.LANCZOS)
    r = size * VISION_SAFE
    ImageDraw.Draw(vg).ellipse((216 - r, 216 - r, 216 + r, 216 + r), outline=(0, 255, 0, 255), width=2)
    tiles.append(vg)

    cols = 6
    sheet = Image.new("RGBA", (size * cols, size * math.ceil(len(tiles) / cols)), (235, 235, 235, 255))
    for i, t in enumerate(tiles):
        sheet.alpha_composite(t, ((i % cols) * size, (i // cols) * size))
    sheet.convert("RGB").save(os.path.join(PREVIEW, "icon-preview.png"))
