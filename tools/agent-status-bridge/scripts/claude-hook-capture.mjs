#!/usr/bin/env node
// Invoked directly by Claude Code global hooks (`~/.claude/settings.json`) once
// per hook event. Imports the compiled formatter and path helper so field
// extraction and shared log location stay in one place — run `pnpm run build`
// after changing either source module, or this script keeps using stale
// compiled output.

import { appendFileSync, mkdirSync } from "node:fs";
import { dirname } from "node:path";
import { formatClaudeHookEvent } from "../dist/src/hooks/formatClaudeHookEvent.js";
import { getDefaultClaudeEventLogPath } from "../dist/src/runtime/claudePaths.js";

const LOG_PATH =
  process.env.CLAUDE_EVENT_LOG_PATH || getDefaultClaudeEventLogPath();

function readStdin() {
  const chunks = [];
  return new Promise((resolve) => {
    process.stdin.on("data", (chunk) => chunks.push(chunk));
    process.stdin.on("end", () =>
      resolve(Buffer.concat(chunks).toString("utf8")),
    );
    process.stdin.on("error", () => resolve(""));
  });
}

const raw = await readStdin();

let payload = null;
try {
  payload = JSON.parse(raw);
} catch {
  // Malformed payload: still respond below so Claude Code is never blocked, just skip logging.
}

if (payload !== null) {
  try {
    const { logLine, response } = formatClaudeHookEvent(
      payload,
      new Date().toISOString(),
    );
    mkdirSync(dirname(LOG_PATH), { recursive: true });
    appendFileSync(LOG_PATH, logLine);
    process.stdout.write(response);
  } catch {
    // Filesystem error: still respond so Claude Code is never blocked, just skip logging.
    process.stdout.write(JSON.stringify({ continue: true }));
  }
} else {
  process.stdout.write(JSON.stringify({ continue: true }));
}

process.exit(0);
