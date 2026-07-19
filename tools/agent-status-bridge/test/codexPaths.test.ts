import { describe, expect, it } from "vitest";
import {
  getCodexEventLogPath,
  getDefaultCodexEventLogPath,
} from "../src/runtime/codexPaths.js";

describe("codexPaths", () => {
  it("returns the macOS machine-wide default path", () => {
    expect(getDefaultCodexEventLogPath()).toContain(
      "Library/Application Support/ambient-matrix-agent-status/codex-events.log",
    );
  });

  it("prefers CODEX_EVENT_LOG_PATH override", () => {
    expect(
      getCodexEventLogPath({
        CODEX_EVENT_LOG_PATH: "/tmp/custom-codex.log",
      } as NodeJS.ProcessEnv),
    ).toBe("/tmp/custom-codex.log");
  });
});
