import { mkdirSync, writeFileSync } from "node:fs";
import { homedir } from "node:os";
import { dirname, join } from "node:path";

export const LAUNCH_AGENT_LABEL = "com.ambientmatrix.agentstatusbridge";

export function getDefaultLaunchAgentPlistPath(): string {
  return join(
    homedir(),
    "Library",
    "LaunchAgents",
    `${LAUNCH_AGENT_LABEL}.plist`,
  );
}

export function getDefaultLaunchAgentLogPath(): string {
  return join(
    homedir(),
    "Library",
    "Logs",
    "agent-status-bridge",
    "bridge.log",
  );
}

export interface LaunchAgentOptions {
  nodePath: string;
  mainScriptPath: string;
  workingDirectory: string;
  logPath: string;
  /** Extra PATH entries launchd's minimal environment otherwise lacks (e.g. the `claude` CLI). */
  pathEnv: string;
}

/**
 * launchd runs agents with a bare PATH (no shell profile sourced), so the
 * bridge's `claude agents --json --all` spawn fails with ENOENT unless PATH
 * is set explicitly here.
 */
export function buildLaunchAgentPlist(options: LaunchAgentOptions): string {
  return `<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>Label</key>
  <string>${LAUNCH_AGENT_LABEL}</string>
  <key>ProgramArguments</key>
  <array>
    <string>${options.nodePath}</string>
    <string>${options.mainScriptPath}</string>
  </array>
  <key>WorkingDirectory</key>
  <string>${options.workingDirectory}</string>
  <key>EnvironmentVariables</key>
  <dict>
    <key>PATH</key>
    <string>${options.pathEnv}</string>
  </dict>
  <key>RunAtLoad</key>
  <true/>
  <key>KeepAlive</key>
  <true/>
  <key>StandardOutPath</key>
  <string>${options.logPath}</string>
  <key>StandardErrorPath</key>
  <string>${options.logPath}</string>
</dict>
</plist>
`;
}

export function installLaunchAgent(
  options: LaunchAgentOptions,
  plistPath: string = getDefaultLaunchAgentPlistPath(),
): { plistPath: string } {
  mkdirSync(dirname(plistPath), { recursive: true });
  mkdirSync(dirname(options.logPath), { recursive: true });
  writeFileSync(plistPath, buildLaunchAgentPlist(options));
  return { plistPath };
}
