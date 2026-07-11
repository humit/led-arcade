# Pinout Reference — 8x32 Matrix Arcade

Bu dosya, mevcut 8x32 LED arcade demo setup'ı için firmware ↔ donanım pin eşlemesini içerir.

## ESP32 pin map

| Function          | ESP32 Pin | Destination           | Notes |
|------------------|-----------|-----------------------|-------|
| LED Data         | GPIO23    | WS2812 Matrix DIN     | 330–340 ohm series resistor üzerinden |
| Audio Output     | GPIO22    | PAM8403 L-IN          | Mono kullanım için sol kanal |
| 5V Power         | 5V / JST+ | PSU 5V+               | Güç girişi |
| Ground           | GND / JST-| PSU 5V-               | Ortak ground |

---

## Passive components

| Component | Connection | Value | Purpose |
|----------|------------|-------|---------|
| Series resistor | GPIO23 → LED DIN | 330–340 ohm | WS2812 data hattını koruma/stabilite |
| Pulldown resistor | LED DIN → GND | 10k | Açılışta rastgele tam parlaklık problemini azaltma |

---

## LED matrix connector map

| Matrix side | Pins | Purpose |
|------------|------|---------|
| Left | DIN / GND / 5V | Data input + power |
| Middle | GND / 5V | Power injection |
| Right | DOUT / GND / 5V | Data output |

> Kullanım: veri soldaki `DIN` hattına girer.

---

## Audio map

| Amplifier pin | Source |
|--------------|--------|
| 5V | PSU 5V+ |
| GND | Common GND |
| L-IN | ESP32 GPIO22 |
| Speaker out | Speaker |

> R-IN kullanılmıyor.

