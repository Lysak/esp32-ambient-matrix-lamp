import { describe, expect, it } from "vitest";
import { normalizeClaudeStatus } from "../src/collectors/normalizeClaudeStatus.js";

describe("normalizeClaudeStatus", () => {
  it("maps confirmed 'busy' to 'thinking'", () => {
    expect(normalizeClaudeStatus("busy")).toBe("thinking");
  });

  it("maps confirmed 'idle' to 'idle'", () => {
    expect(normalizeClaudeStatus("idle")).toBe("idle");
  });

  it("maps confirmed 'waiting' to 'needs_user'", () => {
    expect(normalizeClaudeStatus("waiting")).toBe("needs_user");
  });

  it("maps any unconfirmed raw status to 'error' so it is visible, not silently ignored", () => {
    expect(normalizeClaudeStatus("some_future_status")).toBe("error");
  });
});
