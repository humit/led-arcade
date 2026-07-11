# LED and audio troubleshooting

## Audio works but the LEDs remain dark

Check these items in order:

1. Confirm the physical data wire matches the firmware pin.
   - Matrix: `GPIO23`
   - 1D strip: `GPIO19`
2. Confirm the data wire reaches `DIN`, not `DOUT`.
3. Measure the complete data path with power removed.
   - GPIO to display `DIN` should read approximately the series-resistor value.
4. Verify the `10k ohm` pulldown from `DIN` to `GND`.
5. Measure approximately 5 V directly at the display power pads while powered.
6. Confirm all grounds are common.

### Recorded prototype failure

The matrix was once connected physically to `GPIO19` while the matrix firmware still transmitted on `GPIO23`. The ESP32, network, and audio all worked, but the matrix remained completely dark. Moving the matrix data wire back to `GPIO23` fixed the problem.

This symptom should always trigger a firmware-to-wiring pin check first.

## Random LEDs or full brightness at power-on

- Verify the `10k ohm` `DIN`-to-`GND` pulldown.
- Verify common ground before applying LED power.
- Verify the ESP32 and display receive stable 5 V.

## No audio

- Confirm `GPIO22` reaches PAM8403 `L-IN`.
- Confirm amplifier `5V` and `GND`.
- Confirm the speaker is connected to an amplifier output, not an input.
- Confirm firmware is not using the `MUTE` audio profile.

## Continuity testing notes

A continuity buzzer may not beep through a `330-340 ohm` series resistor. Use resistance mode and expect a value close to the resistor rating.
