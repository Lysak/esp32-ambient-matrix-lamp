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
        turnId: "",
      }),
      line({
        sessionId: "a1",
        label: "codex-1",
        type: "UserPromptSubmit",
        timestamp: "2026-07-16T00:00:05.000Z",
        turnId: "turn-1",
      }),
    ].join("\n");

    const collector = createCodexCollector({
      readCodexEventLog: async () => RAW_LOG,
      now: () => Date.parse("2026-07-16T00:00:05.000Z"),
    });

    const snapshots = await collector.poll();

    expect(snapshots).toEqual([
      {
        family: "codex",
        sourceSessionId: "a1",
        label: "codex-1",
        status: "thinking",
        updatedAt: "2026-07-16T00:00:05.000Z",
        rawEventType: "UserPromptSubmit",
        turnId: "turn-1",
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
        turnId: "turn-1",
      }),
      line({
        sessionId: "a1",
        label: "codex-1",
        type: "Stop",
        timestamp: "2026-07-16T00:00:10.000Z",
        turnId: "turn-1",
      }),
      line({
        sessionId: "b2",
        label: "codex-2",
        type: "PermissionRequest",
        timestamp: "2026-07-16T00:00:01.000Z",
        turnId: "turn-9",
      }),
    ].join("\n");

    const collector = createCodexCollector({
      readCodexEventLog: async () => RAW_LOG,
      now: () => Date.parse("2026-07-16T00:00:10.000Z"),
    });

    const snapshots = await collector.poll();

    expect(snapshots).toEqual([
      {
        family: "codex",
        sourceSessionId: "a1",
        label: "codex-1",
        status: "idle",
        updatedAt: "2026-07-16T00:00:10.000Z",
        rawEventType: "Stop",
        turnId: "turn-1",
      },
      {
        family: "codex",
        sourceSessionId: "b2",
        label: "codex-2",
        status: "needs_user",
        updatedAt: "2026-07-16T00:00:01.000Z",
        rawEventType: "PermissionRequest",
        turnId: "turn-9",
      },
    ]);
  });

  it("retains known sessions when later polls contain only appended events", async () => {
    const logs = [
      line({
        sessionId: "a1",
        label: "codex-1",
        type: "UserPromptSubmit",
        timestamp: "2026-07-16T00:00:05.000Z",
        turnId: "turn-1",
      }),
      line({
        sessionId: "b2",
        label: "codex-2",
        type: "Stop",
        timestamp: "2026-07-16T00:00:10.000Z",
        turnId: "turn-2",
      }),
    ];
    let poll = 0;
    const collector = createCodexCollector({
      readCodexEventLog: async () => logs[poll++] ?? "",
      now: () => Date.parse("2026-07-16T00:00:10.000Z"),
    });

    await collector.poll();

    expect(await collector.poll()).toHaveLength(2);
  });

  it("skips events with an empty sessionId instead of treating them as a phantom session", async () => {
    const RAW_LOG = [
      line({
        sessionId: "",
        label: "",
        type: "",
        timestamp: "2026-07-26T09:34:24.341Z",
        turnId: "",
      }),
      line({
        sessionId: "a1",
        label: "codex-1",
        type: "Stop",
        timestamp: "2026-07-16T00:00:10.000Z",
        turnId: "turn-1",
      }),
    ].join("\n");

    const collector = createCodexCollector({
      readCodexEventLog: async () => RAW_LOG,
      now: () => Date.parse("2026-07-16T00:00:10.000Z"),
    });

    const snapshots = await collector.poll();

    expect(snapshots).toEqual([
      {
        family: "codex",
        sourceSessionId: "a1",
        label: "codex-1",
        status: "idle",
        updatedAt: "2026-07-16T00:00:10.000Z",
        rawEventType: "Stop",
        turnId: "turn-1",
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
        turnId: "turn-1",
      }),
      "this is not valid json at all {broken",
      line({
        sessionId: "a1",
        label: "codex-1",
        type: "Stop",
        timestamp: "2026-07-16T00:00:10.000Z",
        turnId: "turn-1",
      }),
    ].join("\n");

    const collector = createCodexCollector({
      readCodexEventLog: async () => RAW_LOG,
      now: () => Date.parse("2026-07-16T00:00:10.000Z"),
    });

    const snapshots = await collector.poll();

    expect(snapshots).toEqual([
      {
        family: "codex",
        sourceSessionId: "a1",
        label: "codex-1",
        status: "idle",
        updatedAt: "2026-07-16T00:00:10.000Z",
        rawEventType: "Stop",
        turnId: "turn-1",
      },
    ]);
  });

  it("drops a session whose last event is older than the staleness window", async () => {
    const RAW_LOG = line({
      sessionId: "zombie",
      label: "codex-old",
      type: "PermissionRequest",
      timestamp: "2026-07-16T00:00:00.000Z",
      turnId: "turn-1",
    });

    const collector = createCodexCollector({
      readCodexEventLog: async () => RAW_LOG,
      now: () => Date.parse("2026-07-16T00:31:00.000Z"),
    });

    expect(await collector.poll()).toEqual([]);
  });
});
