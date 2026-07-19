import { describe, expect, it } from "vitest";
import { createClaudeCollector } from "../src/collectors/claudeCollector.js";

const RAW_OUTPUT = JSON.stringify([
  {
    pid: 15454,
    cwd: "/Users/Files/www/pet/esp32-ambient-matrix-lamp",
    kind: "interactive",
    startedAt: 1784200314797,
    sessionId: "1e5b7f13-b9e3-45ef-88db-aa3c0936f3a0",
    name: "esp32-ambient-matrix-lamp-56",
    status: "busy",
  },
  {
    pid: 16730,
    cwd: "/Users/dmytrii.lysak/claude",
    kind: "interactive",
    startedAt: 1784200655544,
    sessionId: "a5318d6f-8dd2-456e-b8ad-a6010840a36e",
    name: "claude-e4",
    status: "idle",
  },
]);

describe("createClaudeCollector", () => {
  it("converts raw CLI sessions into normalized source snapshots", async () => {
    const collector = createClaudeCollector({
      runClaudeAgentsJson: async () => RAW_OUTPUT,
    });

    const snapshots = await collector.poll();

    expect(snapshots).toEqual([
      {
        family: "claude",
        sourceSessionId: "1e5b7f13-b9e3-45ef-88db-aa3c0936f3a0",
        label: "esp32-ambient-matrix-lamp-56",
        status: "thinking",
        updatedAt: expect.any(String),
      },
      {
        family: "claude",
        sourceSessionId: "a5318d6f-8dd2-456e-b8ad-a6010840a36e",
        label: "claude-e4",
        status: "idle",
        updatedAt: expect.any(String),
      },
    ]);
  });

  it("returns an empty array when no sessions are running", async () => {
    const collector = createClaudeCollector({
      runClaudeAgentsJson: async () => "[]",
    });

    expect(await collector.poll()).toEqual([]);
  });
});
