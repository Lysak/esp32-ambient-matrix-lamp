import { describe, expect, it } from "vitest";
import { formatClaudeHookEvent } from "../src/hooks/formatClaudeHookEvent.js";

const NOW = "2026-08-08T12:00:00.000Z";

describe("formatClaudeHookEvent", () => {
  it("builds a RawClaudeEvent from a well-formed hook payload", () => {
    const result = formatClaudeHookEvent(
      {
        session_id: "abc-123",
        cwd: "/Users/Files/www/pet/esp32-ambient-matrix-lamp",
        hook_event_name: "UserPromptSubmit",
      },
      NOW,
    );

    expect(result.event).toEqual({
      sessionId: "abc-123",
      label: "/Users/Files/www/pet/esp32-ambient-matrix-lamp",
      type: "UserPromptSubmit",
      timestamp: NOW,
    });
  });

  it("produces a single-line JSON log line terminated by a newline", () => {
    const result = formatClaudeHookEvent(
      { session_id: "abc-123", cwd: "/tmp", hook_event_name: "Stop" },
      NOW,
    );

    expect(result.logLine).toBe(
      `${JSON.stringify({
        sessionId: "abc-123",
        label: "/tmp",
        type: "Stop",
        timestamp: NOW,
      })}\n`,
    );
  });

  it("defaults missing or non-string fields to empty strings instead of throwing", () => {
    const result = formatClaudeHookEvent({}, NOW);

    expect(result.event).toEqual({
      sessionId: "",
      label: "",
      type: "",
      timestamp: NOW,
    });
  });

  it("always responds with continue: true so Claude Code is never blocked", () => {
    const result = formatClaudeHookEvent(null, NOW);

    expect(result.response).toBe(JSON.stringify({ continue: true }));
  });
});
