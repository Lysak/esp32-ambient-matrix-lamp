# Codex Collector Parity Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Bring the `Codex` side of `tools/agent-status-bridge/` to the same working level as `Claude Code`: real hook events from a real Codex session in this repo flow into the existing `codexCollector`/`normalizeCodexStatus` pipeline and print a live `codex_status: idle|thinking|...` line from `main.ts`, the same way `claude_status` already does.

**Architecture:** Codex has no poll-based status command like `claude agents --json --all`. Instead, Codex CLI 0.144.5 has a documented hooks system (`~/.codex/hooks.json` or `[hooks]` in `config.toml`, also readable per-repo from `<repo>/.codex/hooks.json`). Each hook handler is an external command that receives a JSON object on stdin (`session_id`, `cwd`, `hook_event_name`, ...) and must print a JSON response (`{"continue": true}`) on stdout. We register a repo-scoped hook (`.codex/hooks.json` at the repo root) for every lifecycle event Codex documents, pointing at a small capture script that appends one JSON line per event to a local log file. `codexCollector.ts` (already built and tested) reads that log file, keeps the latest event per session, and normalizes it. This is scoped to the repo only — it does **not** touch the user's existing `~/.codex/config.toml` `notify` setting, which is already wired to an unrelated "Codex Computer Use" integration and must not be disturbed.

**Tech Stack:** Node.js 24, TypeScript 7, pnpm, tsx, Vitest, Biome (existing `tools/agent-status-bridge/` package).

## Global Constraints

- Do not modify `~/.codex/config.toml`'s existing `notify` field — it is already used by an unrelated integration ("Codex Computer Use"). Hooks and `notify` are separate mechanisms; this work only adds a `[hooks]`-style registration, scoped to `<repo>/.codex/hooks.json`.
- Confirmed real Codex hook event names (from `developers.openai.com/codex/hooks`, redirected to `learn.chatgpt.com/codex/hooks`): `SessionStart`, `SubagentStart`, `SubagentStop`, `PreToolUse`, `PostToolUse`, `PermissionRequest`, `PreCompact`, `PostCompact`, `UserPromptSubmit`, `Stop`. Do not invent additional event names.
- Match Claude's current scope, per explicit user direction: distinguish "working" vs "not working" first (`idle` / `thinking`), same as `normalizeClaudeStatus` today. `PermissionRequest` -> `needs_user` is kept as a third real state because it is trivial and already implemented — do not add further granularity (no attempt yet to split `done` from `idle`, no per-tool nuance).
- Any hook event name not in the confirmed list still maps to `error` (unconfirmed, surfaces loudly) — same defensive philosophy as `normalizeClaudeStatus`.
- The hook capture script must never block or fail Codex: always exit `0` and always print `{"continue": true}` on stdout, even on a malformed payload.
- This bridge is a single-machine, single-user tool (per `docs/agent-status-lamp/mac-agent-status-bridge-design.md`) — absolute paths tied to this checkout and this machine's Node install are acceptable in `.codex/hooks.json`; do not add portability machinery nobody asked for.
- The hook capture script (`scripts/codex-hook-capture.mjs`) must import `formatCodexHookEvent` from the compiled `dist/` output rather than duplicating its field-extraction logic — single source of truth over build-freshness convenience. This means `pnpm run build` must be run at least once after Task 2/Task 3 changes before the hook actually reflects them; call this out in the script's own comment and in the README (Task 6).
- All code, identifiers, comments, and commit messages in English (per `AGENTS.md`).
- Use the repository's existing pnpm scripts (`pnpm test`, `pnpm run check`, `pnpm run build`) inside `tools/agent-status-bridge/` to verify each task — do not invoke `vitest`/`tsc`/`biome` directly except via those scripts, to match how the package is already run.

---

### Task 1: Expand `normalizeCodexStatus` to the confirmed hook vocabulary

**Files:**
- Modify: `tools/agent-status-bridge/src/collectors/normalizeCodexStatus.ts`
- Modify: `tools/agent-status-bridge/test/normalizeCodexStatus.test.ts`

