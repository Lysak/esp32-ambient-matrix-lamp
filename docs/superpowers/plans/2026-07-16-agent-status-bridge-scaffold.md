# Agent Status Bridge Scaffold Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Create the initial `tools/agent-status-bridge/` workspace with pinned runtime/tooling, core TypeScript interfaces, aggregation logic, and tests, without implementing live Codex or Claude collectors yet.

**Architecture:** Build a small Node.js service package with clear seams: source-agnostic types, a pure aggregation layer, stub collector interfaces, and a future publisher boundary. Keep runtime code separate from test fixtures and keep collector logic behind interfaces so official Codex and Claude integrations can be added later without restructuring the package.

**Tech Stack:** Node.js 24, TypeScript 7, pnpm, tsx, Vitest, Biome

## Global Constraints

- Use `tools/agent-status-bridge/` as the code root.
- Use `Node.js 24`, `TypeScript 7`, `pnpm`, `tsx`, `vitest`, and `biome`.
- Do not use `pm2`.
- Do not use `AppleScript` as the main implementation.
- Do not use direct SQLite reads as the primary `Codex` integration path.
- Keep the bridge isolated from ESPHome internals and lamp effect logic.
- Normalize bridge-visible status values to exactly `idle`, `thinking`, `needs_user`, `done`, and `error`.
- Preserve low coupling with the shape `collectors -> normalizers -> aggregators -> publishers`.

---

### Task 1: Scaffold the package and tooling

**Files:**
- Create: `tools/agent-status-bridge/package.json`
- Create: `tools/agent-status-bridge/pnpm-lock.yaml`
- Create: `tools/agent-status-bridge/tsconfig.json`
- Create: `tools/agent-status-bridge/biome.json`
- Create: `tools/agent-status-bridge/vitest.config.ts`
- Create: `tools/agent-status-bridge/.gitignore`
- Create: `tools/agent-status-bridge/README.md`

**Interfaces:**
- Consumes: none
- Produces: local package scripts `dev`, `build`, `test`, `check`, `format`

- [ ] **Step 1: Write the failing package-shape test**

Create `tools/agent-status-bridge/test/package-shape.test.ts`:

```ts
import { describe, expect, it } from "vitest";
import packageJson from "../package.json";

describe("package shape", () => {
  it("pins the expected runtime and tooling scripts", () => {
    expect(packageJson.engines.node).toBe("24.x");
    expect(packageJson.scripts.dev).toBe("tsx watch src/main.ts");
    expect(packageJson.scripts.build).toBe("tsc -p tsconfig.json");
    expect(packageJson.scripts.test).toBe("vitest run");
    expect(packageJson.scripts.check).toBe("biome check . && tsc -p tsconfig.json --noEmit");
  });
});
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd tools/agent-status-bridge && pnpm test -- --runInBand test/package-shape.test.ts`

Expected: fail because `package.json` and test dependencies do not exist yet.

- [ ] **Step 3: Write minimal package and tooling files**

Create the package files with the exact scripts and pinned runtime from the spec.

- [ ] **Step 4: Install dependencies and run the test to verify it passes**

Run: `cd tools/agent-status-bridge && pnpm install && pnpm test -- test/package-shape.test.ts`

Expected: pass with one passing test.

- [ ] **Step 5: Commit**

```bash
git add tools/agent-status-bridge docs/superpowers/plans/2026-07-16-agent-status-bridge-scaffold.md
git commit -m "feat: scaffold agent status bridge package"
```

### Task 2: Define shared types and collector/publisher interfaces

**Files:**
- Create: `tools/agent-status-bridge/src/types/status.ts`
- Create: `tools/agent-status-bridge/src/types/source.ts`
- Create: `tools/agent-status-bridge/src/collectors/types.ts`
- Create: `tools/agent-status-bridge/src/publishers/types.ts`
- Create: `tools/agent-status-bridge/src/main.ts`
- Test: `tools/agent-status-bridge/test/types.test.ts`

**Interfaces:**
- Consumes: package tooling from Task 1
- Produces:
  - `AgentFamily`
  - `NormalizedStatus`
  - `SessionLifecycleState`
  - `SourceSessionSnapshot`
  - `Collector<TSnapshot>`
  - `StatusPublisher`

- [ ] **Step 1: Write the failing type contract test**

Create `tools/agent-status-bridge/test/types.test.ts`:

```ts
import { describe, expect, it } from "vitest";
import { AGENT_FAMILIES, NORMALIZED_STATUSES } from "../src/types/status";

describe("type contracts", () => {
  it("exports the fixed agent families", () => {
    expect(AGENT_FAMILIES).toEqual(["codex", "claude"]);
  });

  it("exports the fixed normalized statuses", () => {
    expect(NORMALIZED_STATUSES).toEqual([
      "idle",
      "thinking",
      "needs_user",
      "done",
      "error",
    ]);
  });
});
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd tools/agent-status-bridge && pnpm test -- test/types.test.ts`

Expected: fail because the exported symbols do not exist yet.

