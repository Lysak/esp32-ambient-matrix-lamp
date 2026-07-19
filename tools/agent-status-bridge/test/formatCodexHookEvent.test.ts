import { describe, expect, it } from "vitest";
import { formatCodexHookEvent } from "../src/hooks/formatCodexHookEvent.js";

const NOW = "2026-07-16T12:00:00.000Z";

describe("formatCodexHookEvent", () => {
  it("builds a RawCodexEvent from a well-formed hook payload", () => {
    const result = formatCodexHookEvent(
      {
        session_id: "abc-123",
        cwd: "/Users/Files/www/pet/esp32-ambient-matrix-lamp",
        hook_event_name: "UserPromptSubmit",
        turn_id: "turn-1",
      },
      NOW,
    );

    expect(result.event).toEqual({
      sessionId: "abc-123",
      label: "/Users/Files/www/pet/esp32-ambient-matrix-lamp",
      type: "UserPromptSubmit",
      timestamp: NOW,
      turnId: "turn-1",
    });
  });

  it("produces a single-line JSON log line terminated by a newline", () => {
    const result = formatCodexHookEvent(
      { session_id: "abc-123", cwd: "/tmp", hook_event_name: "Stop" },
      NOW,
    );

    expect(result.logLine).toBe(
      `${JSON.stringify({
        sessionId: "abc-123",
        label: "/tmp",
        type: "Stop",
        timestamp: NOW,
        turnId: "",
      })}\n`,
    );
  });

  it("defaults missing or non-string fields to empty strings instead of throwing", () => {
    const result = formatCodexHookEvent({}, NOW);

    expect(result.event).toEqual({
      sessionId: "",
      label: "",
      type: "",
      timestamp: NOW,
      turnId: "",
    });
  });

  it("always responds with continue: true so Codex is never blocked", () => {
    const result = formatCodexHookEvent(null, NOW);

    expect(result.response).toBe(JSON.stringify({ continue: true }));
  });
});
