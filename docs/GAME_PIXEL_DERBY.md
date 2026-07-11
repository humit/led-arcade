# Pixel Derby MVP

- Display: 32×8 WS2812B, serpentine by columns.
- Mapping: index 0 top-left, index 1 below it; odd columns reverse vertically.
- Players: 2–8, one row per player.
- Input: full-screen phone tap over WebSocket.
- Rule: each accepted tap advances one column; first to x=31 wins.
- Server debounce: 65 ms.
- Match: first to two round wins.
- UI flow: platform select → game select → lobby → countdown → race → result.

## v2 arcade feedback layer

The first enhancement pass adds:

- balanced display rows while fewer than eight players are connected,
- a persistent controller score panel,
- session points, wins, streaks and personal best time,
- a larger glowing leader while other racers remain single-pixel,
- an expanding winner animation,
- rainbow result rendering for a new device record.

Scoring in this pass is deliberately simple: every connected racer receives 20 participation points and the round winner receives an additional 80 points. Power-ups and cooperative boss rounds are planned as separate game states after this visual/statistics baseline is validated on hardware.

## v3: Session controls, Turbo, and co-op boss

- `BREAK`: player remains connected and keeps score/slot, but is excluded from active player count, ready checks, race lanes, and boss scaling.
- `LEAVE GAME`: immediately releases the slot. The same browser can join again through `JOIN GAME`.
- Turbo gates are shown at columns 10 and 21. Crossing a gate grants three 2x movement taps and 10 points.
- After every two races, `CONTINUE` starts a 15-second cooperative boss round.
- Boss HP scales with active players. Every tap deals damage; Turbo taps deal 2 damage.
- Defeating the boss awards each active player 50 team points plus their personal damage contribution.

## King Boss (v4)

After every two races, the most successful active player becomes the player-controlled King Boss.
Boss candidacy uses recent race placement, personal records, and collected turbo gates. The previous
boss receives a small rotation penalty so the role does not remain locked to one player.

- Raiders tap to damage the boss.
- The boss taps to charge a pulse attack.
- Every 12 boss taps triggers a short 900 ms stun against all raiders.
- The battle lasts 18 seconds.
- Boss HP is `25 + 22 * raider_count`.
- Raiders win by reducing HP to zero; otherwise the boss wins by surviving.
- Raider victory awards team points, a top-damage bonus, and a last-hit bonus.
- Boss victory awards 100 points.
