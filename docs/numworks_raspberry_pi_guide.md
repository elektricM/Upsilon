# NumWorks + Raspberry Pi Integration: Complete Technical Guide

## Project Overview

This project embeds a Raspberry Pi inside a NumWorks calculator, turning it into a full Linux computer. The calculator's display shows the Pi's output via SPI, and the calculator keyboard sends input to the Pi via UART.

**Original Project by:** Zardam (2018)
**Source:** https://zardam.github.io/post/raspberrypi-numworks/

---

## 1. Calculator Model Compatibility

### Your Calculators: N0100 and N0110

| Feature | N0100 | N0110 |
|---------|-------|-------|
| **MCU** | STM32F412 (Cortex-M4, 100 MHz) | STM32F730 (Cortex-M7, 216 MHz) |
| **Flash** | 1 MB internal only | 8 MB external QSPI + internal |
| **Internal Voltage** | 2.8V regulated | 2.8V regulated |
| **SPI Pads** | ✅ Available | ✅ Available |
| **UART Pads** | ✅ Available | ✅ Available |
| **Open Source Status** | Fully open (schematics available) | Fully open (schematics available) |
| **Custom Firmware** | Full support | Full support |

### ⚠️ Important: Both N0100 and N0110 are compatible with this project

The original project was built for the **N0100**, but it should work on the **N0110** with the same wiring since:
- Both expose the same SPI1 bus on test pads (PA5, PA6, PA7)
- Both have UART accessible
- Both have the same display interface (320×240 @ 16bpp)
- The STM32F730 (N0110) also has 5V-tolerant GPIO pins

**Recommendation:** Use the **N0100** for this project because:
1. It has more internal space (no external flash chip)
2. It's the model the firmware was originally written for
3. The N0110 is more valuable and harder to replace

---

## 2. Compatible Raspberry Pi Models

### Fully Compatible (Recommended)

| Model | Why Compatible | Notes |
|-------|----------------|-------|
| **Raspberry Pi Zero (non-W)** | ✅ Best choice | Works at 2.8V (with WiFi chip disabled naturally), fits inside calculator, lowest power |
| **Raspberry Pi Zero W** | ✅ Compatible with caveats | WiFi needs 3.0V+ minimum, so must disable WiFi or power from battery directly |
| **Raspberry Pi Zero 2 W** | ⚠️ Requires modifications | More power hungry, needs 3.3V+ stable, may not fit as well |

### Why These and Not Others?

#### Physical Size Constraints
The calculator has very limited internal space. Only the Pi Zero form factor (65mm × 30mm × 5mm) fits inside the case.

| Model | Dimensions | Fits in NumWorks? |
|-------|------------|-------------------|
| Pi Zero / Zero W | 65 × 30 × 5 mm | ✅ Yes |
| Pi Zero 2 W | 65 × 30 × 5 mm | ✅ Yes (tight) |
| Pi 3/4/5 | ~85 × 56 mm | ❌ No |
| Pi Pico | 51 × 21 mm | N/A (not a Linux SBC) |

#### Voltage Requirements

This is the **critical factor**:

| Power Source | Voltage | Pi Zero | Pi Zero W | Pi Zero 2 W |
|--------------|---------|---------|-----------|-------------|
| NumWorks internal regulator | 2.8V | ⚠️ Marginal | ❌ WiFi fails | ❌ Too low |
| Direct from LiPo battery | 3.0V–4.2V | ✅ Works | ✅ Works | ✅ Works |
| External 5V supply | 5V | ✅ Works | ✅ Works | ✅ Works |

**Key Insight from Zardam:**
> "The initial RPi I used was a Zero (without W). It seemed to work well when powered by 2.8V... But I realized that it will be sad to not include WiFi, so I ordered a 'W' version. It turns out that it is not happy with 2.8V. The WiFi chip needs at least 3V. Disabling WiFi made the RPi working at 2.8V."

**Final Solution:** Power the Pi directly from the battery (3.0V–4.2V), not from the 2.8V regulator.

---

## 3. Complete Hardware Requirements

### Electronics Components

