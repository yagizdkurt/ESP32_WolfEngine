# Installation & Setup

WolfEngine runs on ESP32 with a display, built with PlatformIO and the ESP-IDF framework. Although you can code and use any display driver, ST7735 TFT 128x160 driver is the engine default.

---

## Requirements

| Tool        | Recommended / Minimum |
|-------------|----------------------|
| IDE         | VS Code (recommended) |
| Python      | 3.8+ (required)       |
| Pip packages| see `tools/requirements.txt` (Pillow) |
| Build system| PlatformIO (recommended) or CMake + ESP-IDF |
| Board       | ESP32                 |
| Display     | ST7735 TFT 128x160 (default driver) |

**Notes:**
- `Python` is required by the asset pipeline (`tools/asset_converter.py`) and a small pre-build script. Install dependencies with `pip install -r tools/requirements.txt`.
- `PlatformIO` is the simplest way to build and flash the firmware (install the PlatformIO VS Code extension or the CLI).
- Desktop testing uses SDL3; see the "Desktop build" section below.

---

## Quick Setup (minimum steps)

1. Clone the repository:

```bash
git clone https://github.com/yagizdkurt/ESP32_WolfEngine.git
cd ESP32_WolfEngine
```

2. Install Python and project Python deps:

```bash
python -m pip install --upgrade pip
python -m pip install -r tools/requirements.txt
```

3. Install PlatformIO (VS Code extension or CLI). For CLI:

```bash
python -m pip install platformio
```

> **Tip:** You can also download it using VS code extensions.

### Using PlatformIO from VS Code (PIO UI)

If you prefer the graphical workflow, PlatformIO's VS Code extension provides a friendly UI (no terminal required). Quick steps:

- Open the project folder in VS Code (`File → Open Folder...`).
- Install and enable the **PlatformIO IDE** extension from the Extensions view.
- Open the PlatformIO sidebar by clicking its ant-helmet icon (left activity bar) or run `View → Command Palette → PlatformIO: Home`.

Table: common PIO UI actions

| Action (PIO UI) | Where to find it | Purpose |
|:---------------|:-----------------|:--------|
| Project Tasks → `env:esp32debug` → Build | PlatformIO sidebar → Project Tasks | Compile the firmware for the selected environment (`esp32debug`) |
| Project Tasks → `env:esp32debug` → Upload | PlatformIO sidebar → Project Tasks | Flash the compiled firmware to the connected ESP32 |
| Project Tasks → `env:esp32debug` → Monitor | PlatformIO sidebar → Project Tasks | Open the serial monitor (115200 baud) to view boot logs |
| Quick Access → Clean | PlatformIO sidebar → Quick Access / Project Tasks | Remove build cache and force regeneration (useful after asset changes) |
| PlatformIO: Select Serial Port | Command Palette (Ctrl/Cmd+Shift+P) | Choose the COM/tty port for upload/monitor if auto-detection fails |

Notes and tips:

- Select the environment to operate on (`esp32debug` or `esp32release`) under **Project Tasks** before clicking Build/Upload.
- The extension runs the `platformio.ini` build pipeline (including the `pre` extra scripts that run the asset converter) automatically.
- If the Upload task stalls, open **PlatformIO → Devices** or run **PlatformIO: Select Serial Port** to choose the correct COM/tty device.
- Use the **Monitor** task to view serial output; you can stop it with the stop button in the terminal tab.
- All task output appears in the integrated terminal panel; click the chevron to expand logs.

Using the PIO UI avoids typing commands and is ideal for quick iteration and for users new to PlatformIO.




The project contains PlatformIO `platformio.ini` with `esp32debug` and `esp32release` environments. The PlatformIO environment handles toolchain and IDF integration automatically.

---

## Additional Setup Notes

### Desktop build (SDL3)

To run the engine on your desktop using SDL3, install SDL3 dev libraries for your OS and build via CMake.

| Platform | Install SDL3 dev package |
|---------:|:-------------------------|
| macOS | `brew install sdl3` |
| Ubuntu / Debian | `sudo apt install libsdl3-dev build-essential cmake` |
| Windows | Download SDL3 development package and set `SDL3_DIR` to the CMake folder for your SDK in the CMake GUI or `-DSDL3_DIR=...` on the command line |

Quick desktop build (from repo root):

```bash
mkdir -p build && cd build
cmake -G "Ninja" -DSDL3_DIR=/path/to/SDL3/cmake ..\desktop
cmake --build .
./WolfEngine_Desktop
```

If you see an SDL window, the desktop build is working.

### SDL3 tips for Windows

- Install Visual Studio Build Tools (MSVC) or use MinGW-w64 for a GCC toolchain.
- Point `SDL3_DIR` to the extracted SDL3 `cmake` folder when running `cmake`.

### USB / Serial drivers (Windows)

If your ESP32 board uses a CP210x or CH340 serial chip, install the corresponding driver from the vendor website before attempting to flash or monitor the device.



### Troubleshooting (common problems)

| Symptom | Likely cause | Quick fix |
|:-------|:-------------|:---------|
| `Pillow` import error during build | Python dependency not installed | `python -m pip install -r tools/requirements.txt` |
| `SDL3` not found by CMake | SDL3 dev not installed or `SDL3_DIR` not set | Install SDL3 dev package; pass `-DSDL3_DIR=...` to CMake |
| Serial port not detected on Windows | Missing USB driver (CP210x/CH340) | Install vendor USB driver; replug device |
| `IDF_PATH` not found (native build) | ESP-IDF not sourced | Run `export IDF_PATH=...` or use ESP-IDF shell/setup script |
| Stale generated assets | Asset converter not run or build cache | `rm -rf src/GeneratedAssets && python tools/asset_converter.py ...` then rebuild |
