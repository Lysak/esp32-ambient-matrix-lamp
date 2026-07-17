# Agent Status Bridge

Mac-side bridge that watches `Codex` and `Claude Code`, normalizes their state, and prepares it for publication into `Home Assistant`.

## Runtime

- `Node.js 24.x`
- `pnpm`

## Setup

```bash
pnpm install
```

## Commands

```bash
pnpm dev
pnpm test
pnpm check
pnpm build
```

## Current scope

This scaffold includes:

- package and tooling setup
- shared bridge status types
- collector and publisher interfaces
- pure aggregation helpers with unit tests
- live `Claude Code` collector using `claude agents --json --all`
- live `Codex` collector reading a real hook event log written by
  `scripts/codex-hook-capture.mjs`
- a family status poll loop wired into `main.ts` for both `Claude Code` and
  `Codex`
- `Home Assistant` publishing: a single aggregated `global_status` (via
  `aggregateGlobalStatus`) is published to an `input_select` entity through
  Home Assistant's REST API, whenever it changes

Confirmed raw `Claude Code` statuses so far: `busy` -> `thinking`, `idle` -> `idle`.
Any other raw status is unconfirmed and is mapped to `error` on purpose, so it
surfaces instead of being silently treated as idle. See
`src/collectors/normalizeClaudeStatus.ts`.

`Codex` has no single poll-based status field like `claude agents --json`.
`src/collectors/codexCollector.ts` instead reads a raw hook event log (one
JSON event per line) and keeps only the most recent event per session.

This scaffold does not include yet:

- richer session grouping, subagent-aware aggregation, or alternative
  publishers beyond Home Assistant (see `mac-agent-status-bridge-design.md`'s
  extension strategy)

`Codex` hooks are registered per-repo in `.codex/hooks.json` at the repository
root (not in the shared `~/.codex/config.toml`, which already has an unrelated
`notify` integration). Confirmed hook events: `SessionStart`, `Stop` -> `idle`;
`UserPromptSubmit`, `PreToolUse`, `PostToolUse`, `SubagentStart`,
`SubagentStop`, `PreCompact`, `PostCompact` -> `thinking`; `PermissionRequest`
-> `needs_user`. Any other event is unconfirmed and maps to `error`. See
`src/collectors/normalizeCodexStatus.ts`.

Codex requires interactive trust approval (`/hooks`) the first time a
repo-scoped hook runs. Run `codex` interactively inside this repo once and
approve the hooks before expecting `.runtime/codex-events.log` to receive
real events.

`scripts/codex-hook-capture.mjs` imports its formatting logic from
`dist/src/hooks/formatCodexHookEvent.js`. Run `pnpm run build` after any
change to `src/hooks/formatCodexHookEvent.ts`, or the hook keeps using the
stale compiled output.

## Notes

- `vite` is pinned to an exact version (`8.1.4`) in `package.json` instead of
  letting `vitest`'s peer range resolve to the newest release. This keeps
  pnpm's `minimumReleaseAge` supply-chain policy enabled (no bypass) while
  still using the newest `vite` that is old enough to clear it. Bump the pin
  yourself once a newer release has had a few days to age.

## Home Assistant setup

1. Copy `.env.example` to `.env` and fill in `HA_URL` and `HA_TOKEN` (a
   long-lived access token from your Home Assistant user profile). `.env`
   is gitignored — never commit it.
2. In your Home Assistant `configuration.yaml`, add the `input_select`
   block from `docs/agent-status-lamp/home-assistant-input-select.yaml`
   (or merge it into an existing `input_select:` key), then restart Home
   Assistant or reload `input_select` entities from Developer Tools -> YAML.
3. Run `pnpm dev`. Without `.env`, the bridge logs
   "HA_URL/HA_TOKEN not set" and behaves exactly as before. With it
   configured, `input_select.agent_status` in Home Assistant should update
   whenever `global_status` changes in the console output.

The mapping from the bridge's internal statuses to Home Assistant's
`input_select` options lives in one place:
`src/publishers/mapToHomeAssistantStatus.ts`. Extending either vocabulary
later (e.g. giving "done" its own HA state instead of "inactive") means
editing only that file.
