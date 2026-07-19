import { describe, expect, it } from "vitest";
import { detectFinishedSessions } from "../src/runtime/detectFinishedSessions.js";
import type { SourceSessionSnapshot } from "../src/types/source.js";
import type { NormalizedStatus } from "../src/types/status.js";

const makeSnapshot = (
  family: SourceSessionSnapshot["family"],
  sourceSessionId: string,
  status: NormalizedStatus,
  extras: Partial<SourceSessionSnapshot> = {},
): SourceSessionSnapshot => ({
  family,
  sourceSessionId,
  label: sourceSessionId,
  status,
  updatedAt: "2026-07-19T00:00:00.000Z",
  ...extras,
});

describe("detectFinishedSessions", () => {
  it("returns a session that went thinking -> idle", () => {
    const previous = new Map<string, SourceSessionSnapshot>([
      ["s1", makeSnapshot("claude", "s1", "thinking")],
    ]);
    const current = [makeSnapshot("claude", "s1", "idle")];

    expect(detectFinishedSessions(previous, current)).toEqual([current[0]]);
  });

  it("ignores a session that stays thinking", () => {
    const previous = new Map<string, SourceSessionSnapshot>([
      ["s1", makeSnapshot("claude", "s1", "thinking")],
    ]);
    const current = [makeSnapshot("claude", "s1", "thinking")];

    expect(detectFinishedSessions(previous, current)).toEqual([]);
  });

  it("ignores a session that stays idle", () => {
    const previous = new Map<string, SourceSessionSnapshot>([
      ["s1", makeSnapshot("claude", "s1", "idle")],
    ]);
    const current = [makeSnapshot("claude", "s1", "idle")];

    expect(detectFinishedSessions(previous, current)).toEqual([]);
  });

  it("ignores a brand new session that appears idle", () => {
    const previous = new Map<string, SourceSessionSnapshot>();
    const current = [makeSnapshot("claude", "s1", "idle")];

    expect(detectFinishedSessions(previous, current)).toEqual([]);
  });

  it("does not treat a disappeared session as finished", () => {
    const previous = new Map<string, SourceSessionSnapshot>([
      ["s1", makeSnapshot("claude", "s1", "thinking")],
    ]);
    const current: SourceSessionSnapshot[] = [];

    expect(detectFinishedSessions(previous, current)).toEqual([]);
  });

  it("does not fire for needs_user -> idle in the MVP", () => {
    const previous = new Map<string, SourceSessionSnapshot>([
      ["s1", makeSnapshot("claude", "s1", "needs_user")],
    ]);
    const current = [makeSnapshot("claude", "s1", "idle")];

    expect(detectFinishedSessions(previous, current)).toEqual([]);
  });

  it("detects multiple finished sessions in one poll", () => {
    const previous = new Map<string, SourceSessionSnapshot>([
      ["s1", makeSnapshot("claude", "s1", "thinking")],
      ["s2", makeSnapshot("claude", "s2", "thinking")],
    ]);
    const current = [
      makeSnapshot("claude", "s1", "idle"),
      makeSnapshot("claude", "s2", "idle"),
    ];

    expect(detectFinishedSessions(previous, current)).toEqual(current);
  });

  it("treats a newly observed Codex Stop event as finished even when the previous poll missed thinking", () => {
    const previous = new Map<string, SourceSessionSnapshot>([
      [
        "c1",
        makeSnapshot("codex", "c1", "idle", {
          rawEventType: "Stop",
          turnId: "turn-1",
          updatedAt: "2026-07-19T00:00:01.000Z",
        }),
      ],
    ]);
    const current = [
      makeSnapshot("codex", "c1", "idle", {
        rawEventType: "Stop",
        turnId: "turn-2",
        updatedAt: "2026-07-19T00:00:05.000Z",
      }),
    ];

    expect(detectFinishedSessions(previous, current)).toEqual(current);
  });

  it("does not repeat the same Codex Stop event on later polls", () => {
    const previousSnapshot = makeSnapshot("codex", "c1", "idle", {
      rawEventType: "Stop",
      turnId: "turn-2",
      updatedAt: "2026-07-19T00:00:05.000Z",
    });
    const previous = new Map<string, SourceSessionSnapshot>([
      ["c1", previousSnapshot],
    ]);
    const current = [previousSnapshot];

    expect(detectFinishedSessions(previous, current)).toEqual([]);
  });
});
