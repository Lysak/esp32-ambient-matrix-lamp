#!/usr/bin/env node

import { execFileSync } from "node:child_process";
import { fileURLToPath } from "node:url";
import { resolveCodexHookNodePath } from "../dist/src/hooks/globalCodexHooks.js";
import {
  getDefaultLaunchAgentLogPath,
  installLaunchAgent,
  LAUNCH_AGENT_LABEL,
} from "../dist/src/runtime/launchAgent.js";

const workingDirectory = fileURLToPath(new URL("..", import.meta.url));
const mainScriptPath = `${workingDirectory}dist/src/main.js`;
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
}
