import { mkdirSync, writeFileSync } from "node:fs";
import { homedir } from "node:os";
import { dirname, join } from "node:path";

export const WATCHDOG_LABEL = "com.ambientmatrix.agentstatusbridge.watchdog";

export function getDefaultWatchdogPlistPath(): string {
  return join(homedir(), "Library", "LaunchAgents", `${WATCHDOG_LABEL}.plist`);
}

export function getDefaultWatchdogLogPath(): string {
  return join(
    homedir(),
    "Library",
    "Logs",
    "agent-status-bridge",
    "watchdog.log",
  );
}

export interface WatchdogAgentOptions {
  mainLabel: string;
  mainPlistPath: string;
  logPath: string;
}

/**
 * The main bridge agent has `RunAtLoad` + `KeepAlive`, which keeps it alive
 * once launchd has loaded it — but on at least one observed boot, launchd
 * never bootstrapped it into the per-user domain at all (not disabled, not
 * blocked by Background Task Management, just never loaded), so `KeepAlive`
 * never got a chance to matter. This watchdog is a second, independent,
 * dead-simple LaunchAgent: it just checks whether the main agent is known to
 * launchd and re-bootstraps it if not, on load and every 5 minutes after. It
 * has no `KeepAlive` itself — it's meant to exit immediately each run, not
 * stay resident.
 */
/**
 * Plist `<string>` content is XML text, so `&` (and `<`/`>`, though only `&`
 * is strictly required) in the shell command must be entity-escaped or the
 * plist is not well-formed XML and fails to load.
 */
function escapeXmlText(value: string): string {
  return value
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;");
}

export function buildWatchdogPlist(options: WatchdogAgentOptions): string {
  const checkAndBootstrap = escapeXmlText(
    `launchctl print "gui/$(id -u)/${options.mainLabel}" >/dev/null 2>&1 || launchctl bootstrap "gui/$(id -u)" "${options.mainPlistPath}"`,
  );

  return `<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>Label</key>
  <string>${WATCHDOG_LABEL}</string>
  <key>ProgramArguments</key>
  <array>
    <string>/bin/sh</string>
    <string>-c</string>
    <string>${checkAndBootstrap}</string>
  </array>
  <key>RunAtLoad</key>
  <true/>
  <key>StartInterval</key>
  <integer>300</integer>
  <key>StandardOutPath</key>
  <string>${options.logPath}</string>
  <key>StandardErrorPath</key>
  <string>${options.logPath}</string>
</dict>
</plist>
`;
}

export function installWatchdogAgent(
  options: WatchdogAgentOptions,
  plistPath: string = getDefaultWatchdogPlistPath(),
): { plistPath: string } {
  mkdirSync(dirname(plistPath), { recursive: true });
  mkdirSync(dirname(options.logPath), { recursive: true });
  writeFileSync(plistPath, buildWatchdogPlist(options));
  return { plistPath };
}
