# Global Codex Hooks for Agent Status Bridge

Date: 2026-07-19

## Goal

Make the `Codex` -> lamp finished-signal path machine-wide on this Mac, not
repository-scoped, so any `Codex` session can trigger the same lamp behavior
that `Claude Code` already triggers through the bridge.

## Problem

The current `Codex` integration is wired through repo-local hooks:

- `.codex/hooks.json` in this repository
- `tools/agent-status-bridge/scripts/codex-hook-capture.mjs`
- `tools/agent-status-bridge/.runtime/codex-events.log`

That means only `Codex` sessions started in this repository write events that
the bridge can see. This is below the intended product behavior. The target
behavior is machine-wide parity with `Claude Code`, where any relevant session
on the Mac is eligible to drive the lamp.

## Non-Goals

- Replacing hooks with SQLite polling or TUI scraping
- Changing `Claude Code` collection
- Changing lamp-side blink semantics
- Adding richer statuses beyond the current finished-signal behavior
- Building launchd packaging in the same change

## Decision

Use global `Codex` hooks as the primary integration path.

Do not keep repo-local `.codex/hooks.json` on the critical path for production
behavior. It may remain as a development fallback only if that does not create
ambiguous duplicate events.

## Architecture

### Shared machine-wide capture

All `Codex` sessions on the Mac should invoke the same hook command.

That command should call one shared capture script that:

- reads the hook payload from stdin
- normalizes it into the existing raw event shape
- appends one JSON line to a shared machine-wide event log
- always returns `{ "continue": true }`

The capture script remains the only writer. The bridge remains a reader only.

### Shared machine-wide runtime storage

`Codex` events must no longer be stored under this repository.

Use a machine-wide runtime path under the current user account, not a repo path.
Preferred location:

- `~/Library/Application Support/ambient-matrix-agent-status/codex-events.log`

This path is:

- stable across repositories
- scoped to the local macOS user
- appropriate for long-lived local app state

If the parent directory does not exist, the capture script creates it.

### Bridge read path

`tools/agent-status-bridge/src/main.ts` should stop hardcoding the repo-local
`.runtime/codex-events.log` path.

Instead:

- default to the shared machine-wide log path
- allow override with an env var for development and tests

Recommended env var:

- `CODEX_EVENT_LOG_PATH`

Behavior:

- if `CODEX_EVENT_LOG_PATH` is set, read that path
- otherwise read the default machine-wide path

### Event semantics

Keep the current event semantics unchanged.

For `Codex`:

- `Stop` means the turn finished and the agent is now idle
- finished detection remains per-session
- multiple concurrent sessions are allowed
- a finished turn from one session can trigger the lamp even if other sessions
  are still active

This preserves parity with the existing bridge logic and avoids mixing the
storage change with behavior changes.

## Hook installation model

The system needs one global `Codex` hook registration under the user's home
directory, not one copy per repository.

The installation flow should:

1. point global `Codex` hook config at the shared capture script
2. register the same event set already used today:
   - `SessionStart`
   - `UserPromptSubmit`
   - `PreToolUse`
   - `PostToolUse`
   - `SubagentStart`
   - `SubagentStop`
   - `PreCompact`
   - `PostCompact`
   - `PermissionRequest`
   - `Stop`
3. preserve the "always continue" contract so hooks never block Codex

The exact global hook config file path and shape must match the live Codex
hooks surface on this Mac. The implementation should verify and use the
currently supported global location, not invent a parallel config format.

## Compatibility and migration

The migration must be explicit.

After this change:

- existing repo-local hooks should not be relied on for normal behavior
- documentation must clearly say that machine-wide behavior requires the global
  hook install step
- the bridge should work even when no repo-specific `.codex/hooks.json` exists

During transition, duplicate hook registrations are risky because they can write
duplicate events and cause duplicate lamp flashes. The implementation should
either:

- remove repo-local registration from the documented path, or
- document that only one registration path should be active at a time

## Files and responsibilities

### Existing files to modify

- `tools/agent-status-bridge/src/main.ts`
  - default `Codex` event log path resolution
- `tools/agent-status-bridge/scripts/codex-hook-capture.mjs`
  - write to machine-wide runtime path instead of repo-local runtime path
- `tools/agent-status-bridge/README.md`
  - update setup and operating model
- `tools/agent-status-bridge/.env.example`
  - optional `CODEX_EVENT_LOG_PATH` override documentation
- `tools/agent-status-bridge/test/codexCollector.test.ts`
  - ensure collector behavior still matches the new path contract where needed
- `tools/agent-status-bridge/test/formatCodexHookEvent.test.ts`
  - keep existing event-shape guarantees intact

### New files to add

- `tools/agent-status-bridge/src/runtime/codexPaths.ts`
  - one place for machine-wide default path resolution
- `tools/agent-status-bridge/test/codexPaths.test.ts`
  - path-resolution unit tests
- `tools/agent-status-bridge/scripts/install-global-codex-hooks.mjs`
  - idempotent installer for global hooks on this machine

Optional:

- `tools/agent-status-bridge/scripts/print-codex-hook-command.mjs`
  - only if needed to keep installer logic simple

## Error handling

The capture path must fail safe.

Rules:

- malformed hook JSON must not block `Codex`
- filesystem write failures must not block `Codex`
- the capture script must still emit `{ "continue": true }`
- the bridge should treat missing event log files as "no events yet"

The installer path must fail loud.

Rules:

- if the global hooks config path is unsupported or cannot be updated, exit
  non-zero with a precise error
- if the script would overwrite a conflicting existing config, report the
  conflict clearly instead of silently destroying unrelated user setup

## Testing

### Automated

- unit tests for machine-wide default path resolution
- unit tests proving env override beats the default path
- existing `formatCodexHookEvent` tests remain green
- existing `codexCollector` and `detectFinishedSessions` tests remain green

### Manual

1. install global hooks
2. run the bridge
3. start a `Codex` session in this repository and confirm a `Stop` reaches the
   shared event log
4. start a separate `Codex` session in a different repository and confirm its
   `Stop` reaches the same shared event log
5. confirm each finished turn triggers one HA button press and one lamp blink
6. confirm `Claude Code` behavior is unchanged

## Risks

### Duplicate event writes

If both repo-local and global hooks stay active, one finish can produce multiple
identical raw events. That can cause duplicate lamp flashes.

Mitigation:

- make the global install path the only documented production path
- keep migration guidance explicit

### Global config drift

The Codex global hooks surface may evolve.

Mitigation:

- use the currently supported live config path on this machine
- keep installer logic small and easy to audit
- document the exact expected config location

### Script path fragility

A global hook pointing into this repo depends on this checkout continuing to
exist at the same path.

Mitigation:

- document that the global install is bound to this checkout path
- keep the script path centralized in the installer
- if needed later, move to a dedicated stable tool path as a separate change

## Recommendation

Implement the smallest viable machine-wide upgrade:

- shared machine-wide event log path
- shared path helper in bridge code
- global hook installer script
- updated docs

Do not mix this with lamp-effect tweaks or broader status-model changes.
