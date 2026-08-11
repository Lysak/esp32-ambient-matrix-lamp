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
- live `Claude Code` collector reading a real hook event log written by
  `scripts/claude-hook-capture.mjs`
- live `Codex` collector reading a real hook event log written by
  `scripts/codex-hook-capture.mjs`
- a family status poll loop wired into `main.ts` for both `Claude Code` and
  `Codex`
- `Home Assistant` publishing: a single aggregated `global_status` (via
  `aggregateGlobalStatus`) is published to an `input_select` entity through
  Home Assistant's REST API, whenever it changes
- per-session "finished" signal: when an individual session goes
  `thinking` -> `idle` (`detectFinishedSessions`), the bridge presses a Home
  Assistant `button` entity directly (`pressHomeAssistantButton`), which the
  lamp firmware turns into a short green blink. The button entity id is
  configurable via `HA_FINISHED_BUTTON_ENTITY_ID` (default
  `button.ambient_matrix_lamp_agent_finished_flash`). No Home Assistant
  automation is involved. See
  `docs/agent-status-lamp/` and the design spec under `docs/superpowers/specs/`.
- per-session "needs you" signal: when an individual session crosses into
  `needs_user` (`detectNeedsUserSessions`), the bridge presses a second
  `button` entity the same way, which the lamp firmware turns into a short
  amber/yellow blink instead of green. Configurable via
  `HA_NEEDS_USER_BUTTON_ENTITY_ID` (default
  `button.ambient_matrix_lamp_agent_needs_user_flash`).

Neither `Claude Code` nor `Codex` has a poll-based status field, so both
collectors (`src/collectors/claudeCollector.ts`,
`src/collectors/codexCollector.ts`) read a raw hook event log (one JSON event
per line) and keep only the most recent event per session, instead of
spawning a CLI process every poll. This keeps the bridge's own CPU overhead
close to zero — a 2-second file read versus a subprocess spawn (previously
`claude agents --json --all`, which cost ~0.28s CPU per call, i.e. ~14% of a
core running continuously).

By default those logs are machine-wide:

- `~/Library/Application Support/ambient-matrix-agent-status/claude-events.log`
- `~/Library/Application Support/ambient-matrix-agent-status/codex-events.log`

Override them only for development/tests with `CLAUDE_EVENT_LOG_PATH` /
`CODEX_EVENT_LOG_PATH`.

Confirmed `Claude Code` hook events: `SessionStart`, `Stop` -> `idle`;
`UserPromptSubmit`, `PreToolUse`, `PostToolUse` -> `thinking`; `Notification`
(wired only to the `permission_prompt` matcher) -> `needs_user`. Any other
event is unconfirmed and maps to `error`. See
`src/collectors/normalizeClaudeHookStatus.ts`.

`Claude Code` hooks are registered globally in `~/.claude/settings.json`, not
per repo, alongside whatever other hooks you already have there. Install or
refresh them with:

```bash
pnpm run install:claude-hooks
```

`scripts/claude-hook-capture.mjs` imports its formatting logic from
`dist/src/hooks/formatClaudeHookEvent.js` and
`dist/src/runtime/claudePaths.js`. Run `pnpm run build` after any change to
`src/hooks/formatClaudeHookEvent.ts` or `src/runtime/claudePaths.ts`, or the
hook keeps using the stale compiled output.

This scaffold does not include yet:

- richer session grouping, subagent-aware aggregation, or alternative
  publishers beyond Home Assistant (see `mac-agent-status-bridge-design.md`'s
  extension strategy)

`Codex` hooks are registered globally in `~/.codex/hooks.json`, not per repo.
Install or refresh them with:

```bash
pnpm run install:codex-hooks
```

The installer prefers a stable Node symlink such as `/opt/homebrew/bin/node`
or `/usr/local/bin/node` when one exists, so `~/.codex/hooks.json` does not
drift to a versioned Homebrew Cellar path that breaks after Node upgrades.

Confirmed hook events: `SessionStart`, `Stop` -> `idle`; `UserPromptSubmit`,
`PreToolUse`, `PostToolUse`, `SubagentStart`, `SubagentStop`, `PreCompact`,
`PostCompact` -> `thinking`; `PermissionRequest` -> `needs_user`. Any other
event is unconfirmed and maps to `error`. See
`src/collectors/normalizeCodexStatus.ts`.

Codex requires interactive trust approval (`/hooks`) the first time a global
hook runs in a newly trusted repo. After installing the hooks, run `codex`
interactively in any repo where you want events and approve the hooks when
prompted.

