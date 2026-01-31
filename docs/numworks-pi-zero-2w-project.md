# NumWorks N0100 + Raspberry Pi Zero 2 W Integration Project

## Project Overview

Modification of the NumWorks N0100 calculator case to directly house a Raspberry Pi Zero 2 W, replacing or supplementing the original calculator functionality with a Linux-capable system.

### Goals
- Integrate Pi Zero 2 W directly into the N0100 case (not as an external module)
- Maximize battery capacity within original case dimensions
- Maintain clean form factor matching original calculator aesthetics

---

## Hardware Specifications

### NumWorks N0100 Calculator
| Specification | Value |
|--------------|-------|
| Total Dimensions | 160mm × 82mm × 10mm (6.3" × 3.2" × 0.4") |
| Internal Height | ~6-7mm (after PCB ~1.6mm + case walls) |
| MCU | STM32F730V8T6 |
| Display | 320×240 LCD |

### Raspberry Pi Zero 2 W
| Specification | Value |
|--------------|-------|
| Dimensions | 65mm × 30mm × ~5mm |
| SoC | BCM2710A1 (quad-core Cortex-A53 @ 1GHz) |
| RAM | 512MB |
| Power | 5V via micro-USB or GPIO |
| Typical Current Draw | 100-350mA (idle to load) |

---

## Original Battery Specifications

| Specification | Value |
|--------------|-------|
| Model | 603759 (Tenergy Li-Polymer) |
| Dimensions | 61.5mm × 38mm × 6.2mm (L × W × T) |
| Capacity | 1450mAh @ 3.7V |
| Weight | ~30g |
| Datasheet | https://www.tenergy.com/30142-0 |

---

## Battery Upgrade Options

### Constraint: 6mm Maximum Thickness

To maintain case closure, battery thickness must stay ≤6mm.

### Available Batteries at 6mm Thickness

| Model | Dimensions (T×W×L) | Capacity | Capacity vs Original | Availability |
|-------|-------------------|----------|---------------------|--------------|
| 603759 | 6 × 38 × 59mm | 1450mAh | 1.00× (baseline) | Original |
| 604060 | 6 × 40 × 60mm | ~1800mAh | 1.24× | Common |
| 605080 | 6 × 50 × 80mm | ~3000mAh | 2.07× | Available |
| **606090** | **6 × 60 × 90mm** | **4000mAh** | **2.76×** | **Very common** |
| 6060100 | 6 × 60 × 100mm | ~4500mAh | 3.10× | Available |

### Recommended: 606090 Battery

**Best balance of capacity, availability, and fit.**

| Specification | Value |
|--------------|-------|
| Model | 606090 |
| Dimensions | 6mm × 60mm × 90mm (92mm with protection circuit) |
| Capacity | 4000mAh @ 3.7V |
| Energy | 14.8Wh |
| Weight | 70-84g |
| Price | €5-10 |
| Certifications | CE, UN38.3, MSDS, UL available |

**Sourcing:**
- Amazon: Search "606090 3.7V 4000mAh"
- AliExpress: Multiple vendors
- RobotShop, Adafruit (quality-verified options)

### LiPo Battery Naming Convention

Battery model numbers encode dimensions:
```
TTWWLL
│││││└── Length in mm (last 2 digits)
││││└─── Width in mm (middle 2 digits)  
│└┴──── Thickness in 0.1mm (first 2 digits)

Example: 606090 = 6.0mm thick × 60mm wide × 90mm long
```

### Alternative: 5mm Thickness

If more clearance is needed:
- Search pattern: `50XXXX` batteries
- Slightly lower capacity but easier fit
- Example: 505080 = 5.0mm × 50mm × 80mm ≈ 2500mAh

---

## Power Management

### Zardam's Original Design (Reference)

The original external implementation by Zardam uses asymmetric power control:

#### Power ON
- STM32 GPIO drives P-channel MOSFET gate LOW
- Enables 3.3V rail to Pi
- Triggered when "external" app launches on NumWorks

#### Power OFF (Manual by Design)
- Pi stays powered until explicit shutdown
- User must run `sudo shutdown -h now`
- STM32 detects Pi halt via GPIO or timeout
- Then safely cuts power via MOSFET

#### Why No Auto-Shutdown on App Exit?
- App exit returns to NumWorks menu, not necessarily end of Pi session
- User might accidentally press Back
- Abrupt power cut would corrupt SD card

### Safe Shutdown Strategies

1. **Shutdown script** - Button combo or menu triggers `shutdown -h now`
2. **GPIO handshake** - Pi signals "ready to die", STM32 then cuts power
3. **Idle timeout** - Auto-shutdown after X minutes of inactivity

---

## Physical Layout Considerations

### Available Internal Space
```
Case Interior: ~160mm × 82mm × 6-7mm usable height

┌─────────────────────────────────────┐
│         NumWorks Case (top view)    │
│  160mm                              │
│ ┌─────────────────────────────────┐ │
│ │                                 │ │
│ │   ┌──────────┐                  │ │ 82mm
│ │   │ Pi Zero  │   [Battery Zone] │ │
│ │   │ 65×30mm  │                  │ │
│ │   └──────────┘                  │ │
│ │                                 │ │
│ └─────────────────────────────────┘ │
└─────────────────────────────────────┘
```

### Fit Analysis for 606090 Battery

- Battery footprint: 60mm × 90mm (92mm with protection circuit)
- Pi Zero 2 W footprint: 65mm × 30mm
- Case interior: ~160mm × 82mm

**Assessment:** Both components should fit side-by-side or with Pi overlapping battery slightly (using standoffs).

---

## Resources

### 3D Models
- **NumWorks Official CAD**: https://github.com/numworks/dieter (STL files)
- **Zardam's External Module**: https://zardam.github.io/post/numworks-music-player/

### Documentation
- NumWorks N0100 teardown/specs
- Pi Zero 2 W mechanical drawings: https://www.raspberrypi.com/documentation/computers/raspberry-pi.html

### Battery Sourcing
- Tenergy 603759 (original): https://www.tenergy.com/30142-0
- LiPo catalogs: batterylipo.com, lipolbattery.com, polybattery.com

### Related Projects
- Zardam's NumWorks Linux: https://zardam.github.io/post/numworks-music-player/
- Zardam's GitHub: https://github.com/zardam

---

## TODO / Next Steps

- [ ] Measure exact available space after Pi Zero 2 W placement
- [ ] Determine optimal battery position in modified case
- [ ] Verify case modification tolerances for 60mm × 90mm battery
- [ ] Design mounting solution for Pi Zero 2 W
- [ ] Plan display connection (reuse NumWorks LCD vs external)
- [ ] Design power management circuit (boost converter, charging, protection)
- [ ] Consider 5mm thickness batteries if clearance issues arise
- [ ] Create modified 3D model of case bottom
- [ ] Plan GPIO breakout / button mapping

---

## Estimated Runtime

With 606090 battery (4000mAh @ 3.7V = 14.8Wh):

| Pi Zero 2 W State | Current Draw | Runtime Estimate |
|-------------------|--------------|------------------|
| Idle | ~100mA | ~30+ hours |
| Light load | ~200mA | ~15 hours |
| Heavy load | ~350mA | ~8-9 hours |

*Note: Actual runtime depends on voltage conversion efficiency (~85-90% typical) and display power consumption.*

---

## Version History

| Date | Change |
|------|--------|
| 2026-01-30 | Initial research: battery specs, upgrade options, power management |
