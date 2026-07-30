# Compacting Status & Shared Claude/Codex Hooks — Design

Date: 2026-07-28

## Problem

During a long context compaction in Claude Code, the lamp blinks green
repeatedly instead of once. Root cause: Claude's status currently comes
*only* from polling `claude agents --json --all`, which reports just
`busy` / `idle` / `waiting` (`normalizeClaudeStatus.ts`). This CLI has no
concept of "compacting", so mid-compaction the polled status appears to
flap between `busy` and `idle`. `detectFinishedSessions.ts` treats every
`thinking -> idle` edge as "agent finished a turn" and presses the Home
Assistant flash button (`esphome/common/led_matrix.yaml:686-705`), so each
false edge produces another green blink.

Codex does not have this bug: its status is built entirely from global
hooks (`~/.codex/hooks.json`, captured via `codex-hook-capture.mjs` into an
event log, read by `codexCollector.ts`), and `PreCompact`/`PostCompact`
already map to `"thinking"` (`normalizeCodexStatus.ts`), never `"idle"`.

Confirmed via WebSearch (2026-07-28): current Claude Code hooks
(`code.claude.com/docs/en/hooks`) now expose the same event vocabulary
Codex already uses, including dedicated `PreCompact`, `PostCompact`, and
`PermissionRequest` events. `claude agents --json --all` is not part of
Claude Code's documented/stable surface — the hook system is the current,
most reliable source of truth for both agent families.

## Goals

1. Compaction never triggers a false "agent finished" edge / green blink,
   for both Codex and Claude Code.
2. Both agent families get an explicit `needs_user` signal driven by a
   dedicated hook (`PermissionRequest`), not inferred from polling.
3. Claude Code status moves onto the same hook-driven architecture Codex
   already uses, since the two hook vocabularies are now identical —
   eliminating the polling-based ambiguity at its root rather than
   patching around it.
4. Avoid duplicating near-identical Codex/Claude hook plumbing: the hook
   payload shape, status normalization, and capture script are shared,
   parameterized by `family`.

## Non-goals

- No changes to `esphome/common/led_matrix.yaml` or the green "finished"
  blink behavior itself — this is purely about not misfiring it.
- No change to the Home Assistant `input_select.agent_status` schema (still
  `inactive` / `thinking` / `needs_user` / `error`). `compacting` is an
  internal bridge concept that maps to `"thinking"` for HA purposes, so no
  manual HA reconfiguration is required.
- No behavior change for `error`/`done` handling.

## Design

### 1. Data model

- `src/types/status.ts`: add `"compacting"` to `NORMALIZED_STATUSES` and to
  `STATUS_PRIORITY_ASCENDING`, placed between `thinking` and `needs_user`
  (still "agent busy", but not more urgent than an explicit user-attention
  request).
- `src/publishers/mapToHomeAssistantStatus.ts`: map `compacting -> "thinking"`.

### 2. Shared generic hook code (replaces Codex-only modules)

- `src/hooks/normalizeHookStatus.ts` (replaces `normalizeCodexStatus.ts`):
  single `normalizeHookStatus(eventType: string): NormalizedStatus`, used by
  both families since the event vocabulary is now identical:
  - `SessionStart`, `Stop` -> `idle`
  - `UserPromptSubmit`, `PreToolUse`, `PostToolUse`, `SubagentStart`,
    `SubagentStop`, `PostCompact` -> `thinking`
  - `PreCompact` -> `compacting`
  - `PermissionRequest` -> `needs_user`
  - anything else -> `error` (unconfirmed event surfaces loudly)
- `src/hooks/formatHookEvent.ts` (replaces `formatCodexHookEvent.ts`): same
  payload extraction (`session_id`, `cwd`, `hook_event_name`, `turn_id`);
  Codex and Claude Code hook payloads share this shape.
- `scripts/hook-capture.mjs` (replaces `codex-hook-capture.mjs`): identical
  script; selects the event log path via an `AGENT_FAMILY` env var
  (`codex` | `claude`) baked into the installed hook command, so one script
  serves both families.
- Old `normalizeCodexStatus.ts`, `formatCodexHookEvent.ts`,
  `codex-hook-capture.mjs`, and their tests are removed; coverage moves to
  the generic versions (parameterized/looped over both families where the
  behavior is identical).

### 3. Claude Code global hook installer (new, family-aware merge)

Unlike `~/.codex/hooks.json` (owned entirely by this bridge — the installer
overwrites each managed event's array wholesale), `~/.claude/settings.json`
is a shared, user-owned file that already has unrelated hooks installed
(jbcontext indexing, a checklist reminder on `UserPromptSubmit`, etc.), with
possibly several hook groups per event and per-event `matcher`s.

`src/hooks/globalClaudeHooks.ts`:

- Reads existing `~/.claude/settings.json` (preserving every key outside
  `hooks`, same as today).
- For each managed event (`SessionStart`, `UserPromptSubmit`, `PreToolUse`,
  `PostToolUse`, `SubagentStart`, `SubagentStop`, `PreCompact`,
  `PostCompact`, `PermissionRequest`, `Stop`):
  - within that event's existing hook-group array, find the group whose
    command string exactly matches our installed command (identifies "our"
    entry among unrelated ones),
  - replace that group in place, or append a new one if none matched yet,
  - leave every other group (other tools' hooks) untouched.
- Our hook group uses a broad matcher (`"*"`) distinct from any existing
  narrower matcher (e.g. the current `PreToolUse` matcher `"Bash|Grep"` for
  jbcontext stays as its own separate group).
- New `pnpm run install:claude-hooks` script, analogous to
  `install:codex-hooks`.

### 4. Collector wiring

- `src/collectors/claudeCollector.ts` (the `claude agents --json --all`
  poller) is deleted entirely.
- A generic `src/collectors/createHookCollector({ family, readEventLog })`
  replaces both `codexCollector.ts` and the old Claude poller: reads the
  family's event log, keeps the latest event per `sourceSessionId`, maps
  through `normalizeHookStatus`.
- `src/main.ts`: both family loops now use `createHookCollector`, reading
  from family-specific log paths (`getEventLogPath("codex" | "claude")` in
  a generalized `runtime/eventLogPaths.ts` replacing `codexPaths.ts`).

### 5. Finished-edge detection

- `detectFinishedSessions.ts` keeps its existing `thinking -> idle` edge
  rule unchanged. No special-case "ignore compacting" logic is needed:
  since `compacting` is now its own status (never collapsed into `idle`),
  the edge simply never fires mid-compaction. Same for Codex, which already
  behaved this way before this change.

## Testing

- vitest: `normalizeHookStatus` (all mapped events + default-to-error for
  both families, since the vocab is shared), `createHookCollector`,
  `formatHookEvent`.
- vitest: `globalClaudeHooks` merge/reconcile logic — existing unrelated
  hook groups preserved, re-running the installer updates (not duplicates)
  our own group, first install appends cleanly to an event that already has
  other groups.
- Manual: run both installers, restart Claude Code and Codex sessions so
  hooks are picked up, trigger a real compaction and confirm no green blink,
  trigger a permission prompt and confirm `needs_user`.

## Open items for the implementation plan

- Exact matcher value Claude Code expects for "all tools" on `PreToolUse`/
  `PostToolUse` (`"*"` vs `""` vs omitted) — verify against
  `code.claude.com/docs/en/hooks` during implementation, not assumed here.
- Confirm `turn_id` is present on Claude Code hook payloads the same way it
  is on Codex's, since `detectFinishedSessions`'s Codex branch relies on it;
  if absent, that branch's `updatedAt`-only fallback already covers it.
