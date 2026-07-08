# Roadmap

## Immediate

- Test current monolithic dev sketch.
- Confirm open AP captive portal behavior on iOS and Android.
- Confirm global pause/resume.
- Confirm global mute/unmute.
- Confirm center white marker LEDs are gone.
- Confirm badminton-style ball head is the only hit point.

## Next refactor

Split the monolithic sketch into:

```text
src/Config.h
src/Types.h
src/SystemState.h
src/hardware/AudioOut.h
src/hardware/LedRenderer.h
src/session/SessionManager.h
src/games/color_rally/ColorRallyGame.h
src/controller/ControllerPage.h
src/net/CaptivePortal.h
```

## Backlog

- Player-level mola / continue.
- Session token.
- Rate limiting.
- Admin/host controller.
- Simon-style color/audio memory game.

## Compile validation

2026-07-08:
- Arduino IDE compile succeeded for `firmware/arduino/color_rally/color_rally.ino`.
- Program storage: 1,359,867 bytes / 2,097,152 bytes, 64%.
- Global variables: 52,256 bytes / 327,680 bytes, 15%.
- Board target: ESP32 / LOLIN32 Lite class.
