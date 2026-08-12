# Stack Shift — Vertical 8×32 MVP

Stack Shift is a solo falling-block game designed for the 8×32 matrix in portrait orientation. The game logic uses a virtual 8-column × 32-row board and the renderer rotates that virtual board onto the physical 32×8 wiring layout.

## Rules

- Arena: portrait 8×32 virtual playfield.
- Pieces: standard seven tetromino shapes using a shuffled seven-piece bag.
- Tap: rotate clockwise.
- Swipe left/right: move one column.
- Swipe down: hard drop and lock immediately.
- Hold or slow downward drag: soft drop.
- Full rows clear with dedicated single, double, triple, and four-line animations and sound motifs.
- The locked board, active tetromino, and currently earned landing ghost are the only normal playfield lights. Stack Shift explicitly clears all 256 pixels each frame and disables FastLED temporal dithering so unused cells stay fully dark.
- A conditional landing ghost is available for the opening pieces and can be earned again through four-line clears and line-clear streaks.
- Every five cleared lines increases the level and fall speed, then pauses briefly for an empty-row bonus screen.
- The game ends when a new piece cannot enter the playfield.
- Score, lines, level, and device best score are shown on the phone result view.

## Orientation

The game uses virtual coordinates independent of the matrix wiring:

```text
virtual width  = 8
virtual height = 32
```

The default clockwise mapping is:

```text
physicalX = virtualY
physicalY = 7 - virtualX
```

Set `STACK_ROTATE_CLOCKWISE` in `Config.h` to `false` if the physical installation is rotated in the opposite direction.

## Scoring and speed

- Soft drop: 1 point per row.
- Hard drop: 2 points per row.
- One line: 100 × level.
- Two lines: 300 × level.
- Three lines: 500 × level.
- Four lines: 800 × level.
- Initial fall interval: 650 ms.
- Minimum fall interval: 110 ms.
- Speed increase: 55 ms per level.
- Level transition bonus: top empty rows × 25 × new level.
- Opening ghost allowance: 10 locked pieces.
- Four-line ghost reward: 8 pieces.
- Every third consecutive line-clear reward: 5 pieces.
- Maximum stored ghost allowance: 24 pieces.

## Presentation and controller behavior

- Portrait presentation avoids long rotated words. Countdown uses large upright digits; GO, level-up, game-over, and record states use reusable motion/icons, while detailed text remains on the phone.
- The next tetromino is centered as a large low-opacity phone background with a faint `NEXT` / `SONRAKİ` label.
- The first-run gesture tutorial disappears after tap, horizontal swipe, and downward swipe are demonstrated; it can be reopened with the help button.
- During Stack Shift gameplay, root overscroll, touch panning, and Android pull-to-refresh are suppressed while Pointer Events continue to drive game input.

## Conditional ghost piece

The landing projection is intentionally not permanent. It is active for a limited number of locked pieces at the start of a game. A four-line clear and every third consecutive line-clearing placement add more ghost-enabled pieces, capped by `STACK_GHOST_MAX_PIECES`. A placement that clears no line resets the streak.

## Level break

When a line clear crosses a level boundary, play pauses for `STACK_LEVEL_BREAK_MS` (2.8 seconds). After the cleared rows collapse, the game counts empty rows from the top of the board and awards:

```text
bonus = top empty rows × STACK_LEVEL_EMPTY_ROW_POINTS × new level
```

The venue display shows an upward level animation, a large upright level number, and a green bonus-height meter. The phone simultaneously shows the exact level, empty-row count, and bonus value before the next piece spawns.

## Main implementation

```text
firmware/arduino/led_arcade/src/games/stack_shift/StackShiftGame.h
```

Integration points:

```text
src/Types.h
src/Config.h
src/controller/ArcadePage.h
src/core/ArcadeGameEngine.h
src/hardware/AudioOut.h
src/hardware/MatrixRenderer.h
src/net/ArcadeNetwork.h
src/presentation/ArcadeDirector.h
```

## Hardware test checklist

1. Select 8×32 Arena and verify six cards fit without scrolling.
2. Select Stack Shift and confirm no automatic CPU is added.
3. Mark one player ready and verify the countdown starts automatically.
4. Physically rotate the matrix and confirm the board is upright.
5. Tap repeatedly and verify clockwise rotation with wall kicks near both sides.
6. Swipe left/right and verify one-column movement without crossing walls.
7. Swipe down quickly and verify immediate hard drop.
8. Hold and slowly drag downward; verify controlled soft drop without accidental hard drop.
9. Fill a row and verify it clears, scores, and plays the line-clear sound.
10. Clear five lines and verify the orientation-aware level break, top-empty-row bonus, and faster gravity.
11. Stack blocks to the top and verify game-over, score, lines, level, and best score.
12. Select rematch and verify a clean board while preserving the device best score.
13. Verify the opening ghost expires by piece count, a four-line clear restores it, and a three-clear streak restores it.
14. Verify unused matrix cells are completely dark during normal Stack Shift play.
15. On Android, swipe down from near the top of the controller and verify the page does not reload.
16. Return to Games and re-test Pixel Pong drag, Tron swipe, Raider swipe, and Color Clash swipe.
