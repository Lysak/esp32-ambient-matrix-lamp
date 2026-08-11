#!/usr/bin/env node

import { fileURLToPath } from "node:url";
import {
  getDefaultClaudeHookScriptPath,
  installGlobalClaudeHooks,
} from "../dist/src/hooks/globalClaudeHooks.js";

const scriptPath = getDefaultClaudeHookScriptPath(
  fileURLToPath(new URL(".", import.meta.url)),
);

if (process.argv[1] === fileURLToPath(import.meta.url)) {
  try {
    const result = installGlobalClaudeHooks({ scriptPath });
    console.log(`Installed global Claude Code hooks to ${result.configPath}`);
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
