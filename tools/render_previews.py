#!/usr/bin/env python3
"""Convert host GFX raster output to PNG; pixels come from production fonts and C++ rendering."""
from pathlib import Path
from PIL import Image, ImageDraw

root = Path(__file__).resolve().parents[1]
out = root / "dist/previews"
out.mkdir(parents=True, exist_ok=True)
for pgm in (root / "tests/build").glob("*.pgm"):
    Image.open(pgm).save(out / (pgm.stem + ".png"))
baseline = root / "baseline/preview/baseline.pgm"
if baseline.exists():
    Image.open(baseline).save(out / "baseline.png")
grid = Image.new("RGB", (624, 810), "#e5e7eb")
d = ImageDraw.Draw(grid)
for i in range(10):
    x, y = 10 + (i % 2) * 308, 10 + (i // 2) * 160
    d.text((x,y), f"E290 296 x 128 / sample {i+1}", fill="black")
    grid.paste(Image.open(out / f"screen-{i}.png"), (x,y+20))
grid.save(out / "overview.png")
print(out)

# These panels use extracted production Tips/Menu render methods with host service stubs.
ui = Image.new("RGB", (920, 1395), "#e5e7eb")
draw = ImageDraw.Draw(ui)
for tip in range(7):
    x, y = 4 + (tip % 3) * 304, 4 + (tip // 3) * 465
    draw.text((x, y), f"Tips / {tip}", fill="black")
    ui.paste(Image.open(out / f"tip-0-{tip}.png"), (x, y + 18))
    ui.paste(Image.open(out / f"tip-1-{tip}.png"), (x, y + 152))
ui.save(out / "ui-tips-overview.png")
menu = Image.new("RGB", (624, 470), "#e5e7eb")
menu.paste(Image.open(out / "menu-root-0.png"), (8, 8))
menu.paste(Image.open(out / "menu-options-0-0.png"), (320, 8))
menu.paste(Image.open(out / "menu-root-1.png"), (8, 150))
menu.paste(Image.open(out / "menu-options-1-0.png"), (160, 150))
menu.paste(Image.open(out / "menu-options-0-9.png"), (320, 150))
menu.save(out / "ui-menu-overview.png")
