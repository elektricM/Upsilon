# UART Fix for RPI Keyboard Input

## Problem

The calculator's RPI app sends keyboard state over UART (`Ion::Console::writeLine()`), but no data arrives at the Pi's `/dev/ttyS0`. The keyboard daemon (`uinput-serial-keyboard`) receives nothing.

## Root Cause

Two issues in the firmware build:

### 1. Console UART code not linked

The build system uses a "flavor" mechanism for conditional compilation. `console_uart.cpp` is only compiled when the `consoleuart` flavor is active. The normal epsilon build has **no flavor**, so `console_dummy.cpp` is linked instead — where `writeChar()` is an empty function that discards all output.

**File**: `ion/src/device/shared/drivers/Makefile`
```
console_uart.cpp:+consoleuart    ← only compiled if consoleuart flavor active
console_dummy.cpp:-consoleuart   ← compiled when consoleuart is NOT active (default)
```

Only the bench builds (`bench.ram`, `bench.flash`) had `consoleuart` enabled.

**Fix in `build/targets.mak`** (line 14):
```makefile
# Before:
$(BUILD_DIR)/epsilon.$(EXE): $(call flavored_object_for,$(epsilon_src))

# After:
ifeq ($(ENABLE_RPI),1)
$(BUILD_DIR)/epsilon.$(EXE): $(call flavored_object_for,$(epsilon_src),consoleuart)
else
$(BUILD_DIR)/epsilon.$(EXE): $(call flavored_object_for,$(epsilon_src))
endif
```

### 2. USART3 peripheral clock not enabled

Even with the UART code linked, USART3 has no clock. The N0100 `initClocks()` enables SPI1 (for the display) when `ENABLE_RPI` is set, but never enables USART3. Without a clock, the UART peripheral is dead.

USART3 is on the APB1 bus. The existing APB1 clock setup only enables TIM3, PWR, and RTCAPB.

**Fix in `ion/src/device/n0100/drivers/board.cpp`** (after line 124):
```cpp
  // APB1 bus
  RCC.APB1ENR()->setTIM3EN(true);
  RCC.APB1ENR()->setPWREN(true);
  RCC.APB1ENR()->setRTCAPB(true);
#ifdef ENABLE_RPI
  RCC.APB1ENR()->setUSART3EN(true);
#endif
```

## Build Command

```bash
make MODEL=n0100 ENABLE_RPI=1 OMEGA_USERNAME="Martin" EPSILON_I18N=en -j$(nproc)
```

## Flash Command

```bash
make MODEL=n0100 ENABLE_RPI=1 OMEGA_USERNAME="Martin" EPSILON_I18N=en epsilon_flash
```

## Verification

After flashing, enter the RPI app and press keys. On the Pi:
```bash
# Stop the daemon temporarily to read raw serial data
sudo systemctl stop nwinput
sudo cat /dev/ttyS0
# Should see lines like :0000000000000001 when keys are pressed
# Restart daemon
sudo systemctl start nwinput
```

## UART Details (N0100)

| Parameter | Value |
|-----------|-------|
| Peripheral | USART3 |
| TX pin | PD8 (AF7) |
| RX pin | PC11 (AF7) |
| Baud rate | 115200 (USARTDIV=417, APB1=48MHz) |
| Format | 8N1 |
| Protocol | `:%16llx\r\n` (colon + 16 hex digits + CR+LF) |
| Pi serial device | `/dev/ttyS0` |
| Pi GPIO | GPIO 15 (RXD), pin 10 on header |

## Status

Changes applied, not yet built or tested. Both fixes are gated behind `#ifdef ENABLE_RPI` / `ifeq ($(ENABLE_RPI),1)` so normal builds are unaffected.
