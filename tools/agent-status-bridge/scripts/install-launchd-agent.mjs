#!/usr/bin/env node

import { execFileSync } from "node:child_process";
import { fileURLToPath } from "node:url";
import { resolveCodexHookNodePath } from "../dist/src/hooks/globalCodexHooks.js";
import {
  getDefaultHealthCheckLogPath,
  HEALTH_CHECK_LABEL,
  installHealthCheckAgent,
} from "../dist/src/runtime/healthCheckAgent.js";
import {
  getDefaultLaunchAgentLogPath,
  installLaunchAgent,
  LAUNCH_AGENT_LABEL,
} from "../dist/src/runtime/launchAgent.js";
import {
  getDefaultWatchdogLogPath,
  installWatchdogAgent,
  WATCHDOG_LABEL,
} from "../dist/src/runtime/launchAgentWatchdog.js";

const workingDirectory = fileURLToPath(new URL("..", import.meta.url));
const mainScriptPath = `${workingDirectory}dist/src/main.js`;
const healthCheckScriptPath = `${workingDirectory}scripts/health-check.sh`;
const nodePath = resolveCodexHookNodePath();

// launchd's default environment lacks the CLI dirs (`claude`, homebrew) a
// normal shell picks up from .zprofile/.zshrc.
const pathEnv = [
  "/opt/homebrew/bin",
  `${process.env.HOME}/.local/bin`,
  "/usr/local/bin",
  "/usr/bin",
  "/bin",
  "/usr/sbin",
  "/sbin",
].join(":");

if (process.argv[1] === fileURLToPath(import.meta.url)) {
  const { plistPath } = installLaunchAgent({
    nodePath,
    mainScriptPath,
    workingDirectory,
    logPath: getDefaultLaunchAgentLogPath(),
    pathEnv,
  });

  try {
    execFileSync("launchctl", ["unload", plistPath], { stdio: "ignore" });
  } catch {
    // Not previously loaded: fine.
  }
  execFileSync("launchctl", ["load", plistPath], { stdio: "inherit" });

  console.log(`Installed launch agent: ${LAUNCH_AGENT_LABEL}`);
  console.log(`Plist: ${plistPath}`);
  console.log(`Runs: ${nodePath} ${mainScriptPath}`);

  // launchd is expected to bootstrap RunAtLoad agents from
  // ~/Library/LaunchAgents automatically at every login, but this has been
  // observed to silently not happen on at least one boot (not disabled, not
  // blocked, just never loaded). This watchdog re-checks every 5 minutes and
  // self-heals instead of requiring a manual `launchctl bootstrap`.
  const { plistPath: watchdogPlistPath } = installWatchdogAgent({
    mainLabel: LAUNCH_AGENT_LABEL,
    mainPlistPath: plistPath,
    logPath: getDefaultWatchdogLogPath(),
  });

  try {
    execFileSync("launchctl", ["unload", watchdogPlistPath], {
      stdio: "ignore",
    });
  } catch {
    // Not previously loaded: fine.
  }
  execFileSync("launchctl", ["load", watchdogPlistPath], {
    stdio: "inherit",
  });

  console.log(`Installed watchdog agent: ${WATCHDOG_LABEL}`);
  console.log(`Plist: ${watchdogPlistPath}`);

  // Runs once per login/reboot and fires a macOS notification if the main
  // agent isn't running or the Codex/Claude hook registrations are missing
  // from their global config files — problems the watchdog above can't
  // detect because it only checks launchd's own bookkeeping.
  const { plistPath: healthCheckPlistPath } = installHealthCheckAgent({
    scriptPath: healthCheckScriptPath,
    logPath: getDefaultHealthCheckLogPath(),
  });

  try {
    execFileSync("launchctl", ["unload", healthCheckPlistPath], {
      stdio: "ignore",
    });
  } catch {
    // Not previously loaded: fine.
  }
  execFileSync("launchctl", ["load", healthCheckPlistPath], {
    stdio: "inherit",
  });

  console.log(`Installed health-check agent: ${HEALTH_CHECK_LABEL}`);
  console.log(`Plist: ${healthCheckPlistPath}`);
}
