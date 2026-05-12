# PuzzleSolver MCU

Embedded C firmware for the PREN2 pick-and-place puzzle solver.
Runs on an **STM32H753ZI** (Cortex-M7) Nucleo-144 board.

## How the System Works

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          Raspberry Pi  (camera + AI)                        │
│                                                                             │
│  1. Captures image of puzzle board                                          │
│  2. Determines piece pick positions, place positions & required rotation    │
│  3. Encodes up to 9 pieces as a PuzzleCommand (protobuf) and sends it       │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │  UART (length-prefixed protobuf frames)
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                        STM32 MCU  (this repo)                               │
│                                                                             │
│  Receives the command, then handles everything autonomously:                │
│  • Homing (finds machine zero via X/Y limit switches)                       │
│  • Piece-by-piece pick & place loop                                         │
│    – XY motion planning (mm → steps, trapezoidal acceleration)              │
│    – Simultaneous rotation of the piece during XY travel                    │
│    – Piston (lower / raise) + electromagnet (grab / release)                │
│  • Emergency stop monitoring and state recovery                             │
│  • Status LED feedback (green / yellow / red)                               │
│  • Sends Ack responses back to the Raspberry Pi                             │
└──────────────────────────────────┬──────────────────────────────────────────┘
                                   │  GPIO / Step+Dir / H-bridge signals
                                   ▼
                       ┌───────────────────────────────┐
                       │   DRV8886 stepper drivers (×3) │
                       │   Piston H-bridge              │
                       │   Electromagnet                │
                       └───────────────────────────────┘
```

### Operating Modes (`RUN_MODE` in `sys_config.h`)

| Mode                  | Behaviour                                                        |
|-----------------------|------------------------------------------------------------------|
| `RUN_MODE_APP`        | Production loop — polls UART for `PuzzleCommand`, runs solver   |
| `RUN_MODE_TEST_CLI`   | Interactive ASCII CLI over ST-Link UART for subsystem testing   |
| `RUN_MODE_TEST_STATE` | Single-shot state machine test with hard-coded piece coordinates |
| `RUN_MODE_LED`        | LED toggle test driven by Start/Reset buttons only              |

### Top-level State Machine

```
[power-on]
    │
    ▼
 IS_INIT ──► IS_HOMING ──► IS_READY ──► IS_RUNNING
                                              │
 IS_ESTOP ◄───────────────────────────────────┘
   │
   └──(E-stop released + Reset pressed)──► IS_HOMING
```

---

## Repository Structure

```
PuzzleSolver_MCU/
├── Core/
│   ├── Inc/                      # Header files (.h)
│   │   ├── actuators/            # Stepper, step generator, piston, magnet,
│   │   │                         #   rotator, limit switches, LEDs, homing, buttons
│   │   ├── communication/        # UART framing, protobuf dispatcher, generated pb headers
│   │   ├── coordination/         # Motion planner, high-level state machine
│   │   ├── system/               # sys_config.h, sys_init, app, interrupt state, utils
│   │   └── tests/                # test_cli — interactive UART test interface
│   │
│   ├── Src/                      # Implementation files (.c) — mirrors Inc/ layout
│   │   ├── actuators/
│   │   ├── communication/
│   │   ├── coordination/
│   │   ├── system/
│   │   └── tests/
│   │
│   └── third_party/
│       └── nanopb/               # Lightweight protobuf library for bare-metal C
│
├── tests/                        # Host-side (native) tests — no hardware required
│   ├── test_communication.c      # Native roundtrip test for UART framing + protobuf decode
│   ├── test_cross_language.py    # Python-to-C cross-language protobuf encode/decode test
│   └── …
│
├── Drivers/
│   ├── CMSIS/                    # ARM Cortex-M low-level definitions (do not modify)
│   └── STM32H7xx_HAL_Driver/     # ST HAL for STM32H7 (do not modify)
│
├── cmake/                        # CMake toolchain config for arm-none-eabi-gcc
├── CMakeLists.txt                # Build definition — add new .c files here
├── CMakePresets.json             # Debug and Release presets
├── PuzzleSolver_MCU.ioc          # STM32CubeMX project (pin/clock config)
├── STM32H753XX_FLASH.ld          # Linker script
├── startup_stm32h753xx.s         # Startup assembly
├── .vscode/                      # VS Code config (extensions, launch, tasks, settings)
└── docs/                         # Hardware reference documents
```

### Key Details for STM32CubeMX Projects

Some files are auto-generated when the `.ioc` is updated in STM32CubeMX. All user code lives in subfolders (`actuators/`, `communication/`, etc.). If `main.h` or `main.c` must change, **only edit between `/* USER CODE BEGIN */` and `/* USER CODE END */` markers** — everything outside is overwritten on regeneration.

> **IntelliSense:** after adding a new `.c` file, register it in `CMakeLists.txt`, rebuild, then reload VS Code (`Ctrl+Shift+P` → `Developer: Reload Window`). IntelliSense reads from `build/Debug/compile_commands.json`, generated by CMake.

---

## Communication Protocol

Frames are length-prefixed over UART:

```
[ len_hi ][ len_lo ][ protobuf payload ... ]
```

The payload is a serialised `PuzzleCommand` (nanopb / protobuf). Up to 9 `PieceCommand` entries are packed into one command, each carrying `pick_x`, `pick_y`, `place_x`, `place_y` (all in mm) and `rotation` (degrees).

The MCU responds to every frame with an `Ack` message containing a `Status` code:

| Status             | Meaning                                        |
|--------------------|------------------------------------------------|
| `STATUS_OK`        | Frame decoded successfully                     |
| `STATUS_ERROR`     | Protobuf decode failed                         |
| `STATUS_BUSY`      | Command accepted, execution started            |
| `STATUS_READY`     | Machine homed and waiting for a command        |
| `STATUS_DONE`      | All pieces placed, machine returned to idle    |

Proto definition lives in `Core/Inc/communication/puzzle.pb.h` (nanopb-generated).

---

## Prerequisites

### 1. ARM GCC Toolchain

```bash
# macOS
brew install --cask gcc-arm-embedded