**Interfaces:**
- Consumes: `NormalizedStatus` from `tools/agent-status-bridge/src/types/status.ts` (unchanged)
- Produces: `normalizeCodexStatus(rawEventType: string): NormalizedStatus` (signature unchanged, mapping expanded)

- [ ] **Step 1: Write the failing tests for the expanded mapping**

Replace the contents of `tools/agent-status-bridge/test/normalizeCodexStatus.test.ts` with:

```ts
import { describe, expect, it } from "vitest";
import { normalizeCodexStatus } from "../src/collectors/normalizeCodexStatus.js";

describe("normalizeCodexStatus", () => {
  it("maps 'SessionStart' to 'idle' (not working yet)", () => {
    expect(normalizeCodexStatus("SessionStart")).toBe("idle");
  });

  it("maps 'Stop' to 'idle' (turn finished)", () => {
    expect(normalizeCodexStatus("Stop")).toBe("idle");
  });

  it("maps 'UserPromptSubmit' to 'thinking' (working)", () => {
    expect(normalizeCodexStatus("UserPromptSubmit")).toBe("thinking");
  });

  it("maps 'PreToolUse' to 'thinking' (working)", () => {
    expect(normalizeCodexStatus("PreToolUse")).toBe("thinking");
  });

  it("maps 'PostToolUse' to 'thinking' (working)", () => {
    expect(normalizeCodexStatus("PostToolUse")).toBe("thinking");
  });

  it("maps 'SubagentStart' to 'thinking' (working)", () => {
    expect(normalizeCodexStatus("SubagentStart")).toBe("thinking");
  });

  it("maps 'SubagentStop' to 'thinking' (parent turn still working)", () => {
    expect(normalizeCodexStatus("SubagentStop")).toBe("thinking");
  });

  it("maps 'PreCompact' to 'thinking' (working)", () => {
    expect(normalizeCodexStatus("PreCompact")).toBe("thinking");
  });

  it("maps 'PostCompact' to 'thinking' (working)", () => {
    expect(normalizeCodexStatus("PostCompact")).toBe("thinking");
  });

  it("maps 'PermissionRequest' to 'needs_user'", () => {
    expect(normalizeCodexStatus("PermissionRequest")).toBe("needs_user");
  });

  it("maps any unconfirmed raw event to 'error' so it is visible, not silently ignored", () => {
    expect(normalizeCodexStatus("some_future_event")).toBe("error");
  });
});
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `cd tools/agent-status-bridge && pnpm test -- test/normalizeCodexStatus.test.ts`

Expected: FAIL — the new event names (`PreToolUse`, `PostToolUse`, `SubagentStart`, `SubagentStop`, `PreCompact`, `PostCompact`) currently fall through to the `default: return "error"` branch, so those specific assertions fail while `SessionStart`/`Stop`/`UserPromptSubmit`/`PermissionRequest`/the unconfirmed-event case still pass.

- [ ] **Step 3: Write the minimal implementation**

Replace the contents of `tools/agent-status-bridge/src/collectors/normalizeCodexStatus.ts` with:

```ts
import type { NormalizedStatus } from "../types/status.js";

/**
 * Codex has no single poll-based status field like `claude agents --json`.
 * Instead it emits lifecycle hook events (documented at
 * developers.openai.com/codex/hooks). For parity with the current
 * `normalizeClaudeStatus` scope, this only distinguishes "working" from
 * "not working", plus "needs_user" since PermissionRequest makes that free.
 * Any other raw event is unconfirmed and maps to "error" so it surfaces
 * loudly instead of being silently treated as idle.
 */
