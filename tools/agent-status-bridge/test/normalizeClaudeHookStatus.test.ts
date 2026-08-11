import { describe, expect, it } from "vitest";
import { normalizeClaudeHookStatus } from "../src/collectors/normalizeClaudeHookStatus.js";

describe("normalizeClaudeHookStatus", () => {
  it("maps 'SessionStart' to 'idle' (not working yet)", () => {
    expect(normalizeClaudeHookStatus("SessionStart")).toBe("idle");
  });

  it("maps 'Stop' to 'idle' (turn finished)", () => {
    expect(normalizeClaudeHookStatus("Stop")).toBe("idle");
  });

  it("maps 'UserPromptSubmit' to 'thinking' (working)", () => {
    expect(normalizeClaudeHookStatus("UserPromptSubmit")).toBe("thinking");
  });

  it("maps 'PreToolUse' to 'thinking' (working)", () => {
    expect(normalizeClaudeHookStatus("PreToolUse")).toBe("thinking");
  });

  it("maps 'PostToolUse' to 'thinking' (working)", () => {
    expect(normalizeClaudeHookStatus("PostToolUse")).toBe("thinking");
  });

  it("maps 'Notification' to 'needs_user' (only fired for the permission_prompt matcher)", () => {
    expect(normalizeClaudeHookStatus("Notification")).toBe("needs_user");
  });

  it("maps any unconfirmed raw event to 'error' so it is visible, not silently ignored", () => {
    expect(normalizeClaudeHookStatus("some_future_event")).toBe("error");
  });
});
