# Upsilon RPi Firmware Port: Implementation Reference

## Overview

This document describes the complete firmware port of zardam's Raspberry Pi integration (`zardam/epsilon@rpi`) into the Upsilon calculator firmware. The RPi app is compiled as a **built-in app** (like calculation, graph, settings, etc.), bypassing the N0100's lack of external app support entirely.

**Source repo:** https://github.com/zardam/epsilon/tree/rpi
**Target repo:** Upsilon (`/Users/martin/projects/Upsilon`, branch `upsilon-dev`)

---

## Build Flag

All RPi code is gated behind `ENABLE_RPI`. Normal builds are completely unaffected.

```bash
# Build WITH RPi app (for N0100 device):
make MODEL=n0100 ENABLE_RPI=1 OMEGA_USERNAME="Martin" EPSILON_I18N=en -j$(nproc)

# Build WITHOUT RPi app (normal, default):
make MODEL=n0100 -j$(nproc)

# Simulator build (RPi app appears, transferControl() is a no-op):
make PLATFORM=simulator ENABLE_RPI=1 -j$(nproc)
```

When `ENABLE_RPI=1`:
- `-DENABLE_RPI=1` is added to compiler flags
- `rpi` is appended to `EPSILON_APPS`
- `ion/src/device/shared/drivers/rpi.cpp` is compiled into device builds
- `ion/src/shared/dummy/rpi.cpp` is compiled into simulator builds (always, via dummy list)
- The ISR vector table slot for EXTI[9:5] points to `rpi_isr` instead of `0`
- SPI1 peripheral clock is enabled in `initClocks()`
- `Rpi::Device::init()` / `shutdown()` are called during board peripheral init/shutdown

---

## File Inventory

### New Files Created (15 files)

