# Global Codex Hooks Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make Codex finished-signal capture machine-wide on this Mac by switching the bridge from repo-local hooks/logs to global hooks and a shared event log.

**Architecture:** Add one runtime path helper that defines the shared Codex event log location, reuse it from both the bridge reader and hook capture writer, and add a small installer that writes the current hook command set into `~/.codex/hooks.json`. Keep the existing event format and finished-detection semantics unchanged so the storage/install change is isolated from behavior changes.

**Tech Stack:** Node.js 24, TypeScript 7, pnpm, tsx, vitest, Biome

## Global Constraints

- Use global `Codex` hooks as the primary integration path.
- Do not replace hooks with SQLite polling or TUI scraping.
- Do not change `Claude Code` collection.
- Do not change lamp-side blink semantics.
- Keep the current `Codex` event semantics unchanged.
- The capture path must always return `{ "continue": true }` even on malformed JSON or write errors.
- Build repo edits with existing toolchain and keep changes focused and minimal.

---

### Task 1: Add shared machine-wide Codex path resolution

**Files:**
- Create: `tools/agent-status-bridge/src/runtime/codexPaths.ts`
- Create: `tools/agent-status-bridge/test/codexPaths.test.ts`
- Modify: `tools/agent-status-bridge/src/main.ts`

**Interfaces:**
- Produces: `getDefaultCodexEventLogPath(): string`
- Produces: `getCodexEventLogPath(env: NodeJS.ProcessEnv): string`
- Consumes: existing `readCodexEventLog()` in `src/main.ts`

- [ ] **Step 1: Write the failing test**

```ts
import { describe, expect, it } from "vitest";
import { getCodexEventLogPath, getDefaultCodexEventLogPath } from "../src/runtime/codexPaths.js";

describe("codexPaths", () => {
  it("returns the macOS machine-wide default path", () => {
    expect(getDefaultCodexEventLogPath()).toContain(
      "Library/Application Support/ambient-matrix-agent-status/codex-events.log",
    );
  });

  it("prefers CODEX_EVENT_LOG_PATH override", () => {
    expect(
      getCodexEventLogPath({ CODEX_EVENT_LOG_PATH: "/tmp/custom-codex.log" } as NodeJS.ProcessEnv),
    ).toBe("/tmp/custom-codex.log");
  });
});
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd tools/agent-status-bridge && pnpm test codexPaths.test.ts`
Expected: FAIL because `src/runtime/codexPaths.ts` does not exist yet

- [ ] **Step 3: Write minimal implementation**

```ts
import { homedir } from "node:os";
import { join } from "node:path";

export function getDefaultCodexEventLogPath(): string {
  return join(
    homedir(),
    "Library",
    "Application Support",
    "ambient-matrix-agent-status",
    "codex-events.log",
  );
}

export function getCodexEventLogPath(env: NodeJS.ProcessEnv): string {
  return env.CODEX_EVENT_LOG_PATH || getDefaultCodexEventLogPath();
}
```

Update `src/main.ts` so the reader uses `getCodexEventLogPath(process.env)` instead of the repo-local `.runtime` path.

- [ ] **Step 4: Run test to verify it passes**

Run: `cd tools/agent-status-bridge && pnpm test codexPaths.test.ts`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add tools/agent-status-bridge/src/runtime/codexPaths.ts \
        tools/agent-status-bridge/src/main.ts \
        tools/agent-status-bridge/test/codexPaths.test.ts