| Component | Quantity | Specification | Purpose |
|-----------|----------|---------------|---------|
| Raspberry Pi Zero (W) | 1 | Any revision | Main compute |
| MicroSD Card | 1 | 8GB+ Class 10 | Storage |
| P-Channel MOSFET | 1 | NTR1P02LT1 or similar "logic level" | Power switching |
| Pull-up Resistor | 1 | 10 kΩ | MOSFET gate pull-up |
| Wire | ~15cm | 30 AWG (thin) | Connections |
| Double-sided adhesive tape | Small amount | Thin foam tape | Mounting Pi |

### MOSFET Requirements
Any P-channel MOSFET with:
- Logic-level gate (Vgs threshold < 2.5V)
- Continuous drain current ≥ 100mA (Pi Zero draws ~80-150mA typical)
- Low Rds(on) at low Vgs

**Alternatives to NTR1P02LT1:**
- Si2301CDS
- AO3401A
- IRLML6402

### Tools Required
- T5 Torx screwdriver (case screws)
- PH00 Phillips screwdriver (motherboard screws)
- Fine tip soldering iron (< 1mm tip)
- Thin solder wire (0.5mm recommended)
- Multimeter
- Fine tweezers
- Hot air station (optional, for SMD work)

---

## 4. Wiring Diagram

### NumWorks SPI Pads Location

The SPI pads are located on the main PCB near the edge. Looking at the board from behind:

```
┌─────────────────────────────────────────┐
│                NumWorks PCB             │
│                                         │
│    ┌─────┐                              │
│    │ SPI │                              │
│    │PADS │                              │
│    └─────┘                              │
│     ○ ○ ○                               │
│    PA7 PA6 PA5    (and GND nearby)      │
│   MOSI MISO CLK                         │
│                                         │
└─────────────────────────────────────────┘
```

### Pin Mapping: NumWorks ↔ Raspberry Pi

```
NumWorks (STM32)          Raspberry Pi Zero
─────────────────         ─────────────────
PA5 (SPI1_SCK)   ──────── GPIO 11 (SPI0_SCLK)  [Pin 23]
PA6 (SPI1_MISO)  ──────── GPIO 8  (SPI0_CE0)   [Pin 24] *Used as CS*
PA7 (SPI1_MOSI)  ──────── GPIO 10 (SPI0_MOSI)  [Pin 19]
GND              ──────── GND                   [Pin 6, 9, 14, 20, 25, 30, 34, 39]

UART_TX (PA9)    ──────── GPIO 15 (RXD)        [Pin 10]
UART_RX (PA10)   ──────── GPIO 14 (TXD)        [Pin 8]
```

### Complete Wiring Table

| Signal | NumWorks Pad | RPi Pin # | RPi GPIO | Wire Color (suggested) |
|--------|--------------|-----------|----------|------------------------|
| SPI Clock | PA5 | 23 | GPIO 11 (SCLK) | Yellow |
| Chip Select | PA6 | 24 | GPIO 8 (CE0) | Orange |
| SPI MOSI | PA7 | 19 | GPIO 10 (MOSI) | Green |
| Ground | GND | 6, 9, 14, etc. | GND | Black |
| UART RX (Pi receives) | PA9 (TX) | 10 | GPIO 15 | Blue |
| UART TX (Pi sends) | PA10 (RX) | 8 | GPIO 14 | White |

### Power Wiring

**Option A: Battery Direct (Recommended)**
```
Battery (+) ───┬─── Normal path to NumWorks
               │
               └─── MOSFET Source ─── MOSFET Drain ─── Pi 5V pin (Pin 2 or 4)
                         │
                    10kΩ │
                         │
               Control GPIO ─── MOSFET Gate
```

**Option B: SD Card Power Pads (Original Method)**
```
The NumWorks has pads for an SD card that are not populated.
The SD power control pin can be repurposed to switch Pi power.
Look for the transistor footprint near the SD card pads on the PCB.
```

### Visual Wiring Diagram

```
                    RASPBERRY PI ZERO (Top View, USB on right)
    ┌─────────────────────────────────────────────────────────────────┐
    │ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○                        │
    │ 1 3 5 7 9  ...                    ...39                        │
    │ 2 4 6 8 10 ...                    ...40                        │
    │ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○                        │
    │                                                                │
    │  [HDMI]        [USB x2]                        [microSD]       │
    └─────────────────────────────────────────────────────────────────┘
    
    Connections:
    Pin 2  (5V)     ← Battery via MOSFET
    Pin 6  (GND)    ← NumWorks GND
    Pin 8  (TXD)    ← NumWorks UART RX (PA10)
    Pin 10 (RXD)    ← NumWorks UART TX (PA9)
    Pin 19 (MOSI)   ← NumWorks SPI MOSI (PA7)
    Pin 23 (SCLK)   ← NumWorks SPI CLK (PA5)
    Pin 24 (CE0)    ← NumWorks SPI CS (PA6)
```

