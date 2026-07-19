import { randomUUID } from "node:crypto";
import { describe, expect, it } from "vitest";
import { aggregateFamilyStatus } from "../src/aggregators/aggregateFamilyStatus.js";
import type { SourceSessionSnapshot } from "../src/types/source.js";

const makeSnapshot = (
  status: SourceSessionSnapshot["status"],
): SourceSessionSnapshot => ({
  family: "codex",
  sourceSessionId: randomUUID(),
  label: status,
  status,
  updatedAt: "2026-07-16T00:00:00.000Z",
});

describe("aggregateFamilyStatus", () => {
  it("returns idle for no sessions", () => {
    expect(aggregateFamilyStatus([])).toBe("idle");
  });

  it("prefers error over every other status", () => {
    expect(
      aggregateFamilyStatus([
        makeSnapshot("thinking"),
        makeSnapshot("needs_user"),
        makeSnapshot("error"),
      ]),
    ).toBe("error");
  });

  it("prefers needs_user over thinking and done", () => {
    expect(
      aggregateFamilyStatus([
        makeSnapshot("thinking"),
        makeSnapshot("done"),
        makeSnapshot("needs_user"),
      ]),
    ).toBe("needs_user");
  });
});