export function normalizeCodexStatus(rawEventType: string): NormalizedStatus {
  switch (rawEventType) {
    case "SessionStart":
    case "Stop":
      return "idle";
    case "UserPromptSubmit":
    case "PreToolUse":
    case "PostToolUse":
    case "SubagentStart":
    case "SubagentStop":
    case "PreCompact":
    case "PostCompact":
      return "thinking";
    case "PermissionRequest":
      return "needs_user";
    default:
      return "error";
  }
}
```

- [ ] **Step 4: Run the tests to verify they pass**

Run: `cd tools/agent-status-bridge && pnpm test -- test/normalizeCodexStatus.test.ts`

Expected: PASS, all 11 tests green.

- [ ] **Step 5: Run the full suite to check for regressions**

Run: `cd tools/agent-status-bridge && pnpm test`

Expected: all test files pass (existing `codexCollector.test.ts` still passes since it only exercises `SessionStart`, `Stop`, `UserPromptSubmit`, `PermissionRequest`, all still mapped the same way).

- [ ] **Step 6: Commit**

```bash
git add tools/agent-status-bridge/src/collectors/normalizeCodexStatus.ts tools/agent-status-bridge/test/normalizeCodexStatus.test.ts
git commit -m "feat: expand normalizeCodexStatus to the confirmed hook event vocabulary"
```

---

### Task 2: Shared `RawCodexEvent` type and a pure hook-payload formatter

**Files:**
- Create: `tools/agent-status-bridge/src/types/codexEvent.ts`
- Modify: `tools/agent-status-bridge/src/collectors/codexCollector.ts`
- Create: `tools/agent-status-bridge/src/hooks/formatCodexHookEvent.ts`
- Test: `tools/agent-status-bridge/test/formatCodexHookEvent.test.ts`

**Interfaces:**
- Consumes: nothing new from earlier tasks
- Produces:
  - `RawCodexEvent` (moved, same shape: `{ sessionId: string; label: string; type: string; timestamp: string }`)
  - `formatCodexHookEvent(payload: unknown, nowIso: string): { event: RawCodexEvent; logLine: string; response: string }`

- [ ] **Step 1: Extract the shared `RawCodexEvent` type**

Create `tools/agent-status-bridge/src/types/codexEvent.ts`:

```ts
export interface RawCodexEvent {
  sessionId: string;
  label: string;
  type: string;
  timestamp: string;
}
```

- [ ] **Step 2: Point `codexCollector.ts` at the shared type**

In `tools/agent-status-bridge/src/collectors/codexCollector.ts`, replace:

```ts
import type { SourceSessionSnapshot } from "../types/source.js";
import { normalizeCodexStatus } from "./normalizeCodexStatus.js";
import type { Collector } from "./types.js";

interface RawCodexEvent {
  sessionId: string;
  label: string;
  type: string;
  timestamp: string;
}
```

with:

```ts
import type { SourceSessionSnapshot } from "../types/source.js";
import type { RawCodexEvent } from "../types/codexEvent.js";
import { normalizeCodexStatus } from "./normalizeCodexStatus.js";
import type { Collector } from "./types.js";
```

- [ ] **Step 3: Run the full suite to confirm the refactor is safe**

Run: `cd tools/agent-status-bridge && pnpm test`

Expected: all test files still pass (this is a pure type-location refactor, no behavior change) — this step has no new failing test because it's a refactor step, not a new behavior; confirm nothing broke before adding new behavior in the next step.

- [ ] **Step 4: Write the failing test for the pure hook-payload formatter**

Create `tools/agent-status-bridge/test/formatCodexHookEvent.test.ts`:

```ts
import { describe, expect, it } from "vitest";
import { formatCodexHookEvent } from "../src/hooks/formatCodexHookEvent.js";

const NOW = "2026-07-16T12:00:00.000Z";

