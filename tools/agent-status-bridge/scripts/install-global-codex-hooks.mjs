#!/usr/bin/env node

import { fileURLToPath } from "node:url";
import {
  getDefaultCodexHookScriptPath,
  installGlobalCodexHooks,
} from "../dist/src/hooks/globalCodexHooks.js";

const scriptPath = getDefaultCodexHookScriptPath(
  fileURLToPath(new URL(".", import.meta.url)),
);

if (process.argv[1] === fileURLToPath(import.meta.url)) {
  try {
    const result = installGlobalCodexHooks({ scriptPath });
    console.log(`Installed global Codex hooks to ${result.configPath}`);
    console.log(`Hook command targets ${result.scriptPath}`);
  } catch (error) {
    console.error(
      error instanceof Error
        ? error.message
        : `Unknown error: ${String(error)}`,
    );
    process.exit(1);
  }
}