# Ubuntu / Debian
sudo apt install gcc-arm-none-eabi

# Arch Linux
sudo pacman -S arm-none-eabi-gcc arm-none-eabi-newlib

# Windows
# Download from: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
# Install, then note the path to the bin/ folder (e.g. C:\ArmGNUToolchain\14.3.rel1\bin)
```

> **Arch Linux:** `arm-none-eabi-newlib` is required — it provides the C standard library for bare-metal targets (`--specs=nano.specs`).

Verify: `arm-none-eabi-gcc --version`

### 2. OpenOCD

Used to flash firmware and attach the debugger via ST-Link.

```bash
# macOS
brew install open-ocd

# Ubuntu / Debian
sudo apt install openocd

# Arch Linux
sudo pacman -S openocd

# Windows
# Download from: https://github.com/openocd-org/openocd/releases
```

### 3. CMake & Ninja

```bash
# macOS
brew install cmake ninja

# Ubuntu
sudo apt install cmake ninja-build

# Arch Linux
sudo pacman -S cmake ninja

# Windows: https://cmake.org/download/ and https://ninja-build.org/
```

### 4. VS Code Extensions

VS Code will prompt to install recommended extensions when the project is opened. Or open the Extensions panel (`Ctrl+Shift+X`), type `@recommended`, and install all **Workspace Recommendations**.

Extensions are defined in `.vscode/extensions.json`:

| Extension | Purpose |
|---|---|
| **C/C++** (ms-vscode.cpptools) | IntelliSense, code navigation |
| **Cortex-Debug** (marus25.cortex-debug) | Flashing & debugging via ST-Link |
| **CMake Tools** (ms-vscode.cmake-tools) | Build integration |
| **Clang-Format** (xaver.clang-format) | Auto-formatting on save |
| **Doxygen Documentation Generator** (cschlosser.doxdocgen) | Generate Doxygen comments |
| **vscode-pdf** (tomoki1207.pdf) | PDF viewer for datasheets |

---

## Build

```bash
# Configure (Debug)
cmake --preset Debug

# Build
cmake --build build/Debug

