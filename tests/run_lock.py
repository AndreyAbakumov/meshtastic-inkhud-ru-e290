#!/usr/bin/env python3
from pathlib import Path
import subprocess,os
root=Path(__file__).resolve().parents[1]
build=root/'tests/build';build.mkdir(exist_ok=True)
source=(root/'firmware/src/graphics/niche/InkHUD/Applets/System/LockScreen/LockScreenApplet.cpp').read_text()
start=source.index('bool InkHUD::LockScreenApplet::input')
end=source.index('void InkHUD::LockScreenApplet::onRender')
(build/'lock_runtime.inc').write_text(source[start:end])
subprocess.run(['clang++','-std=c++17','-O1','-g','-fsanitize=address,undefined','-I'+str(root/'tests/host'),'-I'+str(build),'-I'+str(root/'firmware/src'),str(root/'tests/host/test_lock_runtime.cpp'),str(root/'firmware/src/mesh/Throttle.cpp'),'-o',str(build/'test_lock')],check=True)
subprocess.run([str(build/'test_lock')],check=True,env={**os.environ,'UBSAN_OPTIONS':'halt_on_error=1'})
