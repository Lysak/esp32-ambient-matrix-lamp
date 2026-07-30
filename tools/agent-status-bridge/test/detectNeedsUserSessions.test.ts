import { describe, expect, it } from "vitest";
import { detectNeedsUserSessions } from "../src/runtime/detectNeedsUserSessions.js";
import type { SourceSessionSnapshot } from "../src/types/source.js";
import type { NormalizedStatus } from "../src/types/status.js";

const makeSnapshot = (
  family: SourceSessionSnapshot["family"],
  sourceSessionId: string,
  status: NormalizedStatus,
): SourceSessionSnapshot => ({
  family,
  sourceSessionId,
  label: sourceSessionId,
  status,
  updatedAt: "2026-07-19T00:00:00.000Z",
});

describe("detectNeedsUserSessions", () => {
  it("returns a session that went thinking -> needs_user", () => {
    const previous = new Map<string, SourceSessionSnapshot>([
      ["s1", makeSnapshot("claude", "s1", "thinking")],
    ]);
    const current = [makeSnapshot("claude", "s1", "needs_user")];

    expect(detectNeedsUserSessions(previous, current)).toEqual([current[0]]);
  });

  it("ignores a session that stays needs_user", () => {
    const previous = new Map<string, SourceSessionSnapshot>([
      ["s1", makeSnapshot("claude", "s1", "needs_user")],
    ]);
    const current = [makeSnapshot("claude", "s1", "needs_user")];

    expect(detectNeedsUserSessions(previous, current)).toEqual([]);
  });

  it("ignores a brand new session that appears needs_user", () => {
    const previous = new Map<string, SourceSessionSnapshot>();
    const current = [makeSnapshot("claude", "s1", "needs_user")];

    expect(detectNeedsUserSessions(previous, current)).toEqual([]);
  });

  it("ignores a session that stays thinking", () => {
    const previous = new Map<string, SourceSessionSnapshot>([
      ["s1", makeSnapshot("claude", "s1", "thinking")],
    ]);
    const current = [makeSnapshot("claude", "s1", "thinking")];

    expect(detectNeedsUserSessions(previous, current)).toEqual([]);
  });

  it("detects multiple sessions asking for input in one poll", () => {
    const previous = new Map<string, SourceSessionSnapshot>([
      ["s1", makeSnapshot("claude", "s1", "thinking")],
      ["s2", makeSnapshot("codex", "s2", "thinking")],
    ]);
    const current = [
      makeSnapshot("claude", "s1", "needs_user"),
      makeSnapshot("codex", "s2", "needs_user"),
    ];

    expect(detectNeedsUserSessions(previous, current)).toEqual(current);
  });
});