describe("formatCodexHookEvent", () => {
  it("builds a RawCodexEvent from a well-formed hook payload", () => {
    const result = formatCodexHookEvent(
      {
        session_id: "abc-123",
        cwd: "/Users/Files/www/pet/esp32-ambient-matrix-lamp",
        hook_event_name: "UserPromptSubmit",
      },
      NOW,
    );

    expect(result.event).toEqual({
      sessionId: "abc-123",
      label: "/Users/Files/www/pet/esp32-ambient-matrix-lamp",
      type: "UserPromptSubmit",
      timestamp: NOW,
    });
  });

  it("produces a single-line JSON log line terminated by a newline", () => {
    const result = formatCodexHookEvent(
      { session_id: "abc-123", cwd: "/tmp", hook_event_name: "Stop" },
      NOW,
    );

    expect(result.logLine).toBe(
      `${JSON.stringify({
        sessionId: "abc-123",
        label: "/tmp",
        type: "Stop",
        timestamp: NOW,
      })}\n`,
    );
  });

  it("defaults missing or non-string fields to empty strings instead of throwing", () => {
    const result = formatCodexHookEvent({}, NOW);

    expect(result.event).toEqual({
      sessionId: "",
      label: "",
      type: "",
      timestamp: NOW,
    });
  });

  it("always responds with continue: true so Codex is never blocked", () => {
    const result = formatCodexHookEvent(null, NOW);

    expect(result.response).toBe(JSON.stringify({ continue: true }));
  });
});
```

- [ ] **Step 5: Run the test to verify it fails**

Run: `cd tools/agent-status-bridge && pnpm test -- test/formatCodexHookEvent.test.ts`

Expected: FAIL — `Cannot find module '../src/hooks/formatCodexHookEvent.js'`.

- [ ] **Step 6: Write the minimal implementation**

Create `tools/agent-status-bridge/src/hooks/formatCodexHookEvent.ts`:

```ts
import type { RawCodexEvent } from "../types/codexEvent.js";

export interface FormatCodexHookEventResult {
  event: RawCodexEvent;
  logLine: string;
  response: string;
}

function asString(value: unknown): string {
  return typeof value === "string" ? value : "";
}

/**
 * Codex hook payloads document `session_id`, `cwd`, `hook_event_name` (plus
 * `model` and event-specific fields we don't need here). No timestamp field
 * is documented, so the caller supplies "now" for testability.
 */
export function formatCodexHookEvent(
  payload: unknown,
  nowIso: string,
): FormatCodexHookEventResult {
  const record =
    typeof payload === "object" && payload !== null
      ? (payload as Record<string, unknown>)
      : {};

  const event: RawCodexEvent = {
    sessionId: asString(record.session_id),
    label: asString(record.cwd),
    type: asString(record.hook_event_name),
    timestamp: nowIso,
  };

  return {
    event,
    logLine: `${JSON.stringify(event)}\n`,
    response: JSON.stringify({ continue: true }),
  };
}
```

- [ ] **Step 7: Run the test to verify it passes**

Run: `cd tools/agent-status-bridge && pnpm test -- test/formatCodexHookEvent.test.ts`

Expected: PASS, all 4 tests green.

- [ ] **Step 8: Run the full suite and checks**

Run: `cd tools/agent-status-bridge && pnpm test && pnpm run check`

Expected: all tests pass, biome + tsc report no issues.

- [ ] **Step 9: Commit**

```bash
git add tools/agent-status-bridge/src/types/codexEvent.ts tools/agent-status-bridge/src/collectors/codexCollector.ts tools/agent-status-bridge/src/hooks/formatCodexHookEvent.ts tools/agent-status-bridge/test/formatCodexHookEvent.test.ts
git commit -m "feat: extract RawCodexEvent and add a pure Codex hook-payload formatter"
```

---

### Task 3: The real hook capture script

**Files:**
- Create: `tools/agent-status-bridge/scripts/codex-hook-capture.mjs`
- Modify: `tools/agent-status-bridge/.gitignore`
- Modify: `tools/agent-status-bridge/package.json`

**Interfaces:**
- Consumes: `formatCodexHookEvent` from the compiled `dist/src/hooks/formatCodexHookEvent.js` (built by Task 2's `pnpm run build`)
- Produces: appends one JSON line per invocation to `tools/agent-status-bridge/.runtime/codex-events.log`; the log file's line shape must stay identical to `RawCodexEvent` from Task 2 since `codexCollector.ts` parses it in Task 5.

- [ ] **Step 1: Build so the compiled formatter exists**

Run: `cd tools/agent-status-bridge && pnpm run build`

Expected: succeeds; `dist/src/hooks/formatCodexHookEvent.js` now exists.

- [ ] **Step 2: Write the capture script**

Create `tools/agent-status-bridge/scripts/codex-hook-capture.mjs`:

```js
#!/usr/bin/env node
// Invoked directly by Codex (see ../.codex/hooks.json at the repo root) once
// per hook event. Imports the compiled formatter so the field-extraction
// logic has a single source of truth (src/hooks/formatCodexHookEvent.ts) —
// run `pnpm run build` after changing that file, or this script keeps using
// the stale compiled output.

