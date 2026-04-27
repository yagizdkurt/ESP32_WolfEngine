Import("env")

import os
import subprocess
import sys

project_dir = env.subst("$PROJECT_DIR")
python = sys.executable

# Ensure Pillow is available in this Python environment.
# PlatformIO runs scripts in its own venv, which may not have Pillow.
try:
    import PIL
except ImportError:
    print("Asset converter: Pillow not found — installing into PlatformIO Python...")
    subprocess.check_call([python, "-m", "pip", "install", "Pillow"])

result = subprocess.run(
    [
        python,
        os.path.join(project_dir, "tools", "asset_converter.py"),
        os.path.join(project_dir, "Images"),
        os.path.join(project_dir, "src", "GeneratedAssets"),
        os.path.join(project_dir, "tools", "palettes"),
    ],
    cwd=project_dir,
)

if result.returncode != 0:
    print("ERROR: Asset converter failed (see output above)")
    env.Exit(result.returncode)