---

## 5. Software Requirements

### On the Raspberry Pi

#### 5.1 Operating System
- Raspberry Pi OS Lite (32-bit recommended for Zero)
- Standard Raspbian/Debian-based setup

#### 5.2 SPI Framebuffer Driver (spifb)

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install raspberrypi-kernel-headers build-essential git

# Clone and build the driver
git clone https://github.com/zardam/spifb.git
cd spifb
make -C /lib/modules/$(uname -r)/build M=$PWD
sudo make -C /lib/modules/$(uname -r)/build M=$PWD modules_install
sudo depmod -a
```

#### 5.3 Configuration Files

**/etc/modules** (add these lines):
```
spi-bcm2835
spifb
uinput
```

**/boot/config.txt** (add/modify these lines):
```ini
# Enable SPI
dtparam=spi=on

# Disable HDMI to save power
hdmi_blanking=2

# Enable UART for keyboard input
enable_uart=1

# Disable LED to save power
dtparam=act_led_trigger=none
dtparam=act_led_activelow=on

# If using Pi Zero W, disable WiFi to run at lower voltage:
# dtoverlay=pi3-disable-wifi
# dtoverlay=pi3-disable-bt
```

#### 5.4 Keyboard Daemon

```bash
git clone https://github.com/zardam/uinput-serial-keyboard
cd uinput-serial-keyboard
gcc uinput.c -o uinput
```

Create **/etc/systemd/system/nwinput.service**:
```ini
[Unit]
Description=NumWorks input device

[Service]
Type=simple
WorkingDirectory=/home/pi/uinput-serial-keyboard/
ExecStart=/home/pi/uinput-serial-keyboard/uinput
User=root
Group=root
Restart=on-failure

[Install]
WantedBy=multi-user.target
```

Enable the service:
```bash
sudo systemctl daemon-reload
sudo systemctl enable nwinput
sudo systemctl start nwinput
```

#### 5.5 Disable Serial Console

In **/boot/cmdline.txt**, remove:
```
console=serial0,115200
```

#### 5.6 Optional: fbcp for GPU Acceleration

If you want hardware acceleration (scaling, compositing), use fbcp to copy fb0 to fb1:

```bash
sudo apt-get install cmake
git clone https://github.com/Oper8or/rpi-fbcp.git
cd rpi-fbcp
mkdir build && cd build
cmake ..
make
```

Add to **/boot/config.txt**:
```ini
hdmi_force_hotplug=1
hdmi_cvt=640 480 60 1 0 0 0
hdmi_group=2
hdmi_mode=87
```

### On the NumWorks Calculator

#### 5.7 Custom Firmware (Epsilon with RPi App)

```bash
# Install NumWorks SDK first (see NumWorks documentation)

# Clone the modified firmware
git clone -b rpi https://github.com/zardam/epsilon.git
cd epsilon

