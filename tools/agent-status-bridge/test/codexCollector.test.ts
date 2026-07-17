import { describe, expect, it } from "vitest";
import { createCodexCollector } from "../src/collectors/codexCollector.js";

const line = (event: Record<string, unknown>) => JSON.stringify(event);

describe("createCodexCollector", () => {
  it("converts raw hook event log lines into normalized source snapshots", async () => {
    const RAW_LOG = [
      line({
        sessionId: "a1",
        label: "codex-1",
        type: "SessionStart",
        timestamp: "2026-07-16T00:00:00.000Z",
      }),
      line({
        sessionId: "a1",
        label: "codex-1",
        type: "UserPromptSubmit",
        timestamp: "2026-07-16T00:00:05.000Z",
      }),
    ].join("\n");

    const collector = createCodexCollector({
      readCodexEventLog: async () => RAW_LOG,
    });

    const snapshots = await collector.poll();

    expect(snapshots).toEqual([
      {
        family: "codex",
        sourceSessionId: "a1",
        label: "codex-1",
        status: "thinking",
        updatedAt: "2026-07-16T00:00:05.000Z",
      },
    ]);
  });

  it("uses the most recent event per session when multiple sessions are present", async () => {
    const RAW_LOG = [
      line({
        sessionId: "a1",
        label: "codex-1",
        type: "UserPromptSubmit",
        timestamp: "2026-07-16T00:00:05.000Z",
      }),
      line({
        sessionId: "a1",
        label: "codex-1",
        type: "Stop",
        timestamp: "2026-07-16T00:00:10.000Z",
      }),
      line({
        sessionId: "b2",
        label: "codex-2",
        type: "PermissionRequest",
        timestamp: "2026-07-16T00:00:01.000Z",
      }),
    ].join("\n");

    const collector = createCodexCollector({
      readCodexEventLog: async () => RAW_LOG,
    });

    const snapshots = await collector.poll();

    expect(snapshots).toEqual([
      {
        family: "codex",
        sourceSessionId: "a1",
        label: "codex-1",
        status: "idle",
        updatedAt: "2026-07-16T00:00:10.000Z",
      },
      {
        family: "codex",
        sourceSessionId: "b2",
        label: "codex-2",
        status: "needs_user",
        updatedAt: "2026-07-16T00:00:01.000Z",
      },
    ]);
  });

  it("returns an empty array when the event log is empty", async () => {
    const collector = createCodexCollector({
      readCodexEventLog: async () => "",
    });

    expect(await collector.poll()).toEqual([]);
  });

  it("skips malformed lines and processes valid ones without crashing", async () => {
    const RAW_LOG = [
      line({
        sessionId: "a1",
        label: "codex-1",
        type: "UserPromptSubmit",
        timestamp: "2026-07-16T00:00:05.000Z",
      }),
      "this is not valid json at all {broken",
      line({
        sessionId: "a1",
        label: "codex-1",
        type: "Stop",
        timestamp: "2026-07-16T00:00:10.000Z",
      }),
    ].join("\n");

    const collector = createCodexCollector({
      readCodexEventLog: async () => RAW_LOG,
    });

    const snapshots = await collector.poll();

    expect(snapshots).toEqual([
      {
        family: "codex",
        sourceSessionId: "a1",
        label: "codex-1",
        status: "idle",
        updatedAt: "2026-07-16T00:00:10.000Z",
      },
    ]);
  });
});
