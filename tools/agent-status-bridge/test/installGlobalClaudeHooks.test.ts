import { describe, expect, it } from "vitest";
import { buildGlobalClaudeHooksConfig } from "../src/hooks/globalClaudeHooks.js";
import { getCodexHookScriptCommand } from "../src/hooks/globalCodexHooks.js";

describe("install-global-claude-hooks", () => {
  it("points every configured event at the shared capture script", () => {
    const scriptPath =
      "/Users/Files/www/pet/esp32-ambient-matrix-lamp/tools/agent-status-bridge/scripts/claude-hook-capture.mjs";
    const config = buildGlobalClaudeHooksConfig(scriptPath);

    expect(config.hooks.Stop?.[0]?.hooks[0]?.command).toBe(
      getCodexHookScriptCommand(scriptPath),
    );
    expect(config.hooks.UserPromptSubmit?.[0]?.hooks[0]?.command).toBe(
      getCodexHookScriptCommand(scriptPath),
    );
  });

  it("only wires the Notification hook to the permission_prompt matcher, not all notifications", () => {
    const config = buildGlobalClaudeHooksConfig("/tmp/claude-hook-capture.mjs");

    expect(config.hooks.Notification?.[0]?.matcher).toBe("permission_prompt");
    expect(config.hooks.Stop?.[0]?.matcher).toBe(".*");
  });

  it("preserves unrelated top-level settings keys", () => {
    const config = buildGlobalClaudeHooksConfig(
      "/tmp/claude-hook-capture.mjs",
      {
        skillOverrides: { "find-skills": "off" },
      },
    );

    expect(config.skillOverrides).toEqual({ "find-skills": "off" });
  });

  it("preserves unrelated hooks while replacing a prior bridge command", () => {
    const scriptPath = "/tmp/claude-hook-capture.mjs";
    const config = buildGlobalClaudeHooksConfig(scriptPath, {
      hooks: {
        Stop: [
          {
            hooks: [
              {
                type: "command",
                command:
                  "'/Users/lysak/.config/agterm/agent-status/agterm-agent-status.sh' completed --auto-reset",
              },
            ],
          },
          {
            matcher: ".*",
            hooks: [
              {
                type: "command",
                command: '"/old/node" "/tmp/claude-hook-capture.mjs"',
                timeout: 5,
              },
            ],
          },
        ],
      },
    });

    expect(config.hooks.Stop).toHaveLength(2);
    expect(config.hooks.Stop?.[0]?.hooks[0]?.command).toBe(
      "'/Users/lysak/.config/agterm/agent-status/agterm-agent-status.sh' completed --auto-reset",
    );
    expect(config.hooks.Stop?.[1]?.hooks[0]?.command).toBe(
      getCodexHookScriptCommand(scriptPath),
    );
  });

  it("preserves an unrelated Notification matcher entry (e.g. agterm's permission_prompt hook) alongside ours", () => {
    const scriptPath = "/tmp/claude-hook-capture.mjs";
    const config = buildGlobalClaudeHooksConfig(scriptPath, {
      hooks: {
        Notification: [
          {
            matcher: "permission_prompt",
            hooks: [
              {
                type: "command",
                command:
                  "'/Users/lysak/.config/agterm/agent-status/agterm-agent-status.sh' blocked",
              },
            ],
          },
        ],
      },
    });

    expect(config.hooks.Notification).toHaveLength(2);
    expect(config.hooks.Notification?.[0]?.hooks[0]?.command).toBe(
      "'/Users/lysak/.config/agterm/agent-status/agterm-agent-status.sh' blocked",
    );
    expect(config.hooks.Notification?.[1]?.hooks[0]?.command).toBe(
      getCodexHookScriptCommand(scriptPath),
    );
  });
});
