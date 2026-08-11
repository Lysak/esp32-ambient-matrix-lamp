import { describe, expect, it } from "vitest";
import {
  buildGlobalCodexHooksConfig,
  getCodexHookScriptCommand,
  resolveCodexHookNodePath,
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

  it("prefers a stable Homebrew symlink over a versioned execPath", () => {
    expect(
      resolveCodexHookNodePath(
        ["/opt/homebrew/bin/node"],
        "/opt/homebrew/Cellar/node/26.5.0/bin/node",
        (path) => path === "/opt/homebrew/bin/node",
      ),
    ).toBe("/opt/homebrew/bin/node");
  });

  it("falls back to process.execPath when no stable symlink exists", () => {
    expect(
      resolveCodexHookNodePath(
        ["/opt/homebrew/bin/node"],
        "/tmp/custom-node",
        () => false,
      ),
    ).toBe("/tmp/custom-node");
  });

  it("preserves unrelated hooks while replacing a prior bridge command", () => {
    const scriptPath = "/tmp/codex-hook-capture.mjs";
    const config = buildGlobalCodexHooksConfig(scriptPath, {
      hooks: {
        Stop: [
          {
            matcher: ".*",
            hooks: [
              {
                type: "command",
                command: "'/tmp/agterm-status.sh' stop",
                timeout: 5,
              },
            ],
          },
          {
            matcher: ".*",
            hooks: [
              {
                type: "command",
                command: '"/old/node" "/tmp/codex-hook-capture.mjs"',
                timeout: 5,
              },
            ],
          },
        ],
      },
    });

    expect(config.hooks.Stop).toHaveLength(2);
    expect(config.hooks.Stop?.[0]?.hooks[0]?.command).toBe(
      "'/tmp/agterm-status.sh' stop",
    );
    expect(config.hooks.Stop?.[1]?.hooks[0]?.command).toBe(
      getCodexHookScriptCommand(scriptPath),
    );
  });
});
