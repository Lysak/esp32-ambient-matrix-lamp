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