import { appendFileSync, mkdirSync } from "node:fs";
import { dirname, join } from "node:path";
import { formatCodexHookEvent } from "../dist/src/hooks/formatCodexHookEvent.js";

const LOG_PATH = join(import.meta.dirname, "..", ".runtime", "codex-events.log");

function readStdin() {
  const chunks = [];
  return new Promise((resolve) => {
    process.stdin.on("data", (chunk) => chunks.push(chunk));
    process.stdin.on("end", () => resolve(Buffer.concat(chunks).toString("utf8")));
  });
}

const raw = await readStdin();

let payload = null;
try {
  payload = JSON.parse(raw);
} catch {
  // Malformed payload: still respond below so Codex is never blocked, just skip logging.
}

if (payload !== null) {
  const { logLine, response } = formatCodexHookEvent(payload, new Date().toISOString());
  mkdirSync(dirname(LOG_PATH), { recursive: true });
  appendFileSync(LOG_PATH, logLine);
  process.stdout.write(response);
} else {
  process.stdout.write(JSON.stringify({ continue: true }));
}

process.exit(0);
```

- [ ] **Step 3: Manually verify the script end-to-end**

Run:

```bash
cd tools/agent-status-bridge
rm -f .runtime/codex-events.log
echo '{"session_id":"test-1","cwd":"/tmp","hook_event_name":"UserPromptSubmit"}' | node scripts/codex-hook-capture.mjs
cat .runtime/codex-events.log
```

Expected: the `echo`d command prints `{"continue":true}` with no trailing newline, and `.runtime/codex-events.log` contains one line:
`{"sessionId":"test-1","label":"/tmp","type":"UserPromptSubmit","timestamp":"<ISO timestamp near now>"}`

- [ ] **Step 4: Verify malformed input never blocks Codex**

Run:

```bash
cd tools/agent-status-bridge
echo 'not json' | node scripts/codex-hook-capture.mjs
echo "exit code: $?"
```

Expected: prints `{"continue":true}` then `exit code: 0`, and `.runtime/codex-events.log` gains no new line (still exactly the one line from Step 3).

- [ ] **Step 5: Ignore the runtime log directory**

In `tools/agent-status-bridge/.gitignore`, replace:

```
dist/
node_modules/
```

with:

```
dist/
node_modules/
.runtime/
```

- [ ] **Step 6: Include `scripts/` in lint/format coverage**

In `tools/agent-status-bridge/package.json`, replace:

```json
    "check": "biome check src test package.json tsconfig.json biome.json vitest.config.ts README.md .gitignore && tsc -p tsconfig.json --noEmit",
    "format": "biome format --write src test package.json tsconfig.json biome.json vitest.config.ts README.md .gitignore"
```

with:

```json
    "check": "biome check src test scripts package.json tsconfig.json biome.json vitest.config.ts README.md .gitignore && tsc -p tsconfig.json --noEmit",
    "format": "biome format --write src test scripts package.json tsconfig.json biome.json vitest.config.ts README.md .gitignore"
