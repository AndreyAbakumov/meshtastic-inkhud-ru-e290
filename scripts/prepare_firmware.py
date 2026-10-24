#!/usr/bin/env python3
"""Create the pinned upstream checkout and apply the published RU patch."""
from pathlib import Path
import subprocess
root=Path(__file__).resolve().parents[1]
fw=root/'firmware'
if fw.exists():
    raise SystemExit('firmware/ already exists; use a fresh checkout or inspect it manually. Nothing changed.')
subprocess.run(['git','clone','--no-checkout','https://github.com/meshtastic/firmware.git',str(fw)],check=True)
subprocess.run(['git','checkout','--detach','54e0d8d0ab2ff56b3a9ce967e53f79e49af560fb'],cwd=fw,check=True)
subprocess.run(['git','submodule','update','--init','--recursive'],cwd=fw,check=True)
subprocess.run(['git','apply','--check',str(root/'patches/e290-inkhud-ru.patch')],cwd=fw,check=True)
subprocess.run(['git','apply',str(root/'patches/e290-inkhud-ru.patch')],cwd=fw,check=True)
print('Pinned E290 RU source is ready in firmware/. See docs/BUILDING.md.')
