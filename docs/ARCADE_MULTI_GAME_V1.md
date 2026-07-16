# 8x32 Arcade multi-game v1

## Frozen baseline

`Pixel Derby + King Boss` is the playable baseline. Treat it as frozen except for shared-platform fixes.
Suggested tag: `8x32-pixel-derby-v1-playable`.

## Shared UX decisions

- AP SSID: `! OYUNA KATIL !`
- Controller IP: `10.10.10.10`
- Friendly captive-DNS names: `http://play` and `http://oyna`
- `Mola` replaces the old waiting terminology.
- Each player's controller uses shades of the assigned player color.
- During active play the phone is a minimal full-screen controller, without score cards or session buttons.
- Instructions live in the pre-game lobby.
- TR/EN controller language defaults to browser language and can be overridden.
- Venue LED language is selected with `DISPLAY_LANGUAGE_TR` in `Config.h`.
- Shared matrix cues: countdown, BOSS/color intro, WINNER/color result.

## Games

### Pixel Derby

Playable and retained as the first game. Includes Turbo gates, scoring, records, balanced lane placement and player-controlled King Boss rounds.

### Tron Arena beta

- 2-4 players
- full-screen left/right controller
- constant movement
- trail, wall and simultaneous-head collision
- speed increases after 10 and 20 seconds
- last surviving player wins; simultaneous elimination is a draw

## Next polish

- Store venue language and game settings in NVS.
- Add LED attract-mode animation and animated game-card previews.
- Add Tron Ghost and Shield power-ups only after movement/collision testing.


## Automatic CPU opponent

- In the lobby, one active human automatically gets one CPU opponent.
- When a second active human is present, the CPU is removed before the next round.
- Pixel Derby CPU uses irregular tap timing.
- Tron CPU chooses between forward/left/right based on available trail clearance.
- Players joining during an active round participate from the next lobby to avoid changing the arena mid-round.


## Compact captive portal menu (v10)

- Platform/game selection is a single-viewport arcade menu, not a scrolling web page.
- Game cards are compact 8-bit tiles arranged in two columns.
- Scrolling or swiping over a preview never starts a game. Only the explicit PLAY/OYNA button selects it.
- Debug/status elements are hidden on menu screens; active gameplay and lobby retain only necessary controls.
- As the catalog grows, pagination/category pages should replace a long scrolling list.
