#!/usr/bin/env python3
"""Compile real InkHUD text methods and its pinned GFX rasterizer under ASan/UBSan."""
import os
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
FW = Path(os.environ.get("INKHUD_SOURCE", ROOT / "firmware"))
BUILD = ROOT / "tests/build"
BUILD.mkdir(exist_ok=True)
candidates = list((FW / ".pio/libdeps/heltec-vision-master-e290-inkhud").glob("GFX*/src"))
candidates += list(Path("/tmp/e290-gfx").glob("*/src"))
gfx = Path(os.environ.get("GFX_ROOT", candidates[0] if candidates else ""))
if not (gfx / "GFX.cpp").exists():
    sys.exit("Run PlatformIO dependency installation first, or set GFX_ROOT to pinned GFX_Root/src")

source = (FW / "src/graphics/niche/InkHUD/Applet.cpp").read_text()
names = ["setFont", "getFont", "parse", "getTextWidth", "printWrapped", "getWrappedTextHeight", "printAt",
         "ellipsizeToWidth", "parseShortName", "isPrintable", "hexifyNodeNum"]
methods = []
for match in re.finditer(r"^.*InkHUD::Applet::(" + "|".join(names) + r")\([^;]*?\n\{", source, re.M):
    pos = match.end()
    depth = 1
    while depth:
        depth += (source[pos] == "{") - (source[pos] == "}")
        pos += 1
    methods.append(source[match.start():pos])
(BUILD / "applet_methods.inc").write_text("\n\n".join(methods))
notification = (FW / "src/graphics/niche/InkHUD/Applets/System/Notification/NotificationApplet.cpp").read_text()
start = notification.index("std::string InkHUD::NotificationApplet::getNotificationText")
end = notification.index("\n}\n", start) + 2
(BUILD / "notification_method.inc").write_text(notification[start:end])
tips = (FW / "src/graphics/niche/InkHUD/Applets/System/Tips/TipsApplet.cpp").read_text()
tip_methods = []
for name in ("onRender", "renderWelcome"):
    start = tips.index("void InkHUD::TipsApplet::" + name)
    end = tips.index("\n}\n", start) + 2
    tip_methods.append(tips[start:end])
(BUILD / "tips_methods.inc").write_text("\n\n".join(tip_methods))
menu = (FW / "src/graphics/niche/InkHUD/Applets/System/Menu/MenuApplet.cpp").read_text()
menu_methods = []
for name in ("onRender", "drawSystemInfoPanel"):
    start = menu.index("void InkHUD::MenuApplet::" + name)
    end = menu.index("\n}\n", start) + 2
    menu_methods.append(menu[start:end])
(BUILD / "menu_methods.inc").write_text("\n\n".join(menu_methods))
catalog = json.loads((ROOT / "localization/ru.json").read_text())
(BUILD / "locale_expected.inc").write_text("\n".join(
    "assert(f.decodeUTF8(" + json.dumps(ru, ensure_ascii=False) + ") == INKHUD_TEXT(" + json.dumps(en) + "));"
    for en, ru in catalog.items()))
neutral = BUILD / "locale_neutral.cpp"
neutral.write_text('#include "graphics/niche/InkHUD/Localization.h"\n#include <cassert>\n#include <cstring>\n'
                   'int main() { assert(strcmp(INKHUD_TEXT("Back"), "Back") == 0); }\n')
subprocess.run(["clang++", "-std=c++17", "-I" + str(FW / "src"), str(neutral), "-o", str(BUILD / "locale_neutral")], check=True)
subprocess.run([str(BUILD / "locale_neutral")], check=True)
lock = (FW / "src/graphics/niche/InkHUD/Applets/System/LockScreen/LockScreenApplet.cpp").read_text()
start = lock.index("void InkHUD::LockScreenApplet::onRender")
end = lock.index("\n}\n", start) + 2
(BUILD / "lock_render.inc").write_text(lock[start:end])
cmd = ["clang++", "-std=c++17", "-g", "-O1", "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
       "-DMESHTASTIC_INCLUDE_INKHUD", "-DHELTEC_VISION_MASTER_E290", "-I" + str(ROOT / "tests/host"), "-I" + str(gfx),
       "-I" + str(FW / "src"), "-I" + str(BUILD), str(ROOT / "tests/host/test_text.cpp"),
       str(FW / "src/graphics/niche/InkHUD/AppletFont.cpp"), str(gfx / "GFX.cpp"),
       "-o", str(BUILD / "test_text")]
subprocess.run(cmd, check=True)
test_env = dict(os.environ)
if "--baseline" not in sys.argv:
    test_env["UBSAN_OPTIONS"] = "halt_on_error=1:print_stacktrace=1"
subprocess.run([str(BUILD / "test_text"), *sys.argv[1:]], cwd=ROOT, check=True, env=test_env)