| File | Purpose |
|------|---------|
| `apps/rpi/app.h` | App class declaration (Descriptor, Snapshot) |
| `apps/rpi/app.cpp` | App implementation — launches `transferControl()` then returns to home |
| `apps/rpi/rpi_controller.h` | Minimal ViewController holding RpiView |
| `apps/rpi/rpi_controller.cpp` | Controller implementation |
| `apps/rpi/rpi_view.h` | Simple View (gray fill) |
| `apps/rpi/rpi_view.cpp` | View implementation — fills screen with 0x808080 |
| `apps/rpi/Makefile` | Build integration for the app |
| `apps/rpi/rpi_icon.png` | 55x56 home screen icon (from zardam's repo) |
| `apps/rpi/base.{en,fr,de,es,nl,pt,it,hu}.i18n` | Localization: "Raspberry Pi" / "RASPBERRY PI" |
| `ion/include/ion/rpi.h` | Public API: `Ion::Rpi::transferControl()` |
| `ion/src/device/shared/drivers/rpi.h` | Device driver header (pin config, init/shutdown, ISR) |
| `ion/src/device/shared/drivers/rpi.cpp` | Core device driver (SPI, DMA, EXTI, keyboard loop) |
| `ion/src/shared/dummy/rpi.cpp` | No-op stub for simulator/emscripten |

### Existing Files Modified (9 files)

| File | Change |
|------|--------|
| `build/config.mak` | Added `ENABLE_RPI ?= 0` flag + conditional `EPSILON_APPS += rpi` |
| `apps/home/apps_layout.csv` | Added `rpi` between `reader` and `settings` in both layout rows |
| `ion/include/ion.h` | Added `#include <ion/rpi.h>` |
| `ion/src/device/shared/boot/isr.c` | Forward-declared `rpi_isr`, placed at EXTI[9:5] slot (#ifdef guarded) |
| `ion/src/device/shared/drivers/board.cpp` | Added `Rpi::Device::init()` after Display, `shutdown()` before Display |
| `ion/src/device/shared/drivers/Makefile` | Conditionally adds `rpi.cpp` to device sources |
| `ion/src/device/n0100/drivers/board.cpp` | Added `apb2enr.setSPI1EN(true)` in `initClocks()` |
| `ion/src/device/n0110/drivers/board.cpp` | Same SPI1 clock enable |
| `ion/src/simulator/Makefile` | Added `rpi.cpp` to dummy sources list |

---

## Architecture

### App Layer

The app follows the standard Upsilon app pattern (same as Reader, Atomic, etc.):

```
Rpi::App
  ├── Descriptor  →  name: "Raspberry Pi", icon: RpiIcon
  ├── Snapshot    →  unpack() creates App via placement new
  └── didBecomeActive()
        ├── ::App::didBecomeActive(window)
        ├── Ion::Rpi::transferControl()    ← blocks until HOME pressed
        └── switchTo(homeAppSnapshot)      ← return to home screen
```

Key difference from zardam's original: Upsilon's `App` constructor takes only `Snapshot *` (not `Container *, Snapshot *`). We use `AppsContainer::sharedAppsContainer()` to get the container for navigation instead of storing a member pointer.

### Driver Layer

The driver (`ion/src/device/shared/drivers/rpi.cpp`) is a near-direct port of zardam's `ion/src/device/rpi.cpp`, using Upsilon's register abstractions (which are identical in API).

#### Hardware Configuration

```
SPI1 (APB2 bus, 0x40013000):
  - Slave mode, receive-only, 16-bit frames
  - Software slave management (SSM=1, SSI controlled by ISR)
  - RX DMA enabled

DMA2 Stream 2 Channel 3 (SPI1_RX):
  - Peripheral-to-memory, 16-bit
  - Circular mode, no memory increment
  - Source: SPI1.DR
  - Destination: Display DataAddress (0x60020000)
  - NDTR: 1 (single item, circular refills)

EXTI Line 6 (PA6 = Chip Select from Pi):
  - Both-edge trigger (rising + falling)
  - Routed via SYSCFG.EXTICR2 to GPIOA
  - NVIC IRQ 23 (EXTI[9:5])

Power: PB9 → P-channel MOSFET gate
  - LOW = Pi on, HIGH/analog = Pi off
```

#### ISR Flow (`rpi_isr`)

```
EXTI[9:5] fires on PA6 edge →
  Clear pending bit
  if PA6 HIGH (CS deasserted = rising edge):
    SSI = 1 (deselect SPI slave → stops receiving)
  else (CS asserted = falling edge):
    setDrawingArea(full screen, landscape)
    write MemoryWrite command to display
    SSI = 0 (select SPI slave → DMA starts receiving pixels)
```

#### Keyboard Loop (`transferControl`)

```
Fill screen gray (0x808080)
Power on Pi (PB9 output LOW)
Enable EXTI display bridge

Loop:
  scan = Keyboard::scan()
  if scan != lastScan:
    encode scan as 17-char hex string (":XXXXXXXXXXXXXXXX")
    Console::writeLine(buf)  // sends over USART3 at 115200
  if scan == 0x40:  // HOME key bit
    break

Disable EXTI display bridge
```

The hex encoding sends a `:` prefix followed by 16 hex digits representing the 64-bit keyboard state bitmap. The Pi's `uinput-serial-keyboard` daemon parses this over `/dev/ttyAMA0`.

---

## Pin Configuration Reference

### SPI1 (Display Bridge)

| STM32 Pin | Function | Pi GPIO | Pi Pin # | Direction |
|-----------|----------|---------|----------|-----------|
| PA5 | SPI1_SCK (AF5) | GPIO 11 (SCLK) | 23 | Pi → STM32 |
| PA6 | Chip Select (Input) | GPIO 8 (CE0) | 24 | Pi → STM32 |
| PA7 | SPI1_MOSI (AF5) | GPIO 10 (MOSI) | 19 | Pi → STM32 |

### UART (Keyboard Data)

| STM32 Pin | Function | Pi GPIO | Pi Pin # | Direction |
|-----------|----------|---------|----------|-----------|
| PD8 / PA9 | USART3 TX | GPIO 15 (RXD) | 10 | STM32 → Pi |
| PC11 / PA10 | USART3 RX | GPIO 14 (TXD) | 8 | Pi → STM32 |

Note: The exact UART pins depend on the console config for your board. Upsilon's console uses USART3 by default.

### Power Control

| STM32 Pin | Function | Connection |
|-----------|----------|------------|
| PB9 | GPIO Output | P-channel MOSFET gate (10k pull-up to Vbat) |

---

## Register Mapping (zardam → Upsilon)

All register APIs are identical between zardam's Epsilon and Upsilon:

| Register | Namespace | Notes |
|----------|-----------|-------|
| `GPIOA`, `GPIOB` | `Ion::Device::Regs` | Same GPIO class with MODER/ODR/IDR/AFR |
| `SPI1` | `Ion::Device::Regs` | Same CR1/CR2/DR fields including RXONLY, DFF |
| `DMA2` | `Ion::Device::Regs` | Same SCR/SNDTR/SPAR/SM0AR with stream index |
| `EXTI` | `Ion::Device::Regs` | Same IMR/RTSR/FTSR/PR mask registers |
| `NVIC` | `Ion::Device::Regs` | Same NVIC_ISER0/NVIC_ICER0 |
| `SYSCFG` | `Ion::Device::Regs` | Same EXTICR2.setEXTI() |
| `Display::setDrawingArea()` | `Ion::Device::Display` | Same function signature |
| `Display::CommandAddress` | `Ion::Device::Display` | 0x60000000 (FSMC bank 1) |
| `Display::DataAddress` | `Ion::Device::Display` | 0x60020000 (FSMC bank 1 + bit 17) |

---

## DMA Channel Assignment

| DMA Engine | Stream | Channel | Usage | Conflict? |
|------------|--------|---------|-------|-----------|
| DMA2 | Stream 0 | - | Display pushPixels (memory-to-memory) | No |
| DMA2 | Stream 2 | Ch 3 | **SPI1_RX (RPi display bridge)** | No |

No conflict: Stream 0 (display) and Stream 2 (RPi) are independent.

---

## Differences from zardam's Original

| Aspect | zardam/epsilon@rpi | Upsilon port |
|--------|-------------------|--------------|
| App constructor | `App(Container *, Snapshot *)` | `App(Snapshot *)` |
| Container access | Stored as `m_appsContainer` member | `AppsContainer::sharedAppsContainer()` |
| Build integration | Always compiled in | Gated behind `ENABLE_RPI` flag |
| ISR registration | Directly in vector table | `#ifdef ENABLE_RPI` conditional |
| SPI1 clock enable | Always enabled | `#ifdef ENABLE_RPI` conditional |
| Driver file location | `ion/src/device/rpi.cpp` | `ion/src/device/shared/drivers/rpi.cpp` |
| Header organization | `rpi.h` in device root | Split: `ion/include/ion/rpi.h` (public) + `drivers/rpi.h` (private) |
| i18n languages | 5 (de, en, es, fr, pt) | 8 (de, en, es, fr, hu, it, nl, pt) |
| Simulator support | Not addressed | Dummy stub in `ion/src/shared/dummy/rpi.cpp` |

---

## Conditional Compilation Summary

All hardware-specific code is guarded so normal builds are unaffected:

```
#ifdef ENABLE_RPI    (C/C++ preprocessor, set by -DENABLE_RPI=1)
  - isr.c: rpi_isr in vector table
  - board.cpp: Rpi::Device::init()/shutdown() calls
  - board.cpp (n0100/n0110): SPI1EN clock enable

ifeq ($(ENABLE_RPI),1)    (Make variable)
  - config.mak: EPSILON_APPS += rpi
  - config.mak: SFLAGS += -DENABLE_RPI=1
  - drivers/Makefile: ion_device_src += rpi.cpp
```

The dummy `rpi.cpp` is always compiled into simulator builds (unconditionally in the dummy source list) to satisfy the `Ion::Rpi::transferControl()` symbol from `ion/rpi.h` which is always included via `ion.h`.

---

## Testing Checklist

### Simulator Build (no hardware needed)
- [ ] `make PLATFORM=simulator ENABLE_RPI=1 -j$(nproc)` compiles without errors
- [ ] RPi icon appears on home screen in simulator
- [ ] Tapping icon does nothing visible (no-op transferControl) and returns to home

### Normal Build (RPi disabled)
- [ ] `make MODEL=n0100 -j$(nproc)` compiles without errors
- [ ] No RPi icon on home screen
- [ ] No RPi code in binary (no SPI1 init, no ISR)

### Device Build (RPi enabled)
- [ ] `make MODEL=n0100 ENABLE_RPI=1 -j$(nproc)` compiles without errors
- [ ] RPi icon appears on home screen
- [ ] Binary includes rpi_isr at EXTI[9:5] vector slot
- [ ] SPI1 clock enabled in APB2ENR

### Hardware Test (with Pi wired)
- [ ] Tapping RPi icon fills screen gray, then Pi takes over display
- [ ] Keyboard presses appear on Pi (check with `cat /dev/ttyAMA0`)
- [ ] Pressing HOME exits back to calculator home screen
- [ ] Pi powers on when app launches
- [ ] Display bridge works (Pi framebuffer visible on calculator LCD)

---

## Version History

| Date | Change |
|------|--------|
| 2026-01-31 | Initial port from zardam/epsilon@rpi to Upsilon. 15 new files, 9 modified files. |
