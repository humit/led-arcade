# Layered Refactor

This refactor moves the working monolithic Color Rally sketch into explicit layers:

```text
src/
  Config.h
  Types.h
  SystemState.h
  hardware/
    AudioOut.h
    LedRenderer.h
  session/
    SessionManager.h
  games/color_rally/
    ColorRallyGame.h
  controller/
    ControllerPage.h
  net/
    CaptivePortal.h
```

## Behavior preserved / changed

Preserved:
- Color Rally core gameplay
- CPU vs CPU / Human vs CPU / Human vs Human
- spectator queue
- reconnect via `cid` + AP client IP
- badminton-style ball head/tail
- HP and combo

Changed:
- AP is open: `! JOIN GAME !`
- QR is `WIFI:T:nopass;S:! JOIN GAME !;;`
- middle white marker LEDs removed
- global pause and mute controls added to controller UI

## Validation

Run:

```bash
cd /Users/uezerce/src/led-arcade-platform
./tools/compile_color_rally.sh
```
