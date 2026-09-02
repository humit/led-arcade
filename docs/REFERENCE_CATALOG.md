# Venue Arcade Research Reference Catalog

This catalog keeps external projects, runtimes, datasets, and architectural references that may be useful for the LED Arcade / venue multiplayer platform.

The catalog is research material, not an endorsement. Before reusing code, games, assets, or question banks in a commercial venue product, verify the exact current license and any upstream content rights.

## Shared TV + phone-controller party games

### Rumpus
- Repo: https://github.com/rodwilco/rumpus
- Model: shared `/host` TV display + `/play` phone clients over LAN.
- Interesting for: trivia, bluffing, prompt games, plugin architecture, local/self-hosted party-game UX.
- License noted during research: AGPL-3.0; re-verify before reuse.

### GameNight
- Repo: https://github.com/abhijatchaturvedi/gamenight
- Model: browser/LAN party games.
- Interesting for: quiz, drawing/scribble, social-deduction and card-game mechanics.

### Trivia Game (buxxi)
- Repo: https://github.com/buxxi/trivia-game
- Model: shared-screen trivia with phone answers.
- Interesting for: question + image/video/music rounds and Turkish trivia adaptation.

### Buzz TV Party Game / Couch Kit
- Repo: https://github.com/faluciano/buzz-tv-party-game
- Related toolkit: https://github.com/faluciano/react-native-couch-kit
- Model: TV host + QR phone controllers, local WebSocket/session model.
- Interesting for: buzzer/trivia UX, reconnect/session handling, offline/LAN operation.

### Joystick Jammers / multiplayer-racer
- Repo: https://github.com/cdilga/multiplayer-racer
- Model: shared-screen realtime racing with phones as controllers.
- Interesting for: low-latency realtime TV demo and spectator-readable racing.

### Amoeba Mixer
- Repo: https://github.com/inosaint/amoebas
- Model: drop-in QR multiplayer shared arena.
- Interesting for: late join / spectator-to-player conversion mechanics.

### PartyPad
- Repo: https://github.com/benmross/partypad
- Model: browser phones become virtual controllers; experimental emulator/RetroArch integration and Linux input bridge.
- Interesting for: QR controller pairing, `uinput`, AP/local-network ideas, emulator input bridging.
- Maturity note: early-phase; use as architectural/reference material before making it a production dependency.

## HUB75 / retro / homebrew

### PiMatrixOS
- Project: https://www.instructables.com/PiMatrixOS-Raspberry-Pi-HUB75-LED-Matrix-Platform-/
- Model: Raspberry Pi + HUB75 launcher/platform with matrix-native games.
- Games demonstrated include Pong, Snake, Tetris and Pac-Man-like experiences.
- Interesting for: trying existing Raspberry Pi hardware before buying a Pi 5.

### Adafruit LED Matrix Wall Arcade
- Project: https://learn.adafruit.com/led-matrix-wall-arcade
- Model: PICO-8 rendered on four 64x64 HUB75 matrices as a 128x128 wall.
- Interesting for: separating game runtime/framebuffer from HUB75 renderer.
- Reference implementation uses Raspberry Pi 5, but Pi 5 is not a platform requirement for this project.

### PICO-8
- Home: https://www.lexaloffle.com/pico-8.php
- Type: modern fantasy console, native 128x128 display and constrained retro input/palette.
- Interesting for: mature homebrew catalog, local multiplayer, stable games, pixel-perfect HUB75/TV rendering.
- Licensing: commercial export/use requires careful distinction between PICO-8 runtime rights and individual cartridge author rights. Verify before venue deployment.

### Wonderville PICO-8 arcade residency
- Reference: https://www.lexaloffle.com/bbs/?pid=171510
- Real-world precedent: bar/arcade deployment of curated 2-player PICO-8 games, historically using Raspberry Pi-class hardware.
- Interesting for: curator/residency model where game authors explicitly submit/authorize games for a venue cabinet.

## Turkish trivia / question-data candidates

### bahadiri/Millionaire
- Repo: https://github.com/bahadiri/Millionaire
- Turkish multiple-choice dataset gathered around a "Who Wants to Be a Millionaire?" crowdsourcing research project.
- Approx. 1,908 live questions plus 3,879 practice-question rows are present in the repository.
- Structured fields include four choices and correct-choice labels; this is very close to the venue trivia format.
- Repository LICENSE is MIT, but because question text originated from a television game show, upstream question/content rights still need a separate commercial-use review. Do not assume repository code/data license resolves third-party question copyright.

### Wikidata
- Data: https://www.wikidata.org/
- License: CC0 for structured data.
- Interesting for: generating our own Turkish trivia from structured facts with provenance rather than copying an authored question bank.
- Candidate categories: geography, capitals, flags, people, works, dates, sports entities, science, Turkey-specific facts.
- Recommended long-term source for commercially safer fact-derived questions, with automated validation and refresh.

### Open Trivia DB
- Home/API: https://opentdb.com/
- License: CC BY-SA 4.0 for API data.
- Interesting for: generic trivia schema/API and non-Turkish seed/reference material.
- Turkish coverage was not verified as sufficient; do not treat it as the primary Turkish question source without a language audit.

### Turkish-BQuAD
- Repo: https://github.com/TurQuest/turkish-bquad
- Turkish biology QA dataset, MIT-licensed repository.
- Not directly multiple choice, but potentially useful for generating/validating science question candidates.

### TyDi QA Turkish subset
- Repo: https://github.com/google-research-datasets/tydiqa-wana
- Large Turkish question-answer subset.
- Not directly trivia/MCQ; useful as a possible language/QA source, subject to its dataset license and source-content terms.

### Turkish MMLU
- Dataset: https://huggingface.co/datasets/alibayram/turkish_mmlu
- Large Turkish multiple-choice corpus.
- Current published terms are non-commercial (CC BY-NC family / gated conditions); therefore unsuitable as a production commercial venue question bank unless separate permission is obtained.

## Research rules

1. Prefer commercial-safe sources with explicit licenses and clear provenance.
2. Distinguish software license from embedded content/question/game rights.
3. Public GitHub availability does not imply commercial redistribution rights.
4. For trivia, prefer storing fact provenance separately from authored question wording.
5. For games, record runtime license and cartridge/game license independently.
6. Keep third-party projects replaceable; the platform value is the local join/session/controller/display system.
