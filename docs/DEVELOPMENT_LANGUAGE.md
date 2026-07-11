# Development language policy

The repository uses English for all developer-facing material:

- source code and identifiers,
- code comments,
- filenames and directory names,
- commit messages,
- architecture notes,
- hardware documentation,
- troubleshooting guides,
- UI examples in developer documentation.

End-user controller text may be localized. The current rule is:

- Turkish browser locale (`tr-*`) -> Turkish controller UI,
- every other locale -> English controller UI,
- physical LED announcements -> English,
- access-point SSID -> `! JOIN GAME !`.

Use canonical arcade terms in documentation and English UI examples:

- `READY`
- `START GAME`
- `BREAK`
- `CONTINUE`
- `LEAVE GAME`
- `MAIN MENU`
- `JOIN GAME`
- `WINNER`
- `DRAW`
- `GAME OVER`
- `NEW RECORD`
