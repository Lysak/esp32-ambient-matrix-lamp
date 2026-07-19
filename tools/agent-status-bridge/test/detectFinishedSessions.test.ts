import { describe, expect, it } from "vitest";
import { detectFinishedSessions } from "../src/runtime/detectFinishedSessions.js";
import type { SourceSessionSnapshot } from "../src/types/source.js";
import type { NormalizedStatus } from "../src/types/status.js";

const makeSnapshot = (
  sourceSessionId: string,
  status: NormalizedStatus,
): SourceSessionSnapshot => ({
  family: "claude",
  sourceSessionId,
  label: sourceSessionId,
  status,
  updatedAt: "2026-07-19T00:00:00.000Z",
});

describe("detectFinishedSessions", () => {
  it("returns a session that went thinking -> idle", () => {
    const previous = new Map<string, NormalizedStatus>([["s1", "thinking"]]);
    const current = [makeSnapshot("s1", "idle")];

    expect(detectFinishedSessions(previous, current)).toEqual([current[0]]);
  });

  it("ignores a session that stays thinking", () => {
    const previous = new Map<string, NormalizedStatus>([["s1", "thinking"]]);
    const current = [makeSnapshot("s1", "thinking")];

    expect(detectFinishedSessions(previous, current)).toEqual([]);
  });

  it("ignores a session that stays idle", () => {
    const previous = new Map<string, NormalizedStatus>([["s1", "idle"]]);
    const current = [makeSnapshot("s1", "idle")];

    expect(detectFinishedSessions(previous, current)).toEqual([]);
  });

  it("ignores a brand new session that appears idle", () => {
    const previous = new Map<string, NormalizedStatus>();
    const current = [makeSnapshot("s1", "idle")];

    expect(detectFinishedSessions(previous, current)).toEqual([]);
  });

  it("does not treat a disappeared session as finished", () => {
    const previous = new Map<string, NormalizedStatus>([["s1", "thinking"]]);
    const current: SourceSessionSnapshot[] = [];

    expect(detectFinishedSessions(previous, current)).toEqual([]);
  });

  it("does not fire for needs_user -> idle in the MVP", () => {
    const previous = new Map<string, NormalizedStatus>([["s1", "needs_user"]]);
    const current = [makeSnapshot("s1", "idle")];

    expect(detectFinishedSessions(previous, current)).toEqual([]);
  });

  it("detects multiple finished sessions in one poll", () => {
    const previous = new Map<string, NormalizedStatus>([
      ["s1", "thinking"],
      ["s2", "thinking"],
    ]);
    const current = [makeSnapshot("s1", "idle"), makeSnapshot("s2", "idle")];

    expect(detectFinishedSessions(previous, current)).toEqual(current);
  });
});
