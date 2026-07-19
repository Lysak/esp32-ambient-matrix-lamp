import { describe, expect, it } from "vitest";
import { AGENT_FAMILIES, NORMALIZED_STATUSES } from "../src/types/status.js";

describe("type contracts", () => {
  it("exports the fixed agent families", () => {
    expect(AGENT_FAMILIES).toEqual(["codex", "claude"]);
  });

  it("exports the fixed normalized statuses", () => {
    expect(NORMALIZED_STATUSES).toEqual([
      "idle",
      "thinking",
      "needs_user",
      "done",
      "error",
    ]);
  });
});
