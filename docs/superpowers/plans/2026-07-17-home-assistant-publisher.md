# Home Assistant Publisher Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make `tools/agent-status-bridge/` actually publish a single aggregated status into Home Assistant's `input_select.agent_status`, instead of only logging to the console, per `docs/agent-status-lamp/home-assistant-mvp-plan.md`.

**Architecture:** The bridge already computes per-family statuses (`claude_status`, `codex_status`) and has a tested `aggregateGlobalStatus` pure function that combines them by priority. This plan adds: (1) a small, isolated mapping table translating the bridge's internal `NormalizedStatus` vocabulary (`idle`/`thinking`/`needs_user`/`done`/`error`) into whatever Home Assistant's `input_select` expects — kept in its own file so either vocabulary can grow later without touching the other; (2) a Home Assistant REST API publisher behind the same dependency-injection pattern already used for `claudeCollector`/`codexCollector` (inject the network call, so the pure wiring logic is unit-testable); (3) `main.ts` wiring that tracks both family statuses, recomputes the global status whenever either changes, and publishes only on change — mirroring the existing per-family debounce in `familyStatusLoop.ts`. Home Assistant connection details (URL, token) come from a gitignored `.env` file, loaded with Node's built-in `process.loadEnvFile()` — no new dependency.

**Tech Stack:** Node.js 24 (built-in `fetch` and `process.loadEnvFile`), TypeScript 7, pnpm, tsx, Vitest, Biome (existing `tools/agent-status-bridge/` package).

## Global Constraints

