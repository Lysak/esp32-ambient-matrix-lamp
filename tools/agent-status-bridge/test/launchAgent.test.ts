import { describe, expect, it } from "vitest";
import {
  buildLaunchAgentPlist,
  LAUNCH_AGENT_LABEL,
} from "../src/runtime/launchAgent.js";

describe("buildLaunchAgentPlist", () => {
  it("embeds the node path, main script, working directory, log path, and PATH", () => {
    const plist = buildLaunchAgentPlist({
      nodePath: "/opt/homebrew/bin/node",
      mainScriptPath: "/repo/tools/agent-status-bridge/dist/src/main.js",
      workingDirectory: "/repo/tools/agent-status-bridge",
      logPath: "/home/user/Library/Logs/agent-status-bridge/bridge.log",
      pathEnv: "/opt/homebrew/bin:/usr/bin:/bin",
    });

    expect(plist).toContain(`<string>${LAUNCH_AGENT_LABEL}</string>`);
    expect(plist).toContain("<string>/opt/homebrew/bin/node</string>");
    expect(plist).toContain(
      "<string>/repo/tools/agent-status-bridge/dist/src/main.js</string>",
    );
    expect(plist).toContain("<string>/repo/tools/agent-status-bridge</string>");
    expect(plist).toContain(
      "<string>/home/user/Library/Logs/agent-status-bridge/bridge.log</string>",
    );
    expect(plist).toContain("<string>/opt/homebrew/bin:/usr/bin:/bin</string>");
    expect(plist).toContain("<key>RunAtLoad</key>");
    expect(plist).toContain("<key>KeepAlive</key>");
  });
});
