# Agent Finished Lamp Signal — Design

Date: 2026-07-19

## Goal

When an individual agent session (Claude Code or Codex) transitions from
working to finished, blink the ambient matrix lamp green for ~5 seconds, then
restore whatever it was showing before. Maximally simple MVP; richer status
effects are explicitly out of scope.

## Trigger semantics

- "Finished" = a single source session whose normalized status goes
  `thinking` -> `idle`.
- Per-session, not aggregated: fires even if other sessions are still working.
- `needs_user` and `error` do not trigger a flash in the MVP. Only the clean
  `thinking -> idle` edge counts.
- The existing aggregated `global_status` -> `input_select.agent_status` path
  is unchanged and independent of this feature.

## Data flow

```
Claude/Codex session: thinking -> idle
  -> bridge detects the per-session edge on its next poll
  -> POST /api/events/agent_session_finished  { family, sessionId, label }
  -> Home Assistant automation (trigger: event agent_session_finished)
  -> button.press on the ESPHome "Agent Finished Flash" button
  -> firmware blinks green ~5s, then restores the previous effect/power
```

## Components

### 1. Bridge — per-session finished detection

Repo: `tools/agent-status-bridge`.

- New pure module `src/runtime/detectFinishedSessions.ts`.
  - Input: previous status map `Map<sessionId, NormalizedStatus>` and the
    current `SourceSessionSnapshot[]`.
  - Output: the snapshots that transitioned `thinking` -> `idle`.
  - Pure and deterministic; fully unit-tested with vitest.
- `src/runtime/familyStatusLoop.ts`: add an optional
  `onSnapshots(snapshots: SourceSessionSnapshot[])` callback invoked every
  poll with the raw snapshots. The existing aggregated `onStatus` behavior is
  untouched. This avoids a second poll of the same source.
- `src/publishers/homeAssistantPublisher.ts`: add a transport helper
  `fireHomeAssistantEvent(baseUrl, token, eventType, data)` that does
  `POST {baseUrl}/api/events/{eventType}` with the JSON body. Mirrors the
  existing `setInputSelectOption` thin-wrapper style (not unit-tested
  directly; exercised against a real HA instance).
- `src/main.ts`: for each family loop, keep a
  `Map<sessionId, NormalizedStatus>` of the last seen per-session status.
  On each `onSnapshots`, run `detectFinishedSessions`, fire an
  `agent_session_finished` event per finished session (only when HA is
  configured), then update the map. Log each fired event to the console.

Event payload: `{ family, sessionId, label }`. `family` is `"claude"` or
`"codex"`; `sessionId` is `sourceSessionId`; `label` is the session label.

### 2. Firmware — HA-triggerable green blink

Repo: `esphome/common/led_matrix.yaml`.

- New `button` (platform: template), NOT `internal`, so Home Assistant sees
  it. Name: `"Agent Finished Flash"` ->
  `button.ambient_matrix_lamp_agent_finished_flash` in HA (confirm exact id
  after flashing; the HA automation references whatever ESPHome exposes).
- New globals to remember pre-flash state:
  - `notify_prev_effect` (std::string) — effect name to restore.
  - `notify_prev_powered` (bool) — whether the lamp was on.
- `on_press` action sequence (all firmware-owned, so restore is reliable):
  1. Save `id(current_effect)` and `id(lamp_powered)` into the globals.
  2. Force `lamp_powered = true` for the duration of the blink.
  3. Blink 3 times: `light.turn_on led_matrix` with solid green
     (`red: 0, green: 1.0, blue: 0`, no effect / effect "None") and a fixed
     brightness; `delay 350ms`; `light.turn_off led_matrix`; `delay 250ms`.
  4. Restore: if `notify_prev_powered` was true, set `lamp_powered = true`,
     `current_effect = notify_prev_effect`, and `light.turn_on` with that
     effect; otherwise set `lamp_powered = false` and `light.turn_off`.
- Blink count (3) and delays are constants in the yaml. Total on/off cycle is
  ~1.8s of visible activity; acceptable for the "~5s" MVP target and tunable
  later. If a strict 5s window is wanted, blink count can be raised.
- Build via repository `make` targets only (compile first, then flash),
  per project rules.

### 3. Home Assistant repo — automation

Repo: `/Users/Files/www/pet/home-assistant` (separate git repo, committed
separately).

- Add to `automations.yaml`:
  - Trigger: `platform: event`, `event_type: agent_session_finished`.
  - Action: `service: button.press`, target the ESPHome flash button entity.
  - `mode: queued` (or `parallel`) so rapid consecutive finishes are not
    dropped.
- No new `input_select`/`input_button` entities required; the existing
  `input_select.agent_status` block stays as-is.

## Testing

- Bridge: vitest unit tests for `detectFinishedSessions` covering
  `thinking -> idle` (fires), `thinking -> thinking` (no fire),
  `idle -> idle` (no fire), new session appearing idle (no fire), and a
  session disappearing (no fire in MVP).
- Firmware: `make` compile check; manual flash and visual confirmation of the
  green blink and correct restore of the prior effect and power state.
- HA: manually fire `agent_session_finished` from Developer Tools > Events and
  confirm the lamp blinks; then end-to-end with a real Claude session.

## Out of scope (YAGNI)

- Pulsing / animated status effects beyond a simple blink.
- Per-status colors, blending, or a dedicated status effect class.
- `needs_user` / `error` flashes.
- Disappeared-session handling as a "finished" signal.
- Any change to the aggregated `global_status` path.
