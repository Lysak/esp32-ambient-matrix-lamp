# Mac Agent Status Bridge Design

Date:

- `2026-07-16`

## Goal

Build a Mac-side bridge that observes `Codex` and `Claude Code`, computes normalized status for each tool family, and publishes those statuses into `Home Assistant` for lamp rendering.

This bridge is the state source.

The lamp is not a tracker.
The lamp is a renderer.

## Scope

This design covers:

- Mac-side runtime and process model
- collector boundaries for `Codex` and `Claude Code`
- normalized status contract
- aggregation rules
- publishing boundary to `Home Assistant`
- repository layout for bridge code and docs

This design does not yet cover:

- final Home Assistant entity naming
- final ESPHome implementation for status rendering
- launchd plist details

## Decisions

### Runtime

Use:

- `Node.js 24`
- `TypeScript 7`
- `pnpm`
- `tsx`
- `vitest`
- `biome`

Do not use:

- `pm2`
- `AppleScript` as the main implementation
- direct SQLite reads as the primary `Codex` integration path

Rationale:

- `Node.js + TypeScript` is a good fit for long-running local process work, JSON-based protocols, Home Assistant HTTP/WebSocket APIs, and clean separation between collectors, aggregation, and publishing.
- `tsx` keeps local iteration simple.
- `vitest` is a good fit for deterministic unit tests around status normalization and aggregation.
- `biome` is enough for formatting and linting without adding `eslint + prettier`.
- `pm2` is unnecessary on macOS for this local-only service. Native `launchd` is the intended long-running host later.

### Repository layout

Bridge code lives in:

- `tools/agent-status-bridge/`

Related docs live in:

- `docs/agent-status-lamp/`

### Correct integration path

For `Codex`, prefer the official live runtime surface:

- `Codex App Server`
- `Codex remote-control` / SDK-facing thread APIs and status notifications

Do not treat the local `~/.codex/*.sqlite` state as the primary integration contract.

For `Claude Code`, prefer the official machine-readable CLI surface:

- `claude agents --json --all`

This means the bridge uses official state surfaces first, not TUI parsing and not local database scraping.

## Architecture

The bridge should be split into small, isolated units.

```text
collectors -> normalizers -> aggregators -> publishers
```

### Collectors

Each collector has one job: gather raw state from one source.

Planned collectors:

- `codexCollector`
- `claudeCollector`

`codexCollector`:

- connects to the official `Codex` runtime surface
- watches thread/session state
- emits raw source events or snapshots

`claudeCollector`:

- runs `claude agents --json --all`
- converts the output into a raw source snapshot
- for MVP uses polling rather than UI automation

### Normalizers

Each normalizer maps tool-specific states to the bridge-wide state model.

Common output shape:

```ts
type AgentFamily = "codex" | "claude";

type NormalizedStatus =
  | "idle"
  | "thinking"
  | "needs_user"
  | "done"
  | "error";
```

This layer hides source-specific wording and data shape from the rest of the system.

### Aggregators

Aggregators compute:

- one `Codex` family state
- one `Claude Code` family state
- one optional global state

Priority order:

```text
error > needs_user > thinking > done > idle
```

This is intentionally strict.

If any tracked session in a family requires user action, that family should show `needs_user` even if other sessions are still thinking.

### Publishers

Publishers convert normalized states into outbound transport.

MVP publisher:

- `homeAssistantPublisher`

Its job is only to publish state.
It does not decide status priority and does not know how the lamp renders it.

## Process model

The bridge should run as one long-lived local process on the Mac.

It should not be a short shell script triggered manually each time.

MVP shape:

- one Node.js process
- one internal polling/event loop
- in-memory current state cache
- structured log output

Recommended timing:

- `Codex`: event-driven if the chosen official surface supports it cleanly
- `Claude Code`: polling every `2s`
- publisher debounce: about `300-500ms`
- `done` visibility window: hold for `10-20s` before falling back to `idle` if no further activity appears

## State contract

The bridge should publish at least:

- `codex_status`
- `claude_status`

Optional later:

- `global_status`

Initial normalized meanings:

- `idle` — no active tracked session
- `thinking` — one or more tracked sessions are actively working
- `needs_user` — a tracked session is waiting for user input or approval
- `done` — all tracked sessions in the family have completed
- `error` — a tracked session failed or requires intervention

## Home Assistant boundary

The bridge should publish normalized state only.

Home Assistant should remain responsible for:

- entity exposure
- automations
- lamp behavior routing

The bridge should not know:

- lamp preset names
- ESPHome internals
- LED effect implementation

That keeps coupling low.

## Extension strategy

This must support growth without rewriting the core.

Planned future extensions:

- richer `Codex` session grouping
- richer `Claude Code` session grouping
- subagent-aware aggregation
- separate `global_status`
- status metadata such as active count or top-priority session label
- alternative publishers such as MQTT

The rule is:

- extend by adding a collector, normalizer, or publisher
- do not rewrite the core aggregation contract unless the contract itself is wrong

## Implementation phases

### Phase 1

Docs and scaffolding:

- create `tools/agent-status-bridge/`
- create package/runtime/tooling files
- define types and interfaces
- add unit tests for aggregation rules

### Phase 2

Collector MVP:

- implement `claudeCollector` using `claude agents --json --all`
- implement `codexCollector` using the official `Codex` runtime surface

### Phase 3

Publisher MVP:

- publish `codex_status` and `claude_status` into `Home Assistant`
- test state transitions manually

### Phase 4

Lamp integration:

- connect Home Assistant state to lamp rendering

## Initial filesystem target

Planned code shape:

```text
tools/agent-status-bridge/
  package.json
  tsconfig.json
  biome.json
  vitest.config.ts
  src/
    main.ts
    config/
    collectors/
    normalizers/
    aggregators/
    publishers/
    types/
  test/
```

## Notes

- The current repo already uses Home Assistant as the intended orchestration layer for the lamp.
- The bridge is a separate local subsystem and should stay isolated from firmware logic.
- `needs_user` should later support blink semantics, but blinking belongs to lamp rendering, not to the bridge.