```

- [ ] **Step 7: Run checks**

Run: `cd tools/agent-status-bridge && pnpm run check`

Expected: biome reports no issues for `scripts/codex-hook-capture.mjs`; `tsc --noEmit` still passes (the script is plain `.mjs`, outside `tsconfig.json`'s `include`, so it is not type-checked — only linted/formatted).

- [ ] **Step 8: Commit**

```bash
git add tools/agent-status-bridge/scripts tools/agent-status-bridge/.gitignore tools/agent-status-bridge/package.json
git commit -m "feat: add the Codex hook capture script"
```

---

### Task 4: Register the repo-scoped Codex hooks

**Files:**
- Create: `.codex/hooks.json` (repo root, next to `AGENTS.md`)

**Interfaces:**
- Consumes: the absolute path to `tools/agent-status-bridge/scripts/codex-hook-capture.mjs` from Task 3
- Produces: Codex, when run inside this repo, invokes the capture script for every confirmed lifecycle event

- [ ] **Step 1: Write the repo-scoped hooks file**

Create `.codex/hooks.json` at the repository root:

```json
{
  "hooks": {
    "SessionStart": [
      {
        "matcher": ".*",
        "hooks": [
          {
            "type": "command",
            "command": "/Users/dmytrii.lysak/.nvm/versions/node/v24.15.0/bin/node /Users/Files/www/pet/esp32-ambient-matrix-lamp/tools/agent-status-bridge/scripts/codex-hook-capture.mjs",
            "timeout": 5
          }
        ]
      }
    ],
    "UserPromptSubmit": [
      {
        "matcher": ".*",
        "hooks": [
          {
            "type": "command",
            "command": "/Users/dmytrii.lysak/.nvm/versions/node/v24.15.0/bin/node /Users/Files/www/pet/esp32-ambient-matrix-lamp/tools/agent-status-bridge/scripts/codex-hook-capture.mjs",
            "timeout": 5
          }
        ]
      }
    ],
    "PreToolUse": [
      {
        "matcher": ".*",
        "hooks": [
          {
            "type": "command",
            "command": "/Users/dmytrii.lysak/.nvm/versions/node/v24.15.0/bin/node /Users/Files/www/pet/esp32-ambient-matrix-lamp/tools/agent-status-bridge/scripts/codex-hook-capture.mjs",
            "timeout": 5
          }
        ]
      }
    ],
    "PostToolUse": [
      {
        "matcher": ".*",
        "hooks": [
          {
            "type": "command",
            "command": "/Users/dmytrii.lysak/.nvm/versions/node/v24.15.0/bin/node /Users/Files/www/pet/esp32-ambient-matrix-lamp/tools/agent-status-bridge/scripts/codex-hook-capture.mjs",
            "timeout": 5
          }
        ]
      }
    ],
    "SubagentStart": [
      {
        "matcher": ".*",
        "hooks": [
          {
            "type": "command",
            "command": "/Users/dmytrii.lysak/.nvm/versions/node/v24.15.0/bin/node /Users/Files/www/pet/esp32-ambient-matrix-lamp/tools/agent-status-bridge/scripts/codex-hook-capture.mjs",
            "timeout": 5
          }
        ]
      }
    ],
    "SubagentStop": [
      {
        "matcher": ".*",
        "hooks": [
          {
            "type": "command",
            "command": "/Users/dmytrii.lysak/.nvm/versions/node/v24.15.0/bin/node /Users/Files/www/pet/esp32-ambient-matrix-lamp/tools/agent-status-bridge/scripts/codex-hook-capture.mjs",
            "timeout": 5
          }
        ]
      }
    ],
    "PreCompact": [
      {
        "matcher": ".*",
        "hooks": [
          {
            "type": "command",
            "command": "/Users/dmytrii.lysak/.nvm/versions/node/v24.15.0/bin/node /Users/Files/www/pet/esp32-ambient-matrix-lamp/tools/agent-status-bridge/scripts/codex-hook-capture.mjs",
            "timeout": 5
          }
        ]
      }
    ],
    "PostCompact": [
      {
        "matcher": ".*",
        "hooks": [
          {
            "type": "command",
            "command": "/Users/dmytrii.lysak/.nvm/versions/node/v24.15.0/bin/node /Users/Files/www/pet/esp32-ambient-matrix-lamp/tools/agent-status-bridge/scripts/codex-hook-capture.mjs",
            "timeout": 5
          }
        ]
      }
    ],
    "PermissionRequest": [
      {
        "matcher": ".*",
        "hooks": [
          {
            "type": "command",
            "command": "/Users/dmytrii.lysak/.nvm/versions/node/v24.15.0/bin/node /Users/Files/www/pet/esp32-ambient-matrix-lamp/tools/agent-status-bridge/scripts/codex-hook-capture.mjs",
            "timeout": 5
          }
        ]
      }
    ],
    "Stop": [
      {
        "matcher": ".*",
        "hooks": [
          {
            "type": "command",
            "command": "/Users/dmytrii.lysak/.nvm/versions/node/v24.15.0/bin/node /Users/Files/www/pet/esp32-ambient-matrix-lamp/tools/agent-status-bridge/scripts/codex-hook-capture.mjs",
            "timeout": 5
          }
        ]
      }
    ]
  }
}
```

- [ ] **Step 2: Validate the file is well-formed JSON**

Run: `node -e "JSON.parse(require('fs').readFileSync('.codex/hooks.json', 'utf8')); console.log('valid json')"`

Expected: prints `valid json`.

- [ ] **Step 3: Commit**

```bash
git add .codex/hooks.json
git commit -m "feat: register repo-scoped Codex hooks for the agent-status-bridge capture script"
```

- [ ] **Step 4: Note the manual trust-approval step (cannot be automated)**

Codex requires explicit trust review for non-managed hooks the first time they run, via the interactive `/hooks` command. This cannot be approved by an agent — the human operator must run `codex` interactively inside this repo at least once and approve the new hooks when prompted, before Task 6's end-to-end verification can pass. Record this as an open checkpoint; do not attempt to bypass or auto-approve it.

---

### Task 5: Wire the Codex collector into `main.ts`

**Files:**
- Modify: `tools/agent-status-bridge/src/main.ts`

**Interfaces:**
- Consumes: `createCodexCollector` and `CodexCollectorDeps` from `tools/agent-status-bridge/src/collectors/codexCollector.ts` (existing, unchanged signature); `aggregateFamilyStatus` and `startFamilyStatusLoop` (existing, unchanged)
- Produces: a running `codex_status: ...` poll loop identical in shape to the existing `claude_status` one

- [ ] **Step 1: Replace `main.ts`**

Replace the full contents of `tools/agent-status-bridge/src/main.ts` with:

```ts
import { readFile } from "node:fs/promises";
import { join } from "node:path";
import { aggregateFamilyStatus } from "./aggregators/aggregateFamilyStatus.js";
import {
  createClaudeCollector,
  runClaudeAgentsJson,
} from "./collectors/claudeCollector.js";
import { createCodexCollector } from "./collectors/codexCollector.js";
import { startFamilyStatusLoop } from "./runtime/familyStatusLoop.js";

