# LED Arcade firmware workflow

## Canonical commands

```bash
./tools/arcade doctor
./tools/arcade compile --clean
./tools/arcade deploy --port /dev/cu.usbserial-XXXX
./tools/arcade monitor --port /dev/cu.usbserial-XXXX
```

`make -f Makefile.arcade compile`, `deploy`, `monitor`, and `doctor` are aliases.

## Environment overrides

```bash
PORT=/dev/cu.usbserial-XXXX ./tools/arcade deploy
WS_TCP_TIMEOUT_MS=300 ./tools/arcade compile --clean
FQBN='esp32:esp32:lolin32-lite:PartitionScheme=no_ota,UploadSpeed=115200' \
  ./tools/arcade compile
```

The canonical compiler injects `WEBSOCKETS_TCP_TIMEOUT` into the separate
arduinoWebSockets library compilation unit. Defining this macro only in an
application header is insufficient because Arduino libraries are compiled
separately.

## Wi-Fi drop regression test

Run each scenario in both a 1D game and an 8×32 game:

1. Start with one phone and enter active gameplay.
2. Disable Wi-Fi on the phone without closing the controller page.
3. Verify that LED animation does not stop for several seconds.
4. Reconnect the same phone and verify that its CID restores the same player slot.
5. Repeat with two phones, disconnecting only one.
6. Verify that the remaining controller stays responsive.
7. Repeat the disconnect/reconnect cycle ten times.
8. Leave the device in attract mode for at least five minutes and verify that
   no stale player remains active.

Expected transport behavior:

- An individual stale synchronous WebSocket operation is bounded by the compile-time timeout.
- Protocol heartbeat removes clients that disappear without a clean close frame.
- Any AP station departure recycles WebSocket transports without deleting player sessions;
  remaining phones reconnect with their stored CID.
- AP station events remain a fast-path signal; they are not polled every render frame.
