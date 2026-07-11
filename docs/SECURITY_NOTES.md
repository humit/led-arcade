# Security Notes

## Current MVP choice

The AP is open for onboarding speed:

```text
SSID: ! JOIN GAME !
Auth: none
IP: 10.10.10.10
```

This makes both QR scan and "tap WiFi network" UX easier.

## Risk

An open AP increases the chance of accidental or targeted abuse:

- fake controller requests
- request spam
- pause/mute abuse
- state endpoint scraping
- denial-of-service by repeated joins

## Required next layer

WPA is not sufficient by itself because a venue QR distributes the password. Next security tasks:

1. Server-issued session token.
2. Token validation on `/state`, `/press`, `/game_pause`, `/mute`.
3. Per-IP and per-session rate limiting.
4. Admin/host-only controls for global pause/mute/reset.
5. No public debug/admin endpoints.
6. Production build with reduced serial logging.
