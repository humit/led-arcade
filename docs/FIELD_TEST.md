# LED Arcade Field Test

This build is intended for short, supervised device-compatibility and stability tests. It keeps all diagnostics on the ESP32 local access point and does not upload data to an external service.

## What is recorded

- boot count and ESP32 reset reason;
- the previous boot's last checkpoint, free heap and last recorded event;
- current and minimum free heap;
- Wi-Fi station joins and disconnects, including station MAC, association ID and disconnect reason code;
- current and peak Wi-Fi station counts;
- WebSocket connects, disconnects, queue drops and heartbeat timeouts;
- controller CID, local IP and a compact browser/device profile;
- the first pointer and touch events produced by each browser session;
- accepted and rejected gestures with surface, direction/result, distance, duration and pointer type.

The in-memory event ring contains the most recent 64 events. Boot checkpoints are stored in RTC memory, so they normally survive software resets, watchdog resets, panics and brownouts. A complete power loss can clear RTC memory.

## Accessing diagnostics

While connected to `! OYUNA KATIL !`, open:

- `http://10.10.10.10/debug` for the auto-refreshing field dashboard;
- `http://10.10.10.10/debug.json` for the current machine-readable summary;
- `http://10.10.10.10/debug.log` to download the event log.

Serial output at 115200 baud contains the same events with a `[FIELD]` prefix. If the ESP32 reboots, save both the Serial output and `/debug.log` after it comes back.

## Suggested device matrix

For every phone or tablet, record a short label outside the ESP32, such as `android-a`, `iphone`, or `tablet-b`, then test:

1. Join the Wi-Fi network and open the captive page.
2. Confirm the device appears in `/debug.json` with the expected IP and browser profile.
3. Play Pixel Pong and Stack Shift in portrait and landscape.
4. Perform short, slow, diagonal and valid directional swipes.
5. Lock the screen for 35 seconds, unlock it and observe reconnect behavior.
6. Disable and re-enable Wi-Fi during a game.
7. Repeat until all devices are connected concurrently.
8. Download `/debug.log` after any missed gesture, disconnect, slowdown or reboot.

## Reset reason limitations

The reset reason distinguishes software resets, watchdogs, panics and brownouts. It does not contain a complete exception backtrace. For a panic, keep the USB Serial monitor attached when possible; the ROM panic output is required for source-level crash decoding.
