# Arcade Presentation Layer v1

This layer gives all games a shared low-resolution arcade identity without changing game rules.

## Visual system

- Palette-indexed sprites use `1`, `2`, and `3` as dim, medium, and bright shades.
- Menu attract mode uses an original chomper-like character and ghost-like follower.
- Game selection/lobby transitions include racing, ship, and paint teaser animations.
- Rendering remains compatible with the current 8x32 matrix mapping.

## Audio system

`AudioOut` is now non-blocking. Tone patterns are advanced from `audio.update()` in the main loop, so sound effects do not pause WebSocket handling or game ticks.

Included cues:

- menu selection
- game start
- countdown / go
- power-up
- crash / defeat
- restart
- winner / new record
- boss intro / boss defeated

Audio profiles are represented by `MUTE`, `QUIET`, and `NORMAL`. The first version defaults to `NORMAL`; controller configuration and persistence can be added later.

## Integration

`ArcadeDirector` watches shared game state transitions and triggers the corresponding visual and audio cues. Individual games continue to own their mechanics.