const CLAUDE_POLL_INTERVAL_MS = 2000;
const CODEX_POLL_INTERVAL_MS = 2000;
const CODEX_EVENT_LOG_PATH = join(
  import.meta.dirname,
  "..",
  ".runtime",
  "codex-events.log",
);

async function readCodexEventLog(): Promise<string> {
  try {
    return await readFile(CODEX_EVENT_LOG_PATH, "utf8");
  } catch (error) {
    if ((error as NodeJS.ErrnoException).code === "ENOENT") {
      return "";
    }
    throw error;
  }
}

const claudeCollector = createClaudeCollector({ runClaudeAgentsJson });
const codexCollector = createCodexCollector({ readCodexEventLog });

startFamilyStatusLoop({
  collector: claudeCollector,
  aggregate: aggregateFamilyStatus,
  intervalMs: CLAUDE_POLL_INTERVAL_MS,
  onStatus: (status) => {
    console.log(`claude_status: ${status}`);
  },
});

startFamilyStatusLoop({
  collector: codexCollector,
  aggregate: aggregateFamilyStatus,
  intervalMs: CODEX_POLL_INTERVAL_MS,
  onStatus: (status) => {
    console.log(`codex_status: ${status}`);
  },
});

console.log(
  "agent-status-bridge is running; Home Assistant publisher is not implemented yet",
);
```

- [ ] **Step 2: Run checks**

Run: `cd tools/agent-status-bridge && pnpm test && pnpm run check && pnpm run build`

Expected: all tests pass, biome + tsc clean, build succeeds (this confirms `main.ts` compiles and type-checks; it has no dedicated test file, matching the existing pattern where `main.ts` is the composition root and is verified by running it, not by unit tests).

- [ ] **Step 3: Smoke-test the loop manually**

Run: `cd tools/agent-status-bridge && timeout 6 pnpm dev || true`

Expected: within a few seconds you see at least one `claude_status: ...` line (real, from any running Claude Code sessions) and one `codex_status: idle` line (real, read from `.runtime/codex-events.log` — `idle` if the file is empty or missing, matching `aggregateFamilyStatus([])` returning `"idle"`).

- [ ] **Step 4: Commit**

```bash
git add tools/agent-status-bridge/src/main.ts
git commit -m "feat: wire the Codex collector into the bridge's status loop"
```

---

### Task 6: Documentation and end-to-end verification

**Files:**
- Modify: `tools/agent-status-bridge/README.md`

**Interfaces:**
- Consumes: nothing new
- Produces: updated scope description; no code interfaces

- [ ] **Step 1: Update the README's "Current scope" and "Notes" sections**

In `tools/agent-status-bridge/README.md`, replace:

```
- live `Claude Code` collector using `claude agents --json --all`
- a family status poll loop wired into `main.ts` for `Claude Code`
- a pure `Codex` collector and normalizer, not yet wired into `main.ts`
```

with:

```
- live `Claude Code` collector using `claude agents --json --all`
- live `Codex` collector reading a real hook event log written by
  `scripts/codex-hook-capture.mjs`
