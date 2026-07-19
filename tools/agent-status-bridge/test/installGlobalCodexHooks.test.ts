import { describe, expect, it } from "vitest";
import {
  buildGlobalCodexHooksConfig,
  getCodexHookScriptCommand,
} from "../src/hooks/globalCodexHooks.js";

describe("install-global-codex-hooks", () => {
  it("points every configured event at the shared capture script", () => {
    const scriptPath =
      "/Users/Files/www/pet/esp32-ambient-matrix-lamp/tools/agent-status-bridge/scripts/codex-hook-capture.mjs";
    const config = buildGlobalCodexHooksConfig(scriptPath);

    expect(config.hooks.Stop?.[0]?.hooks[0]?.command).toBe(
      getCodexHookScriptCommand(scriptPath),
    );
    expect(config.hooks.UserPromptSubmit?.[0]?.hooks[0]?.command).toBe(
      getCodexHookScriptCommand(scriptPath),
    );
  });

  it("builds a node command that targets codex-hook-capture.mjs", () => {
    expect(getCodexHookScriptCommand("/tmp/codex-hook-capture.mjs")).toContain(
      "codex-hook-capture.mjs",
    );
  });
});
