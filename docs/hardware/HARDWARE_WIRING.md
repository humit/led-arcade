# LED Arcade hardware wiring

This document is the authoritative wiring reference for the current ESP32 prototype.

## Pin assignment

| Function | ESP32 GPIO | Status |
|---|---:|---|
| 8x32 matrix data | `GPIO23` | Validated |
| 1D strip data | `GPIO19` | Reserved; not yet validated together with the matrix |
| PAM8403 audio input | `GPIO22` | Validated |

The two LED data outputs are intentionally separate so a future firmware build can drive both displays without rewiring.

## Shared power

All modules use the same regulated 5 V supply and common ground:

- PSU `5V+` -> ESP32 `5V`, matrix `5V`, 1D strip `5V`, PAM8403 `5V`
- PSU `5V-` -> ESP32 `GND`, matrix `GND`, 1D strip `GND`, PAM8403 `GND`

Do not power the matrix or long strip through an ESP32 5 V pin. Feed LED power directly from the PSU with appropriately sized wiring.

## Matrix data path

```text
ESP32 GPIO23
  -> 330-340 ohm series resistor
  -> 8x32 matrix DIN

8x32 matrix DIN
  -> 10k ohm pulldown resistor
  -> GND
```

## 1D strip data path

```text
ESP32 GPIO19
  -> 330-340 ohm series resistor
  -> 1D strip DIN

1D strip DIN
  -> 10k ohm pulldown resistor
  -> GND
```

`GPIO19` is reserved for this path, but simultaneous matrix + strip operation has not yet been validated in firmware or under load.

## Audio path

```text
ESP32 GPIO22
  -> PAM8403 L-IN

PSU 5V+ -> PAM8403 5V
PSU 5V- -> PAM8403 GND
PAM8403 left speaker output -> speaker
```

The right amplifier input is currently unused.

## Related documents

- [8x32 matrix wiring](WIRING_8X32_ESP32.md)
- [ESP32 pinout](PINOUT_8X32_ESP32.md)
- [LED and audio troubleshooting](TROUBLESHOOTING_LED_AUDIO.md)
- [Wiring diagram](led-arcade-wiring.svg)