# Build and flash (with calculator connected via USB)
make epsilon_flash MODEL=n0100   # For N0100
# or
make epsilon_flash MODEL=n0110   # For N0110
```

**Note:** This firmware is based on an older version of Epsilon. For newer Epsilon versions, you may need to port the RPi app code.

---

## 6. Keyboard Mapping

The calculator has only 46 keys, so modifier keys are used to access all functions:

| Calculator Key | Default | With "x,n,t" | With "var" |
|----------------|---------|--------------|------------|
| A-L | Q,B,C,D,E,F,G,H,I,J,K,L | F1-F12 | - |
| M-O | ;, N, O | 7, 8, 9 | - |
| P-Q | P, A | 5, - | - |
| R-T | R, S, T | 4, 5, 6 | - |
| Arrows | Numpad 4,8,2,6 | Mouse movement | - |
| OK | Left click | - | - |
| Back | Right click | - | - |
| Exe | Enter | = | - |
| Space | Space | - (minus) | - |

**Mouse Mode:** Press Power button to toggle mouse keys (arrows become mouse movement).

---

## 7. Physical Installation

### Step 1: Open the Calculator
1. Remove 4 rubber feet to expose T5 screws
2. Remove 6 T5 screws
3. Carefully separate back cover (N0110 has clips)
4. Disconnect battery connector

### Step 2: Prepare Mounting Points
1. The Pi Zero fits in the space above/beside the motherboard
2. Use double-sided tape on the Pi's HDMI connector (non-essential side)
3. Route wires carefully to avoid interference with keyboard flex cable

### Step 3: Solder Connections
1. **SPI Pads:** Carefully solder 30 AWG wires to PA5, PA6, PA7 pads
2. **UART Pads:** Solder to PA9 (TX) and PA10 (RX)
3. **Power:** Solder MOSFET circuit to battery terminals
4. **Ground:** Use USB port shield or dedicated ground pad

### Step 4: Test Before Closing
1. Flash custom firmware to calculator
2. Power on and verify SPI connection
3. Check keyboard input via UART
4. Verify power switching works

---

## 8. Performance Specifications

| Metric | Value |
|--------|-------|
| Display Resolution | 320 × 240 pixels |
| Color Depth | 16-bit (RGB565) |
| SPI Speed | 62.5 MHz (tested stable, spec is 50 MHz) |
| Theoretical Max FPS | ~50 fps |
| Practical FPS | 30-40 fps with fbcp scaling |
| Power Consumption | ~120mA typical (Pi Zero + display) |
| Battery Life | 3-5 hours (depends on usage) |

---

## 9. Troubleshooting

### Display Issues
- **Black screen:** Check SPI wiring, especially SCLK and MOSI
- **Corrupted display:** SPI speed too high, try reducing in driver
- **No frames:** Check CS/MISO connection (PA6)

### Keyboard Issues
- **No input:** Verify UART connections, check nwinput service
- **Wrong keys:** Check keyboard mapping in uinput.c

### Power Issues
- **Pi won't boot:** Battery voltage too low, check MOSFET circuit
- **Random crashes:** Power supply unstable, add capacitor (100µF) near Pi

### Firmware Issues
- **Can't flash:** Enter recovery mode (hold 6 + reset on N0100, or hold 6 while connecting USB)
- **App missing:** Rebuild with correct MODEL flag

---

## 10. Resources and Links

### Official Documentation
- NumWorks Schematics: https://www.numworks.com/resources/engineering/hardware/electrical/schematics/
- NumWorks PCB Layout: https://www.numworks.com/resources/engineering/hardware/electrical/pcb/
- STM32F412 Reference Manual: (linked on NumWorks site)

### GitHub Repositories
- Modified Epsilon Firmware: https://github.com/zardam/epsilon/tree/rpi
- SPI Framebuffer Driver: https://github.com/zardam/spifb
- Keyboard Daemon: https://github.com/zardam/uinput-serial-keyboard
- fbcp (framebuffer copy): https://github.com/Oper8or/rpi-fbcp

### Datasheets
- STM32F412 (N0100 MCU)
- STM32F730 (N0110 MCU)
- CYW43438 WiFi chip (for Pi Zero W voltage requirements)

---

## 11. Summary: Complete Parts List

### For N0100 + Pi Zero W Build

| Item | Qty | Notes |
|------|-----|-------|
| NumWorks N0100 | 1 | Your existing calculator |
| Raspberry Pi Zero W | 1 | Or non-W for simpler power |
| MicroSD Card 16GB+ | 1 | Class 10, good brand |
| P-Channel MOSFET (NTR1P02LT1) | 1 | Logic level, SOT-23 |
| 10kΩ Resistor | 1 | 0402 or 0603 SMD |
| 30 AWG Wire | 1 roll | Multiple colors helpful |
| Double-sided tape | - | Thin foam tape |
| T5 Torx screwdriver | 1 | For case |
| PH00 Phillips screwdriver | 1 | For motherboard |
| Soldering equipment | - | Fine tip required |

**Total additional cost:** ~$15-20 (Pi Zero W) + ~$2 (components)

---

*This guide was compiled from the original Zardam project and updated with compatibility information for modern hardware. Last updated: January 2026*