# Configure + Build (Release)
cmake --preset Release && cmake --build build/Release
```

Output: `build/Debug/PuzzleSolver_MCU.elf`

There are no on-target test or lint commands — this is bare-metal firmware flashed to hardware.

---

## Local Path Configuration

Debug and IntelliSense configs use environment variables so no machine-specific paths are committed. Set two variables pointing to the `bin/` folders of the installed tools.

**Windows** (Start menu → "Edit environment variables"):

```
ARM_TOOLCHAIN_PATH = C:\ArmGNUToolchain\14.3.rel1\bin
OPENOCD_PATH       = C:\openocd\bin
```

**macOS / Linux** (add to `~/.zshrc` or `~/.bashrc`):

```bash
export ARM_TOOLCHAIN_PATH=$(dirname $(which arm-none-eabi-gcc))
export OPENOCD_PATH=$(dirname $(which openocd))
```

Restart VS Code after setting the variables.

> If you prefer not to use environment variables, edit `.vscode/launch.json` directly with full paths — but do not commit those changes.

---

## Test CLI (`RUN_MODE_TEST_CLI`)

Set `RUN_MODE RUN_MODE_TEST_CLI` in `sys_config.h`, flash, then connect a serial terminal to the ST-Link Virtual COM Port (USART3, 115200 8N1).

| Command        | Description                                           |
|----------------|-------------------------------------------------------|
| `?`            | Print help                                            |
| `s`            | Print system state and actuator busy flags            |
| `h`            | Start homing sequence (blocks until done or timeout)  |
| `m <x> <y>`    | Move X/Y axes by step count (signed int32)            |
| `r <steps>`    | Move rotator by step count (signed int32)             |
| `p <0..3>`     | Set piston: 0=START, 1=MOVE, 2=GRAB, 3=RELEASE       |
| `g <0\|1>`     | Magnet off / on                                       |
| `l <0\|1>`     | Work-area LED off / on                                |
| `a <0..29>`    | Status LED: 0-9=green, 10-19=yellow, 20-29=red; last digit: 0=off, 1=blink, 2=on |
| `b <x> <y>`    | Move to absolute position in µm (via motion planner)  |

---

## Reference Docs

The `docs/` folder contains hardware references:

| Document | Content |
|---|---|
| `drv8886.pdf` | DRV8886 stepper motor driver datasheet |
| `ips4260lm.pdf` | IPS4260LM piston/H-bridge driver datasheet |
| `mb1364-h753zi-e01-schematic.pdf` | Nucleo-H753ZI board schematic |
| `um2407-stm32h7-nucleo144-boards-mb1364-stmicroelectronics.pdf` | Nucleo-144 user manual |
| `stm32h753zi.pdf` | STM32H753ZI datasheet |
| `pren_pinout_stm32.csv` | Project-specific pin assignment table |
| `PREN_schematic.pdf` | Project electrical schematic |

Software references:
- [STM32H7 HAL Documentation](https://www.st.com/resource/en/user_manual/um2217-description-of-stm32h7-hal-and-lowlayer-drivers-stmicroelectronics.pdf)
- [STM32CubeMX User Guide](https://www.st.com/resource/en/user_manual/um1718-stm32cubemx-for-stm32-configuration-and-initialization-c-code-generation-stmicroelectronics.pdf)
- [nanopb documentation](https://jpa.kapsi.fi/nanopb/)

---

## Pin Assignment (STM32H753ZI)

Only assigned pins are listed. Power, reset, oscillator, and unassigned I/O pins are omitted.

### Stepper X (DRV8886)

| Pin  | Label              | Direction |
|------|--------------------|-----------|
| PA0  | STEPPER_X_NSLEEP   | Output    |
| PB0  | STEPPER_X_STEP     | Output    |
| PB10 | STEPPER_X_DIR      | Output    |
| PB11 | STEPPER_X_M1       | Output    |
| PE0  | STEPPER_X_M0       | Output    |
| PE15 | STEPPER_X_ENABLE   | Output    |
| PE6  | STEPPER_X_NFAULT   | Input     |

### Stepper Y (DRV8886)

| Pin  | Label              | Direction |
|------|--------------------|-----------|
| PD12 | STEPPER_Y_STEP     | Output    |
| PE10 | STEPPER_Y_DIR      | Output    |
| PE12 | STEPPER_Y_M1       | Output    |
| PD11 | STEPPER_Y_M0       | Output    |
| PE7  | STEPPER_Y_ENABLE   | Output    |
| PD13 | STEPPER_Y_NSLEEP   | Output    |
| PE8  | STEPPER_Y_NFAULT   | Input     |

### Stepper Rotator (DRV8886)

| Pin  | Label               | Direction |
|------|---------------------|-----------|
| PF6  | STEPPER_ROT_STEP    | Output    |
| PB6  | STEPPER_ROT_DIR     | Output    |
| PB7  | STEPPER_ROT_M1      | Output    |
| PG6  | STEPPER_ROT_M0      | Output    |
| PG14 | STEPPER_ROT_ENABLE  | Output    |
| PF5  | STEPPER_ROT_NSLEEP  | Output    |
| PG13 | STEPPER_ROT_NFAULT  | Input     |

### UART / Communication

| Pin  | Signal      | Label      | Usage                          |
|------|-------------|------------|--------------------------------|
| PD8  | USART3_TX   | STLINK_RX  | ST-Link VCP — test CLI         |
| PD9  | USART3_RX   | STLINK_TX  | ST-Link VCP — test CLI         |
| PB12 | UART5_RX    | UART_RX    | Raspberry Pi — protobuf frames |
| PB13 | UART5_TX    | UART_TX    | Raspberry Pi — protobuf frames |

### Digital Outputs (DOUT)

| Pin  | Signal              | Label  |
|------|---------------------|--------|
| PB4  | MAGNET              | DOUT_1 |
| PF3  | LED (work area)     | DOUT_2 |
| PA4  | PISTON_EXTEND       | DOUT_3 |
| PD14 | PISTON_RETRACT      | DOUT_4 |
| PB3  | STATUS_LED_GREEN    | DOUT_5 |
| PD15 | STATUS_LED_YELLOW   | DOUT_6 |
| PC7  | STATUS_LED_RED      | DOUT_7 |
| PB5  | ISR_TIMING (test)   | DOUT_8 |

### Digital Inputs (DIN)

| Pin  | Signal         | Label  |
|------|----------------|--------|
| PE5  | LIM_X_MIN      | DIN_1  |
| PF10 | LIM_X_MAX      | DIN_2  |
| PE3  | LIM_Y_MIN      | DIN_3  |
| PB2  | LIM_Y_MAX      | DIN_4  |
| PF8  | —              | DIN_5  |
| PE9  | —              | DIN_6  |
| PF7  | —              | DIN_7  |
| PF2  | —              | DIN_8  |
| PF9  | EMERGENCY_STOP | DIN_9  |
| PF1  | BUTTON_START   | DIN_10 |
| PF0  | BUTTON_RESET   | DIN_11 |
