import { describe, expect, it } from "vitest";
import { aggregateGlobalStatus } from "../src/aggregators/aggregateGlobalStatus.js";

describe("aggregateGlobalStatus", () => {
  it("returns idle for no statuses", () => {
    expect(aggregateGlobalStatus([])).toBe("idle");
  });

  it("prefers error over every other family status", () => {
    expect(aggregateGlobalStatus(["thinking", "needs_user", "error"])).toBe(
      "error",
    );
  });

  it("prefers needs_user over thinking and done", () => {
    expect(aggregateGlobalStatus(["thinking", "done", "needs_user"])).toBe(
      "needs_user",
    );
  });
});
