# Machine Access Interlock Controller

![Machine Interlock Controller Board Assembly](hardware/Machine%20interlock%20controller%20assembly.png)


## Overview

The Machine Access Interlock Controller is an industrial safety device intended to be used in conjunction with the site access control system to ensure a machine can only be enabled (powered) when all safety interlocks are engaged (“sealed”), and the user is authorised. It disables the machine if any interlock is broken or a fault is detected. Upon enabling the machine, timers ensure the machine does not remain enabled once the user has finished. Machine run-time is detected via current transformer input, with configurable threshold, calibration, and amps-per-volt ratio. Designed for reliability and configurability, with persistent configuration stored in flash memory, and provides a serial terminal menu for setup and diagnostics.

---

## Features

- **Multiple Interlock Inputs:** Four independent interlock channels, each can be enabled/disabled in software.
- **Safety Relays:** Controls contactors and output gates to enable/disable machine power.
- **Current Sensing:** Monitors machine current via CT (current transformer) input, with configurable threshold, calibration, and amps-per-volt ratio.
- **Timers:** Configurable enable (pre-start) and pause (post-stop) timers.
- **Fault Detection:** Detects faults such as power supply undervoltage/overvoltage, CT disconnect, overcurrent, and running while disabled.
- **Status Indication:** RGB LEDs for status, enable, and interlock indication.
- **Persistent Configuration:** All settings are stored in flash memory and survive power cycles.
- **Serial Terminal Menu:** Interactive configuration and status via serial terminal (115200 baud), with ANSI color support.

---

## Hardware Overview

- **Inputs:**
  - 4× Interlock inputs (dry contact, pulled up)
  - 1× Enable input (dry contact, pulled up)
  - 1× CT analog input (current transformer)
  - 1× PSU voltage feedback
- **Outputs:**
  - Contactor 24VDC coil control
  - Machine status (running/idle)
  - Fault/alarm status
  - Status RGB LED
  - Enable and interlock LEDs
- **Board:** Custom board definition (`scion_isc_m0.json`), with pin mappings in `src/sys_init.h`.

---

## Configuration & Operation

- On power-up, the device loads configuration from flash. If not initialised, it writes defaults.
- The main loop continuously checks interlock status, current draw, and handles timers.
- If all interlocks are sealed and enable input is active, the machine can be enabled.
- If any interlock opens or a fault occurs, the machine is immediately disabled.
- Configuration (interlocks, timers, current settings) is accessible via the serial terminal menu.
- All changes can be saved to flash for persistence.

---

## Persistent Storage

- Uses a `flash_data` struct to store:
  - Interlock enable/disable states
  - Enable & pause timer values
  - Current threshold
  - CT calibration multiplier
  - Amps-per-volt ratio
- Data is validated on load; defaults are restored if out-of-range.

---

## Serial Terminal Menu

- Connect at **115200 baud**.
- Menu allows:
  - Enable/disable interlock inputs
  - Setting enable/pause timers
  - Adjusting current threshold
  - Setting CT ratio and calibration
- Changes can be saved to flash by typing `'s'` in the menu.

---

## Faults Detected

- Power supply undervoltage/overvoltage
- CT disconnected
- Overcurrent
- Running while disabled (tamper detection)

---

## Building & Flashing

- **Platform:** Arduino (via PlatformIO)
- **Board:** `scion_isc_m0` (custom board)
- **Build:**
  - Install [PlatformIO](https://platformio.org/)
  - Build and upload using PlatformIO, or use Arduino IDE with proper board support
- **Configuration:** See `platformio.ini` for environment and dependencies

---

## Project Structure

- `src/` – Main source code
- `hardware/` – Schematics, PCB, and pinout documentation
- `boards/` – Custom board definition
- `lib/` – Libraries (including FlashStorage for persistent config)
- `platformio.ini` – PlatformIO build configuration

---

## License

(C) 2024 Scion Research. For internal and research use. See source files for details.