- a family status poll loop wired into `main.ts` for both `Claude Code` and
  `Codex`
```

Then, in the same file, replace:

```
This scaffold does not include yet:

- the real hook script that writes the `Codex` event log (currently only the
  collector's read/parse side exists, exercised via injected test doubles)
- wiring the `Codex` collector into `main.ts`'s status loop and aggregator
- `Home Assistant` publishing implementation
```

with:

```
This scaffold does not include yet:

- `Home Assistant` publishing implementation

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
```

- [ ] **Step 2 (MANUAL — requires a live Codex session, do not attempt as a subagent): End-to-end verification with a real Codex session**

Run, inside the repository root (not inside `tools/agent-status-bridge/`):

```bash
codex exec "Reply with exactly the word OK and do nothing else."
```

Expected: Codex either prompts for hook trust approval (if this is the first repo-scoped hook run — stop and hand this back to the human operator per Task 4 Step 4) or runs cleanly. Either way, check the log afterward:

```bash
cat tools/agent-status-bridge/.runtime/codex-events.log
```

Expected: one or more JSON lines with `"type"` values from the confirmed vocabulary (at minimum `UserPromptSubmit` and `Stop`), non-empty `sessionId`, and recent `timestamp` values.

- [ ] **Step 3 (MANUAL — depends on Step 2's real log data, do not attempt as a subagent): Confirm the bridge reflects the real event**

Run: `cd tools/agent-status-bridge && timeout 6 pnpm dev || true`

Expected: a `codex_status: idle` line (since the `Stop` event from Step 2 already landed and the session finished) proving the full pipeline — real Codex hook -> capture script -> log file -> `codexCollector` -> `aggregateFamilyStatus` -> `main.ts` -> console — is working, matching what `claude_status` already does.

- [ ] **Step 4: Commit**

```bash
git add tools/agent-status-bridge/README.md
git commit -m "docs: document the live Codex hook pipeline"
```