- Home Assistant states for this MVP are exactly these 4, per the user's explicit choice: `inactive`, `thinking`, `needs_user`, `error`. `done` is not yet produced by any collector — map it to `inactive` for now (documented as provisional), not a new HA state. Do not add a 5th HA state in this plan.
- The mapping between `NormalizedStatus` and the HA option string must live in exactly one file (`src/publishers/mapToHomeAssistantStatus.ts`) as a `Record` lookup — this is the deliberate SRP seam the user asked for so either vocabulary can be extended later by editing only this table.
- Connection details (`HA_URL`, `HA_TOKEN`) come from a `.env` file loaded via `process.loadEnvFile()` — no `dotenv` package, no new dependency. `.env` itself must be gitignored; commit only `.env.example` as a template.
- If `HA_URL`/`HA_TOKEN` are not set, the bridge must keep running exactly as it does today (console-log only, no crash, no publish attempt) — Home Assistant publishing is additive, never a hard requirement to run the bridge locally.
- A failed or unreachable Home Assistant call must never crash the bridge process — catch and log, never let a rejected publish become an unhandled rejection inside a `setInterval` callback. (A previous task in this project's history hit exactly this class of bug in a different code path — same discipline applies here.)
- This bridge publishes ONE aggregated global status (`aggregateGlobalStatus`), not one entity per family — matches `home-assistant-mvp-plan.md`'s "one logical state carrier" recommendation. Do not create per-family Home Assistant entities in this plan.
- Creating the actual `input_select.agent_status` entity inside the user's live Home Assistant instance is a manual, human-only step (editing their HA config, outside this repo) — this plan only produces the YAML for them to paste in; it does not and cannot apply it.
- All code, identifiers, comments, and commit messages in English.
- No `Co-Authored-By: Claude` trailer in any commit message — this repository's owner does not want that in git history.
- **Git staging discipline:** this repository's working tree has many unrelated pre-existing staged/modified files belonging to a different, unrelated feature. Every commit in this plan must `git add` only the exact files that task names — never `git add -A`, `git add .`, or `git commit -a`. Before each commit, run `git status --short` and confirm the staged list is exactly the task's files; after each commit, run `git show --stat HEAD` and confirm the same.
- Use the repository's existing pnpm scripts (`pnpm test`, `pnpm run check`, `pnpm run build`) inside `tools/agent-status-bridge/` to verify each task.

---

### Task 1: The status-mapping seam

**Files:**
- Create: `tools/agent-status-bridge/src/publishers/mapToHomeAssistantStatus.ts`
- Test: `tools/agent-status-bridge/test/mapToHomeAssistantStatus.test.ts`

**Interfaces:**
- Consumes: `NormalizedStatus` from `tools/agent-status-bridge/src/types/status.ts` (unchanged)
- Produces: `mapToHomeAssistantStatus(status: NormalizedStatus): string`

- [ ] **Step 1: Write the failing test**

Create `tools/agent-status-bridge/test/mapToHomeAssistantStatus.test.ts`:

```ts
import { describe, expect, it } from "vitest";
import { mapToHomeAssistantStatus } from "../src/publishers/mapToHomeAssistantStatus.js";

describe("mapToHomeAssistantStatus", () => {
  it("maps 'idle' to 'inactive'", () => {
    expect(mapToHomeAssistantStatus("idle")).toBe("inactive");
  });

  it("maps 'thinking' to 'thinking'", () => {
    expect(mapToHomeAssistantStatus("thinking")).toBe("thinking");
  });

  it("maps 'needs_user' to 'needs_user'", () => {
    expect(mapToHomeAssistantStatus("needs_user")).toBe("needs_user");
  });

  it("maps 'error' to 'error'", () => {
    expect(mapToHomeAssistantStatus("error")).toBe("error");
  });

  it("maps the not-yet-produced 'done' to 'inactive' as a provisional default", () => {
    expect(mapToHomeAssistantStatus("done")).toBe("inactive");
  });
});
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `cd tools/agent-status-bridge && pnpm test -- test/mapToHomeAssistantStatus.test.ts`

Expected: FAIL — `Cannot find module '../src/publishers/mapToHomeAssistantStatus.js'`.

- [ ] **Step 3: Write the minimal implementation**

Create `tools/agent-status-bridge/src/publishers/mapToHomeAssistantStatus.ts`:

```ts
import type { NormalizedStatus } from "../types/status.js";

/**
 * Isolates the bridge's internal NormalizedStatus vocabulary from whatever
 * Home Assistant's `input_select.agent_status` entity expects, so either
 * side can grow independently — e.g. splitting "done" into its own HA
 * state later means editing only this table, nothing else in the bridge.
 *
 * "done" is not yet produced by any collector; treated as "inactive" until
 * a future HA state (e.g. "unread") is added for it.
 */
const STATUS_TO_HOME_ASSISTANT_OPTION: Record<NormalizedStatus, string> = {
  idle: "inactive",
  thinking: "thinking",
  needs_user: "needs_user",
  done: "inactive",
  error: "error",
};

export function mapToHomeAssistantStatus(status: NormalizedStatus): string {
  return STATUS_TO_HOME_ASSISTANT_OPTION[status];
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run: `cd tools/agent-status-bridge && pnpm test -- test/mapToHomeAssistantStatus.test.ts`

Expected: PASS, all 5 tests green.

- [ ] **Step 5: Run the full suite to check for regressions**

Run: `cd tools/agent-status-bridge && pnpm test`

Expected: all test files pass.

- [ ] **Step 6: Commit**

```bash
git add tools/agent-status-bridge/src/publishers/mapToHomeAssistantStatus.ts tools/agent-status-bridge/test/mapToHomeAssistantStatus.test.ts
git commit -m "feat: add the NormalizedStatus to Home Assistant option mapping"
```

---

### Task 2: The Home Assistant publisher

**Files:**
- Create: `tools/agent-status-bridge/src/publishers/homeAssistantPublisher.ts`
- Test: `tools/agent-status-bridge/test/homeAssistantPublisher.test.ts`

**Interfaces:**
- Consumes: `mapToHomeAssistantStatus` from Task 1; `NormalizedStatus` from `types/status.ts`
- Produces:
  - `createHomeAssistantPublisher(deps: HomeAssistantPublisherDeps): { publishGlobalStatus(status: NormalizedStatus): Promise<void> }`
  - `HomeAssistantPublisherDeps = { entityId: string; setOption: (entityId: string, option: string) => Promise<void> }`
  - `setInputSelectOption(baseUrl: string, token: string, entityId: string, option: string): Promise<void>` — the real network call, used by `main.ts` in a later task, not unit-tested directly (matches the existing precedent of `runClaudeAgentsJson`/`runClaudeAgentsJson`-style real IO functions in this codebase — only the logic around them is unit-tested via injected fakes).

- [ ] **Step 1: Write the failing test**

Create `tools/agent-status-bridge/test/homeAssistantPublisher.test.ts`:

```ts
import { describe, expect, it } from "vitest";
import { createHomeAssistantPublisher } from "../src/publishers/homeAssistantPublisher.js";

describe("createHomeAssistantPublisher", () => {
  it("maps the status and calls setOption with the configured entity id", async () => {
    const calls: Array<{ entityId: string; option: string }> = [];

    const publisher = createHomeAssistantPublisher({
      entityId: "input_select.agent_status",
      setOption: async (entityId, option) => {
        calls.push({ entityId, option });
      },
    });

    await publisher.publishGlobalStatus("needs_user");

    expect(calls).toEqual([
      { entityId: "input_select.agent_status", option: "needs_user" },
    ]);
  });

  it("maps 'idle' to the 'inactive' HA option before calling setOption", async () => {
    const calls: Array<{ entityId: string; option: string }> = [];

    const publisher = createHomeAssistantPublisher({
      entityId: "input_select.agent_status",
      setOption: async (entityId, option) => {
        calls.push({ entityId, option });
      },
    });

    await publisher.publishGlobalStatus("idle");

    expect(calls).toEqual([
      { entityId: "input_select.agent_status", option: "inactive" },
    ]);
  });

  it("propagates a rejection from setOption to the caller", async () => {
    const publisher = createHomeAssistantPublisher({
      entityId: "input_select.agent_status",
      setOption: async () => {
        throw new Error("network down");
      },
    });

    await expect(publisher.publishGlobalStatus("error")).rejects.toThrow(
      "network down",
    );
  });
});
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `cd tools/agent-status-bridge && pnpm test -- test/homeAssistantPublisher.test.ts`

Expected: FAIL — `Cannot find module '../src/publishers/homeAssistantPublisher.js'`.

- [ ] **Step 3: Write the minimal implementation**

Create `tools/agent-status-bridge/src/publishers/homeAssistantPublisher.ts`:

```ts
import { mapToHomeAssistantStatus } from "./mapToHomeAssistantStatus.js";
import type { NormalizedStatus } from "../types/status.js";

export interface HomeAssistantPublisherDeps {
  entityId: string;
  setOption: (entityId: string, option: string) => Promise<void>;
}

export interface HomeAssistantPublisher {
  publishGlobalStatus(status: NormalizedStatus): Promise<void>;
}

export function createHomeAssistantPublisher(
  deps: HomeAssistantPublisherDeps,
): HomeAssistantPublisher {
  return {
    async publishGlobalStatus(status: NormalizedStatus) {
      const option = mapToHomeAssistantStatus(status);
      await deps.setOption(deps.entityId, option);
    },
  };
}

/**
 * Calls Home Assistant's REST API to set an `input_select` entity's current
 * option. Not unit-tested directly — it is a thin wrapper around the
 * platform `fetch`, exercised via the `main.ts` manual verification step
 * against a real Home Assistant instance instead.
 */
export async function setInputSelectOption(
  baseUrl: string,
  token: string,
  entityId: string,
  option: string,
): Promise<void> {
  const response = await fetch(
    `${baseUrl}/api/services/input_select/select_option`,
    {
      method: "POST",
      headers: {
        Authorization: `Bearer ${token}`,
        "Content-Type": "application/json",
      },
      body: JSON.stringify({ entity_id: entityId, option }),
    },
  );

  if (!response.ok) {
    throw new Error(
      `Home Assistant request failed: ${response.status} ${response.statusText}`,
    );
  }
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run: `cd tools/agent-status-bridge && pnpm test -- test/homeAssistantPublisher.test.ts`

Expected: PASS, all 3 tests green.

- [ ] **Step 5: Run the full suite and checks**

Run: `cd tools/agent-status-bridge && pnpm test && pnpm run check`

Expected: all tests pass, biome + tsc clean.

- [ ] **Step 6: Commit**

```bash
git add tools/agent-status-bridge/src/publishers/homeAssistantPublisher.ts tools/agent-status-bridge/test/homeAssistantPublisher.test.ts
git commit -m "feat: add the Home Assistant input_select publisher"
```

---

### Task 3: Env template, gitignore, and the Home Assistant YAML for the user

**Files:**
- Create: `tools/agent-status-bridge/.env.example`
- Modify: `tools/agent-status-bridge/.gitignore`
- Create: `docs/agent-status-lamp/home-assistant-input-select.yaml`
- Modify: `tools/agent-status-bridge/package.json`

**Interfaces:**
- Consumes: nothing from earlier tasks
- Produces: the exact environment variable names `main.ts` (Task 4) reads: `HA_URL`, `HA_TOKEN`, `HA_ENTITY_ID`

- [ ] **Step 1: Write the env template**

Create `tools/agent-status-bridge/.env.example`:

```
# Copy this file to .env and fill in your own values. .env is gitignored.

# Base URL of your Home Assistant instance, no trailing slash.
HA_URL=http://homeassistant.local:8123

# A long-lived access token, created in Home Assistant under your profile.
HA_TOKEN=

# Optional: override the entity id if you named it differently.
HA_ENTITY_ID=input_select.agent_status
```

- [ ] **Step 2: Ignore the real `.env`**

In `tools/agent-status-bridge/.gitignore`, replace:

```
dist/
node_modules/
.runtime/
```

with:

```
dist/
node_modules/
.runtime/
.env
```

- [ ] **Step 3: Write the Home Assistant YAML for the user to add**

Create `docs/agent-status-lamp/home-assistant-input-select.yaml`:

```yaml
# Paste this into Home Assistant's configuration.yaml under an `input_select:`
# key (or merge it into an existing one), then restart Home Assistant or
# reload input_select entities from Developer Tools -> YAML.
input_select:
  agent_status:
    name: Agent Status
    options:
      - inactive
      - thinking
      - needs_user
      - error
    initial: inactive
    icon: mdi:robot
```

- [ ] **Step 4: Include `.env.example` in lint/format coverage**

In `tools/agent-status-bridge/package.json`, replace:

```json
    "check": "biome check src test scripts package.json tsconfig.json biome.json vitest.config.ts README.md .gitignore && tsc -p tsconfig.json --noEmit",
    "format": "biome format --write src test scripts package.json tsconfig.json biome.json vitest.config.ts README.md .gitignore"
```

with:

```json
    "check": "biome check src test scripts package.json tsconfig.json biome.json vitest.config.ts README.md .gitignore .env.example && tsc -p tsconfig.json --noEmit",
    "format": "biome format --write src test scripts package.json tsconfig.json biome.json vitest.config.ts README.md .gitignore .env.example"
```

- [ ] **Step 5: Run checks**

Run: `cd tools/agent-status-bridge && pnpm run check`

Expected: clean (biome may reformat `.env.example`'s trailing newline; that's fine).

- [ ] **Step 6: Commit**

```bash
git add tools/agent-status-bridge/.env.example tools/agent-status-bridge/.gitignore tools/agent-status-bridge/package.json docs/agent-status-lamp/home-assistant-input-select.yaml
git commit -m "feat: add Home Assistant env template and input_select YAML"
```

---

### Task 4: Wire the publisher into `main.ts`

**Files:**
- Modify: `tools/agent-status-bridge/src/main.ts`

**Interfaces:**
- Consumes: `aggregateGlobalStatus` (existing, unchanged), `createHomeAssistantPublisher`/`setInputSelectOption` from Task 2, `HA_URL`/`HA_TOKEN`/`HA_ENTITY_ID` env vars from Task 3
- Produces: a running bridge that logs `global_status: ...` and publishes it to Home Assistant when `HA_URL`/`HA_TOKEN` are set, and otherwise runs exactly as before (console-log only)

- [ ] **Step 1: Replace `main.ts`**

Replace the full contents of `tools/agent-status-bridge/src/main.ts` with:

```ts
import { readFile } from "node:fs/promises";
import { join } from "node:path";
import { aggregateFamilyStatus } from "./aggregators/aggregateFamilyStatus.js";
import { aggregateGlobalStatus } from "./aggregators/aggregateGlobalStatus.js";
import {
  createClaudeCollector,
  runClaudeAgentsJson,
} from "./collectors/claudeCollector.js";
import { createCodexCollector } from "./collectors/codexCollector.js";
import {
  createHomeAssistantPublisher,
  setInputSelectOption,
} from "./publishers/homeAssistantPublisher.js";
import { startFamilyStatusLoop } from "./runtime/familyStatusLoop.js";
import type { NormalizedStatus } from "./types/status.js";

try {
  process.loadEnvFile();
} catch {
  // .env is optional: the bridge must run without Home Assistant configured.
}

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

const haUrl = process.env.HA_URL;
const haToken = process.env.HA_TOKEN;
const haEntityId = process.env.HA_ENTITY_ID ?? "input_select.agent_status";

const publisher =
  haUrl && haToken
    ? createHomeAssistantPublisher({
        entityId: haEntityId,
        setOption: (entityId, option) =>
          setInputSelectOption(haUrl, haToken, entityId, option),
      })
    : undefined;

if (publisher) {
  console.log(`Home Assistant publishing enabled: ${haEntityId} at ${haUrl}`);
} else {
  console.log(
    "HA_URL/HA_TOKEN not set: Home Assistant publishing disabled, logging only",
  );
}

let latestClaudeStatus: NormalizedStatus = "idle";
let latestCodexStatus: NormalizedStatus = "idle";
let lastPublishedGlobalStatus: NormalizedStatus | undefined;

function publishGlobalStatusIfChanged(): void {
  const globalStatus = aggregateGlobalStatus([
    latestClaudeStatus,
    latestCodexStatus,
  ]);

  if (globalStatus === lastPublishedGlobalStatus) {
    return;
  }

  lastPublishedGlobalStatus = globalStatus;
  console.log(`global_status: ${globalStatus}`);

  if (publisher) {
    publisher.publishGlobalStatus(globalStatus).catch((error: unknown) => {
      console.error("Home Assistant publish failed:", error);
    });
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
    latestClaudeStatus = status;
    publishGlobalStatusIfChanged();
  },
});

startFamilyStatusLoop({
  collector: codexCollector,
  aggregate: aggregateFamilyStatus,
  intervalMs: CODEX_POLL_INTERVAL_MS,
  onStatus: (status) => {
    console.log(`codex_status: ${status}`);
    latestCodexStatus = status;
    publishGlobalStatusIfChanged();
  },
});

console.log("agent-status-bridge is running");
```

Note: `publishGlobalStatusIfChanged` is synchronous and never `await`s the publish call — it attaches a `.catch()` directly to the returned promise so a rejected publish is always handled and can never become an unhandled promise rejection (which would crash the whole process, killing both status loops). This mirrors the lesson learned from a prior unhandled-rejection bug in this project's collector code.

- [ ] **Step 2: Run checks**

Run: `cd tools/agent-status-bridge && pnpm test && pnpm run check && pnpm run build`

Expected: all tests pass, biome + tsc clean, build succeeds. (`main.ts` has no dedicated test file, matching the existing pattern — it's the composition root, verified by building and running it.)

- [ ] **Step 3: Smoke-test without Home Assistant configured (regression check)**

Run: `cd tools/agent-status-bridge && timeout 6 pnpm dev || true`

Note: `timeout` is a GNU coreutils command and may not exist on this Mac by default (a prior task in this project hit exactly this — no `timeout`/`gtimeout` binary available). If the command errors with "command not found", instead run `pnpm dev` directly in the background, wait ~5 seconds, capture its output, then stop it — for example:

```bash
cd tools/agent-status-bridge
pnpm dev > /tmp/ha-publisher-smoke-1.log 2>&1 &
DEV_PID=$!
sleep 5
kill "$DEV_PID" 2>/dev/null
cat /tmp/ha-publisher-smoke-1.log
```

Expected: prints `HA_URL/HA_TOKEN not set: Home Assistant publishing disabled, logging only`, then the usual `claude_status`/`codex_status`/`global_status` lines — exactly as before, no crash, no attempted network call.

- [ ] **Step 4: Smoke-test with a deliberately wrong Home Assistant URL (failure-handling check)**

Run (or use the same background/`kill` fallback as Step 3 if `timeout` is unavailable, exporting `HA_URL`/`HA_TOKEN` in that shell first):

```bash
cd tools/agent-status-bridge
HA_URL=http://localhost:1 HA_TOKEN=fake timeout 6 pnpm dev || true
```

Expected: prints `Home Assistant publishing enabled: ...`, then once a status changes, a `Home Assistant publish failed: ...` error line on a failed connection — but the process keeps running and printing `claude_status`/`codex_status` lines afterward; it must not crash or exit early.

- [ ] **Step 5: Commit**

```bash
git add tools/agent-status-bridge/src/main.ts
git commit -m "feat: publish the aggregated global status to Home Assistant"
```

---

### Task 5: Documentation

**Files:**
- Modify: `tools/agent-status-bridge/README.md`

**Interfaces:**
- Consumes: nothing new
- Produces: updated scope description; no code interfaces

- [ ] **Step 1: Update the README**

In `tools/agent-status-bridge/README.md`, replace:

```
- `Home Assistant` publishing implementation
```

with:

```
- `Home Assistant` publishing: a single aggregated `global_status` (via
  `aggregateGlobalStatus`) is published to an `input_select` entity through
  Home Assistant's REST API, whenever it changes
```

Then, at the end of the file (after the existing `## Notes` section), add:

```

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
```

- [ ] **Step 2: Manual end-to-end verification against a real Home Assistant instance (requires your own HA instance and cannot be done by an agent)**

After completing the Home Assistant setup steps above, run `pnpm dev` and change a Codex/Claude session's state (e.g. submit a prompt, let it finish) and confirm `input_select.agent_status` visibly updates in Home Assistant's UI, matching the `global_status` line printed to the console.

- [ ] **Step 3: Commit**

```bash
git add tools/agent-status-bridge/README.md
git commit -m "docs: document the Home Assistant publisher setup"
```
