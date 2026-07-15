# Pixel Pong — 8×32 MVP

Pixel Pong is the first new 8×32 game added after the 1D arena baseline. It uses the existing platform selector, game launcher, lobby, WebSocket controller, audio system, player slots, CPU fallback, result view, and matrix renderer.

## Rules

- Arena: horizontal 32×8 matrix.
- Two sides: left and right.
- Paddle height: 3 LEDs.
- Controller: upper half moves up; lower half moves down.
- One human automatically receives a CPU opponent in the lobby.
- Two humans play directly against each other.
- First side to 5 points wins.
- Ball speed increases after each paddle return and resets after a point.
- Paddle contact position controls the outgoing vertical direction.

## Runtime behavior

- A match requires exactly two active players after CPU assignment.
- More than two active humans disables start for Pixel Pong.
- A disconnected side forfeits; if both sides disappear, the result is a draw.
- A point creates an 850 ms pause before the next serve.
- Rematch returns to the lobby and requires players to ready again.

## Main implementation

```text
firmware/arduino/pixel_derby/src/games/pixel_pong/PixelPongGame.h
```

Integration points:

```text
src/Types.h
src/Config.h
src/controller/ArcadePage.h
src/games/pixel_derby/PixelDerbyGame.h
src/hardware/AudioOut.h
src/hardware/MatrixRenderer.h
src/net/ArcadeNetwork.h
src/presentation/ArcadeDirector.h
```

## Hardware test checklist

1. Boot the current 8×32 + 1D firmware and confirm both arenas remain selectable.
2. Enter 8×32 Arena and confirm five game cards fit in portrait and landscape.
3. Select Pixel Pong and verify its lobby teaser and selection sound.
4. Join with one phone, mark ready, and start against CPU.
5. Confirm upper/lower controller halves move only the assigned paddle.
6. Confirm wall bounce, paddle bounce, impact angle, acceleration, point sound, and score pips.
7. Confirm the match ends at 5 points and the result page shows the final score.
8. Return to Games and verify every game card remains visible.
9. Join with a second phone and verify human-vs-human play with no CPU.
10. Disconnect one phone during play and confirm rendering/network loops remain responsive and the connected side wins.
11. Re-test Reflex Rally and Power Push on GPIO22 to ensure the preserved matrix clear change did not regress arena switching.

## Build

```bash
./tools/compile_pixel_derby.sh
```

Default FQBN:

```text
esp32:esp32:lolin32-lite:PartitionScheme=no_ota,UploadSpeed=115200
```