- [ ] **Step 3: Write minimal shared types and a no-op main entrypoint**

Implement the exact exported constants and types the tests expect, plus a `main.ts` that only logs that the bridge scaffold is not configured yet.

- [ ] **Step 4: Run tests to verify they pass**

Run: `cd tools/agent-status-bridge && pnpm test -- test/types.test.ts`

Expected: pass with both tests green.

- [ ] **Step 5: Commit**

```bash
git add tools/agent-status-bridge
git commit -m "feat: add bridge type contracts"
```

### Task 3: Implement the pure aggregation layer with tests

**Files:**
- Create: `tools/agent-status-bridge/src/aggregators/aggregateFamilyStatus.ts`
- Create: `tools/agent-status-bridge/src/aggregators/aggregateGlobalStatus.ts`
- Test: `tools/agent-status-bridge/test/aggregateFamilyStatus.test.ts`
- Test: `tools/agent-status-bridge/test/aggregateGlobalStatus.test.ts`

**Interfaces:**
- Consumes:
  - `AgentFamily`
  - `NormalizedStatus`
  - `SourceSessionSnapshot`
- Produces:
  - `aggregateFamilyStatus(snapshots: SourceSessionSnapshot[]): NormalizedStatus`
  - `aggregateGlobalStatus(statuses: NormalizedStatus[]): NormalizedStatus`

- [ ] **Step 1: Write the failing family aggregation test**

Create `tools/agent-status-bridge/test/aggregateFamilyStatus.test.ts`:

```ts
import { describe, expect, it } from "vitest";
import { aggregateFamilyStatus } from "../src/aggregators/aggregateFamilyStatus";
import type { SourceSessionSnapshot } from "../src/types/source";

const makeSnapshot = (status: SourceSessionSnapshot["status"]): SourceSessionSnapshot => ({
  family: "codex",
  sourceSessionId: crypto.randomUUID(),
  label: status,
  status,
  updatedAt: "2026-07-16T00:00:00.000Z",
});

describe("aggregateFamilyStatus", () => {
  it("returns idle for no sessions", () => {
    expect(aggregateFamilyStatus([])).toBe("idle");
  });

  it("prefers error over every other status", () => {
    expect(
      aggregateFamilyStatus([
        makeSnapshot("thinking"),
        makeSnapshot("needs_user"),
        makeSnapshot("error"),
      ]),
    ).toBe("error");
  });

  it("prefers needs_user over thinking and done", () => {
    expect(
      aggregateFamilyStatus([
        makeSnapshot("thinking"),
        makeSnapshot("done"),
        makeSnapshot("needs_user"),
      ]),
    ).toBe("needs_user");
  });
});
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd tools/agent-status-bridge && pnpm test -- test/aggregateFamilyStatus.test.ts`

Expected: fail because the aggregator does not exist yet.

- [ ] **Step 3: Write minimal aggregation code**

Implement fixed priority ordering:

```ts
const STATUS_PRIORITY = ["idle", "done", "thinking", "needs_user", "error"] as const;
```

and return the highest-priority status present.

- [ ] **Step 4: Write and run the global aggregation test**

Create `tools/agent-status-bridge/test/aggregateGlobalStatus.test.ts` with the same priority semantics over family-level statuses, then run:

`cd tools/agent-status-bridge && pnpm test -- test/aggregateFamilyStatus.test.ts test/aggregateGlobalStatus.test.ts`

Expected: both test files pass.

- [ ] **Step 5: Commit**

```bash
git add tools/agent-status-bridge
git commit -m "feat: add bridge aggregation logic"
```

### Task 4: Document the scaffold usage and verify the workspace

**Files:**
- Modify: `tools/agent-status-bridge/README.md`

**Interfaces:**
- Consumes: package scripts and exported modules from Tasks 1-3
- Produces: setup and verification instructions for future bridge implementation

- [ ] **Step 1: Write the failing verification step**

Run:

```bash
cd tools/agent-status-bridge
pnpm check
pnpm test
pnpm build
```

Expected: one or more commands fail until the README, imports, and scripts are fully aligned.

- [ ] **Step 2: Update the README with exact local commands**

Document:

- required Node version
- `pnpm install`
- `pnpm dev`
- `pnpm test`
- `pnpm check`
- `pnpm build`
- current non-goal: no live collectors yet

- [ ] **Step 3: Run verification again**

Run:

```bash
cd tools/agent-status-bridge
pnpm check
pnpm test
pnpm build
```

Expected: all commands pass cleanly.

- [ ] **Step 4: Review git diff for scope**

Run: `git diff -- tools/agent-status-bridge docs/agent-status-lamp docs/superpowers/plans/2026-07-16-agent-status-bridge-scaffold.md`

Expected: only scaffold/package/docs changes for the bridge.

- [ ] **Step 5: Commit**

```bash
git add tools/agent-status-bridge docs/agent-status-lamp docs/superpowers/plans/2026-07-16-agent-status-bridge-scaffold.md
git commit -m "docs: add agent status bridge scaffold plan"
```