git commit -m "feat: add shared codex log path helper"
```

### Task 2: Move hook capture to the shared log path and add global hook installer

**Files:**
- Modify: `tools/agent-status-bridge/scripts/codex-hook-capture.mjs`
- Create: `tools/agent-status-bridge/scripts/install-global-codex-hooks.mjs`
- Modify: `tools/agent-status-bridge/package.json`

**Interfaces:**
- Consumes: `formatCodexHookEvent(payload, nowIso)`
- Consumes: `getDefaultCodexEventLogPath(): string`
- Produces: installer CLI `node scripts/install-global-codex-hooks.mjs`

- [ ] **Step 1: Write the failing tests**

```ts
it("keeps hook formatter output valid for the capture script", () => {
  const result = formatCodexHookEvent(
    { session_id: "abc", cwd: "/tmp", hook_event_name: "Stop", turn_id: "t1" },
    "2026-07-19T00:00:00.000Z",
  );

  expect(result.logLine).toContain("\"type\":\"Stop\"");
  expect(result.response).toBe(JSON.stringify({ continue: true }));
});
```

Add a new installer test shape:

```ts
expect(serialized.hooks.Stop[0].hooks[0].command).toContain("codex-hook-capture.mjs");
```

- [ ] **Step 2: Run targeted tests to verify failure**

Run: `cd tools/agent-status-bridge && pnpm test formatCodexHookEvent.test.ts`
Expected: existing formatter test still passes, but installer test fails because installer module does not exist yet

- [ ] **Step 3: Write minimal implementation**

Implement:

- `codex-hook-capture.mjs` writes to the shared machine-wide path, creating parent directories as needed
- `install-global-codex-hooks.mjs`:
  - resolves `~/.codex/hooks.json`
  - writes the full event set (`SessionStart`, `UserPromptSubmit`, `PreToolUse`, `PostToolUse`, `SubagentStart`, `SubagentStop`, `PreCompact`, `PostCompact`, `PermissionRequest`, `Stop`)
  - points every command to the repo’s `scripts/codex-hook-capture.mjs`
  - preserves `{ continue: true }` semantics by leaving runtime behavior in the capture script
- add a package script:

```json
"install:codex-hooks": "node scripts/install-global-codex-hooks.mjs"
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `cd tools/agent-status-bridge && pnpm test formatCodexHookEvent.test.ts codexPaths.test.ts`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add tools/agent-status-bridge/scripts/codex-hook-capture.mjs \
        tools/agent-status-bridge/scripts/install-global-codex-hooks.mjs \
        tools/agent-status-bridge/package.json
git commit -m "feat: add global codex hook installer"
```

### Task 3: Update docs, env surface, and full verification

**Files:**
- Modify: `tools/agent-status-bridge/.env.example`
- Modify: `tools/agent-status-bridge/README.md`
- Modify: `tools/agent-status-bridge/test/package-shape.test.ts`

**Interfaces:**
- Consumes: new package script `install:codex-hooks`
- Consumes: `CODEX_EVENT_LOG_PATH` env override

- [ ] **Step 1: Write the failing test**

Extend `package-shape.test.ts` with:

```ts
expect(pkg.scripts["install:codex-hooks"]).toBe("node scripts/install-global-codex-hooks.mjs");
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd tools/agent-status-bridge && pnpm test package-shape.test.ts`
Expected: FAIL until the script is added

- [ ] **Step 3: Write minimal implementation**

Document:

- machine-wide default log path
- `CODEX_EVENT_LOG_PATH` override
- global `~/.codex/hooks.json` install flow
- warning against keeping both repo-local and global hook registrations active

Add the env var comment to `.env.example`.

- [ ] **Step 4: Run full bridge verification**

Run:

```bash
cd tools/agent-status-bridge && pnpm test
cd tools/agent-status-bridge && pnpm check
cd tools/agent-status-bridge && pnpm build
```

Expected:

- tests: PASS
- check: PASS
- build: PASS

Manual follow-up after code lands:

```bash
cd tools/agent-status-bridge && pnpm run install:codex-hooks
cat /Users/dmytrii.lysak/.codex/hooks.json
```

Expected: global hooks now point to this repo’s `codex-hook-capture.mjs`

- [ ] **Step 5: Commit**

```bash
git add tools/agent-status-bridge/.env.example \
        tools/agent-status-bridge/README.md \
        tools/agent-status-bridge/test/package-shape.test.ts
git commit -m "docs: document global codex hook setup"
```
