# 8x32 LED Arcade Wiring (ESP32 + 8x32 WS2812 + PAM8403 + 5V PSU)

Bu doküman, 8x32 matrix arcade demosunun güncel fiziksel bağlantı şemasını içerir.

## Sistem bileşenleri

- ESP32 development board
- 8x32 WS2812 LED matrix
- PAM8403 amplifier
- Speaker
- 5V PSU
- 330–340 ohm series resistor on LED data line
- 10k pulldown resistor from LED DIN to GND

---

## Ana bağlantı özeti

### Güç dağıtımı

- PSU `5V+` → LED `5V`
- PSU `5V+` → Amplifier `5V`
- PSU `5V+` → ESP32 `5V / JST +`

- PSU `5V-` → LED `GND`
- PSU `5V-` → Amplifier `GND`
- PSU `5V-` → ESP32 `GND / JST -`

> Tüm sistem ortak ground kullanır.

---

## ESP32 bağlantıları

### LED data

- `GPIO23` → `330–340 ohm resistor` → LED matrix `DIN`
- LED `DIN` → `10k resistor` → `GND`

### Audio

- `GPIO22` → PAM8403 `L-IN`
- ESP32 `GND` → PAM8403 `GND`

> Şu anda yalnızca sol audio input kullanılmaktadır.

---

## LED matrix bağlantısı

8x32 panel için veri girişi:

- Sol taraftaki giriş konnektörü: `DIN / GND / 5V`

Ek güç beslemesi:

- Orta güç noktası: `5V / GND`

Çıkış:

- Sağ taraftaki konnektör: `DOUT / GND / 5V`

> Veri mutlaka `DIN` tarafına verilmelidir. `DOUT` tarafına bağlanırsa panel çalışmaz.

---

## Hoparlör bağlantısı

- PAM8403 speaker output → speaker terminals

> Kullanılan hoparlör, PAM8403’ün ilgili çıkışına bağlanır.
> Amp çıkış tarafında board üzerindeki işaretlere göre bağlantı yapılmalıdır.

---

## Mermaid şema

```mermaid
flowchart LR
    PSU[5V PSU]

    ESP[ESP32]
    AMP[PAM8403 Amp]
    SPK[Speaker]
    LED[8x32 WS2812 Matrix]

    PSU -->|5V+| ESP
    PSU -->|GND| ESP

    PSU -->|5V+| AMP
    PSU -->|GND| AMP

    PSU -->|5V+| LED
    PSU -->|GND| LED

    ESP -->|GPIO22| AMP
    AMP --> SPK

    ESP -->|GPIO23| R1[330-340 ohm]
    R1 --> DIN[LED DIN]
    DIN --> R2[10k pulldown]
    R2 --> GND1[GND]

