import { describe, expect, it } from "vitest";
import {
  getClaudeEventLogPath,
  getDefaultClaudeEventLogPath,
} from "../src/runtime/claudePaths.js";

describe("claudePaths", () => {
  it("returns the macOS machine-wide default path", () => {
    expect(getDefaultClaudeEventLogPath()).toContain(
      "Library/Application Support/ambient-matrix-agent-status/claude-events.log",
    );
  });

  it("prefers CLAUDE_EVENT_LOG_PATH override", () => {
    expect(
      getClaudeEventLogPath({
        CLAUDE_EVENT_LOG_PATH: "/tmp/custom-claude.log",
      } as NodeJS.ProcessEnv),
    ).toBe("/tmp/custom-claude.log");
  });
});