`scripts/codex-hook-capture.mjs` imports its formatting logic from
`dist/src/hooks/formatCodexHookEvent.js` and
`dist/src/runtime/codexPaths.js`. Run `pnpm run build` after any change to
`src/hooks/formatCodexHookEvent.ts` or `src/runtime/codexPaths.ts`, or the
hook keeps using the stale compiled output.

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
2. Install the global Codex and Claude Code hooks:

```bash
pnpm run install:codex-hooks
pnpm run install:claude-hooks
```

3. In your Home Assistant `configuration.yaml`, add the `input_select`
   block from `docs/agent-status-lamp/home-assistant-input-select.yaml`
   (or merge it into an existing `input_select:` key), then restart Home
   Assistant or reload `input_select` entities from Developer Tools -> YAML.
4. Run `pnpm dev`. Without `.env`, the bridge logs
   "HA_URL/HA_TOKEN not set" and behaves exactly as before. With it
   configured, `input_select.agent_status` in Home Assistant should update
   whenever `global_status` changes in the console output.
5. For it to survive logout/reboot instead of only running while `pnpm dev`
   is open in a terminal, install it as a macOS launchd agent:

```bash
pnpm run install:launchd
```

This builds the project, writes
`~/Library/LaunchAgents/com.ambientmatrix.agentstatusbridge.plist` (pointing
at this checkout's `dist/src/main.js`, with `RunAtLoad` and `KeepAlive`), and
loads it. It sets its own `PATH` (`/opt/homebrew/bin`, `~/.local/bin`, plus
the standard system dirs), because launchd does not source your shell
profile. Logs still go to
`~/Library/Logs/agent-status-bridge/bridge.log`. Re-run the command any time
the repo moves or `dist/` changes. To stop it:
`launchctl unload ~/Library/LaunchAgents/com.ambientmatrix.agentstatusbridge.plist`.
See `src/runtime/launchAgent.ts` and `scripts/install-launchd-agent.mjs`.

It also installs a second, independent watchdog agent
(`com.ambientmatrix.agentstatusbridge.watchdog`, `RunAtLoad` + a 5-minute
`StartInterval`, no `KeepAlive`). launchd is expected to bootstrap
`RunAtLoad` agents from `~/Library/LaunchAgents` automatically on every
login, but this was observed to silently not happen on at least one boot —
not disabled, not blocked by Background Task Management, just never loaded,
requiring a manual `launchctl bootstrap`. The watchdog just checks whether
the main agent is known to launchd and re-bootstraps it if not; it exits
immediately either way, logging to
`~/Library/Logs/agent-status-bridge/watchdog.log`. See
`src/runtime/launchAgentWatchdog.ts`.

It also installs a third agent, `com.ambientmatrix.agentstatusbridge.healthcheck`
(`RunAtLoad` only, no `KeepAlive`/`StartInterval` — runs once per
login/reboot). It runs `scripts/health-check.sh`, which checks whether the
main agent is actually running and whether the global Codex and Claude Code
hook registrations are still present in `~/.codex/hooks.json` and
`~/.claude/settings.json`. The watchdog above only catches the LaunchAgent
never being bootstrapped; it can't catch the hooks silently disappearing from
those config files (e.g. an editor overwriting `settings.json`), which is a
separate failure mode this has hit before. On any failure it fires a single
macOS notification via `osascript`; it does not attempt to fix anything
itself — re-run `pnpm run install:codex-hooks` / `install:claude-hooks` by
hand. Logs to `~/Library/Logs/agent-status-bridge/health-check.log`. See
`src/runtime/healthCheckAgent.ts`.

Do not keep both repo-local and global Codex hook registrations active at the
same time. That can duplicate raw events and cause duplicate lamp flashes.
The same applies to Claude Code hooks if you ever add a repo-local
`.claude/settings.json` with its own hooks for the same events.

The bridge also refuses to run a second instance of itself: on startup it
takes a PID lock at
`~/Library/Application Support/ambient-matrix-agent-status/bridge.pid`
(override with `AGENT_STATUS_BRIDGE_LOCK_PATH`, mainly for tests). If a live
process already holds the lock, the new instance logs an error and exits
instead of starting a second poll loop that would independently detect the
same finished session and double- (or triple-) fire the Home Assistant flash
button. A stale lock left behind by a killed process is reclaimed
automatically. See `src/runtime/singletonLock.ts`.

The mapping from the bridge's internal statuses to Home Assistant's
`input_select` options lives in one place:
`src/publishers/mapToHomeAssistantStatus.ts`. Extending either vocabulary
later (e.g. giving "done" its own HA state instead of "inactive") means
editing only that file.
