# Color Rally

## Core rule

Press the same color as the incoming ball.

```text
red ball   → press red
green ball → press green
blue ball  → press blue
```

## Current mechanics

- 0 players: CPU vs CPU attract mode
- 1 player: Human vs CPU
- 2 players: Human vs Human
- Extra clients: spectator queue
- Correct hit: ball returns and combo increases
- Wrong hit: combo resets
- Miss: HP decreases
- Combo 3+: faster ball
- Combo 5+: charged ball

## Ball visual

The ball is rendered like a badminton shuttle:

- bright head = real contact point
- gradient tail = visual trail only
- hit detection uses the head
- center white marker LEDs are removed
