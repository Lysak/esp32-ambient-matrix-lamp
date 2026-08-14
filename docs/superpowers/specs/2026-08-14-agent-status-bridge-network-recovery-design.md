# Agent Status Bridge Network Recovery

## Goal

After a Mac Wi-Fi route loss, the bridge must not remain running while unable to
reach Home Assistant. Once the router is available again, launchd must start a
fresh bridge process automatically.

## Design

- While Home Assistant is configured, the bridge sends one authenticated,
  read-only `GET /api/` health check every 30 seconds.
- `EHOSTUNREACH` and `ENETUNREACH` are route failures. A failed health check,
  status publish, or button press with either code exits the bridge with code 1.
- The existing `KeepAlive` LaunchAgent restarts that failed process. A 30-second
  `ThrottleInterval` bounds retries while the router is booting.
- HTTP responses, authentication failures, and other application errors only
  log normally: restarting cannot repair them.
- Escape the watchdog command for valid plist XML so its install remains valid
  across reloads and logins.

## Verification

- Unit-test route-error classification, including Node's nested `cause.code`.
- Unit-test the generated main LaunchAgent throttle and watchdog XML validity.
- Build and run the focused test set; reload the LaunchAgent and confirm it is
  running.
