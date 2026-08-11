import { describe, expect, it } from "vitest";
import {
  buildHealthCheckPlist,
  HEALTH_CHECK_LABEL,
} from "../src/runtime/healthCheckAgent.js";

describe("buildHealthCheckPlist", () => {
  it("runs the health-check script once at load, with no KeepAlive/StartInterval", () => {
    const plist = buildHealthCheckPlist({
      scriptPath: "/home/user/repo/scripts/health-check.sh",
      logPath: "/home/user/Library/Logs/agent-status-bridge/health-check.log",
    });

    expect(plist).toContain(`<string>${HEALTH_CHECK_LABEL}</string>`);
    expect(plist).toContain("<string>/bin/bash</string>");
    expect(plist).toContain(
      "<string>/home/user/repo/scripts/health-check.sh</string>",
    );
    expect(plist).toContain(
      "<string>/home/user/Library/Logs/agent-status-bridge/health-check.log</string>",
    );
    expect(plist).toContain("<key>RunAtLoad</key>");
    expect(plist).not.toContain("<key>KeepAlive</key>");
    expect(plist).not.toContain("<key>StartInterval</key>");
  });
});
