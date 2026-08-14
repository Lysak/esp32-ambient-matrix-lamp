import { execFileSync } from "node:child_process";
import { describe, expect, it } from "vitest";
import {
  buildWatchdogPlist,
  WATCHDOG_LABEL,
} from "../src/runtime/launchAgentWatchdog.js";

describe("buildWatchdogPlist", () => {
  it("checks for the main agent and bootstraps it only when missing, with the shell command XML-escaped", () => {
    const plist = buildWatchdogPlist({
      mainLabel: "com.ambientmatrix.agentstatusbridge",
      mainPlistPath:
        "/home/user/Library/LaunchAgents/com.ambientmatrix.agentstatusbridge.plist",
      logPath: "/home/user/Library/Logs/agent-status-bridge/watchdog.log",
    });

    expect(plist).toContain(`<string>${WATCHDOG_LABEL}</string>`);
    expect(plist).toContain(
      'launchctl print "gui/$(id -u)/com.ambientmatrix.agentstatusbridge" &gt;/dev/null 2&gt;&amp;1 || launchctl bootstrap "gui/$(id -u)" "/home/user/Library/LaunchAgents/com.ambientmatrix.agentstatusbridge.plist"',
    );
    expect(plist).toContain(
      "<string>/home/user/Library/Logs/agent-status-bridge/watchdog.log</string>",
    );
    expect(plist).toContain("<key>StartInterval</key>");
    expect(plist).toContain("<integer>300</integer>");
    expect(plist).toContain("<key>RunAtLoad</key>");
    expect(plist).not.toContain("<key>KeepAlive</key>");
  });

  it("produces well-formed plist XML that plutil accepts", () => {
    const plist = buildWatchdogPlist({
      mainLabel: "com.ambientmatrix.agentstatusbridge",
      mainPlistPath:
        "/home/user/Library/LaunchAgents/com.ambientmatrix.agentstatusbridge.plist",
      logPath: "/home/user/Library/Logs/agent-status-bridge/watchdog.log",
    });

    expect(() =>
      execFileSync("plutil", ["-lint", "-s", "-"], { input: plist }),
    ).not.toThrow();
  });
});
