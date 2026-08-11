import { mkdirSync, writeFileSync } from "node:fs";
import { homedir } from "node:os";
import { dirname, join } from "node:path";

export const HEALTH_CHECK_LABEL =
  "com.ambientmatrix.agentstatusbridge.healthcheck";

export function getDefaultHealthCheckPlistPath(): string {
  return join(
    homedir(),
    "Library",
    "LaunchAgents",
    `${HEALTH_CHECK_LABEL}.plist`,
  );
}

export function getDefaultHealthCheckLogPath(): string {
  return join(
    homedir(),
    "Library",
    "Logs",
    "agent-status-bridge",
    "health-check.log",
  );
}

export interface HealthCheckAgentOptions {
  scriptPath: string;
  logPath: string;
}

/**
 * `RunAtLoad` only, no `KeepAlive`/`StartInterval`: this checks once per
 * login/reboot whether the bridge LaunchAgent and the Codex/Claude hook
 * registrations are actually in place, and fires a single macOS
 * notification (via the script's own `osascript` call) if any of them are
 * missing. It doesn't fix anything itself — that's the watchdog's job for
 * the LaunchAgent; hooks need a manual `pnpm run install:*-hooks` re-run.
 */
export function buildHealthCheckPlist(
  options: HealthCheckAgentOptions,
): string {
  return `<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>Label</key>
  <string>${HEALTH_CHECK_LABEL}</string>
  <key>ProgramArguments</key>
  <array>
    <string>/bin/bash</string>
    <string>${options.scriptPath}</string>
  </array>
  <key>RunAtLoad</key>
  <true/>
  <key>StandardOutPath</key>
  <string>${options.logPath}</string>
  <key>StandardErrorPath</key>
  <string>${options.logPath}</string>
</dict>
</plist>
`;
}

export function installHealthCheckAgent(
  options: HealthCheckAgentOptions,
  plistPath: string = getDefaultHealthCheckPlistPath(),
): { plistPath: string } {
  mkdirSync(dirname(plistPath), { recursive: true });
  mkdirSync(dirname(options.logPath), { recursive: true });
  writeFileSync(plistPath, buildHealthCheckPlist(options));
  return { plistPath };
}
