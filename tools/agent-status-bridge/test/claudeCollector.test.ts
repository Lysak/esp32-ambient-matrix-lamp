import { describe, expect, it } from "vitest";
import { createClaudeCollector } from "../src/collectors/claudeCollector.js";

const line = (event: Record<string, unknown>) => JSON.stringify(event);

describe("createClaudeCollector", () => {
  it("converts raw hook event log lines into normalized source snapshots", async () => {
    const RAW_LOG = [
      line({
        sessionId: "s1",
        label: "/Users/Files/www/pet/esp32-ambient-matrix-lamp",
        type: "SessionStart",
        timestamp: "2026-08-08T00:00:00.000Z",
      }),
      line({
        sessionId: "s1",
        label: "/Users/Files/www/pet/esp32-ambient-matrix-lamp",
        type: "UserPromptSubmit",
        timestamp: "2026-08-08T00:00:05.000Z",
      }),
    ].join("\n");

    const collector = createClaudeCollector({
      readClaudeEventLog: async () => RAW_LOG,
      now: () => Date.parse("2026-08-08T00:00:05.000Z"),
    });

    const snapshots = await collector.poll();

    expect(snapshots).toEqual([
      {
        family: "claude",
        sourceSessionId: "s1",
        label: "/Users/Files/www/pet/esp32-ambient-matrix-lamp",
        status: "thinking",
        updatedAt: "2026-08-08T00:00:05.000Z",
        rawEventType: "UserPromptSubmit",
      },
    ]);
  });

  it("uses the most recent event per session when multiple sessions are present", async () => {
    const RAW_LOG = [
      line({
        sessionId: "s1",
        label: "claude-1",
        type: "UserPromptSubmit",
        timestamp: "2026-08-08T00:00:05.000Z",
      }),
      line({
        sessionId: "s1",
        label: "claude-1",
        type: "Stop",
        timestamp: "2026-08-08T00:00:10.000Z",
      }),
      line({
        sessionId: "s2",
        label: "claude-2",
        type: "Notification",
        timestamp: "2026-08-08T00:00:01.000Z",
      }),
    ].join("\n");

    const collector = createClaudeCollector({
      readClaudeEventLog: async () => RAW_LOG,
      now: () => Date.parse("2026-08-08T00:00:10.000Z"),
    });

    const snapshots = await collector.poll();

    expect(snapshots).toEqual([
      {
        family: "claude",
        sourceSessionId: "s1",
        label: "claude-1",
        status: "idle",
        updatedAt: "2026-08-08T00:00:10.000Z",
        rawEventType: "Stop",
      },
      {
        family: "claude",
        sourceSessionId: "s2",
        label: "claude-2",
        status: "needs_user",
        updatedAt: "2026-08-08T00:00:01.000Z",
        rawEventType: "Notification",
      },
    ]);
  });

  it("skips events with an empty sessionId instead of treating them as a phantom session", async () => {
    const RAW_LOG = [
      line({
        sessionId: "",
        label: "",
        type: "",
        timestamp: "2026-08-08T00:00:00.000Z",
      }),
      line({
        sessionId: "s1",
        label: "claude-1",
        type: "Stop",
        timestamp: "2026-08-08T00:00:10.000Z",
      }),
    ].join("\n");

    const collector = createClaudeCollector({
      readClaudeEventLog: async () => RAW_LOG,
      now: () => Date.parse("2026-08-08T00:00:10.000Z"),
    });

    const snapshots = await collector.poll();

    expect(snapshots).toEqual([
      {
        family: "claude",
        sourceSessionId: "s1",
        label: "claude-1",
        status: "idle",
        updatedAt: "2026-08-08T00:00:10.000Z",
        rawEventType: "Stop",
      },
    ]);
  });

  it("returns an empty array when the event log is empty", async () => {
    const collector = createClaudeCollector({
      readClaudeEventLog: async () => "",
    });

    expect(await collector.poll()).toEqual([]);
  });

  it("skips malformed lines and processes valid ones without crashing", async () => {
    const RAW_LOG = [
      line({
        sessionId: "s1",
        label: "claude-1",
        type: "UserPromptSubmit",
        timestamp: "2026-08-08T00:00:05.000Z",
      }),
      "this is not valid json at all {broken",
      line({
        sessionId: "s1",
        label: "claude-1",
        type: "Stop",
        timestamp: "2026-08-08T00:00:10.000Z",
      }),
    ].join("\n");

    const collector = createClaudeCollector({
      readClaudeEventLog: async () => RAW_LOG,
      now: () => Date.parse("2026-08-08T00:00:10.000Z"),
    });

    const snapshots = await collector.poll();

    expect(snapshots).toEqual([
      {
        family: "claude",
        sourceSessionId: "s1",
        label: "claude-1",
        status: "idle",
        updatedAt: "2026-08-08T00:00:10.000Z",
        rawEventType: "Stop",
      },
    ]);
  });

  it("drops a session whose last event is older than the staleness window", async () => {
    const RAW_LOG = line({
      sessionId: "zombie",
      label: "claude-old",
      type: "UserPromptSubmit",
      timestamp: "2026-08-08T00:00:00.000Z",
    });

    const collector = createClaudeCollector({
      readClaudeEventLog: async () => RAW_LOG,
      now: () => Date.parse("2026-08-08T00:31:00.000Z"),
    });

    expect(await collector.poll()).toEqual([]);
  });

  it("drops non-error events from CodexBar's throwaway probe session, keeping error ones", async () => {
    const RAW_LOG = [
      line({
        sessionId: "probe-1",
        label: "/tmp/CodexBar/ClaudeProbe",
        type: "UserPromptSubmit",
        timestamp: "2026-08-08T00:00:05.000Z",
      }),
      line({
        sessionId: "probe-2",
        label: "/tmp/CodexBar/ClaudeProbe",
        type: "some_unmapped_event",
        timestamp: "2026-08-08T00:00:06.000Z",
      }),
      line({
        sessionId: "s1",
        label: "claude-1",
        type: "Stop",
        timestamp: "2026-08-08T00:00:10.000Z",
      }),
    ].join("\n");

    const collector = createClaudeCollector({
      readClaudeEventLog: async () => RAW_LOG,
      now: () => Date.parse("2026-08-08T00:00:10.000Z"),
    });

    const snapshots = await collector.poll();

    expect(snapshots).toEqual([
      {
        family: "claude",
        sourceSessionId: "probe-2",
        label: "/tmp/CodexBar/ClaudeProbe",
        status: "error",
        updatedAt: "2026-08-08T00:00:06.000Z",
        rawEventType: "some_unmapped_event",
      },
      {
        family: "claude",
        sourceSessionId: "s1",
        label: "claude-1",
        status: "idle",
        updatedAt: "2026-08-08T00:00:10.000Z",
        rawEventType: "Stop",
      },
    ]);
  });
});
