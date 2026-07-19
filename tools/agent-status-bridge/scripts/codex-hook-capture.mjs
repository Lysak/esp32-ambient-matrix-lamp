#!/usr/bin/env node
// Invoked directly by Codex global hooks (`~/.codex/hooks.json`) once per hook
// event. Imports the compiled formatter and path helper so field extraction and
// shared log location stay in one place — run `pnpm run build` after changing
// either source module, or this script keeps using stale compiled output.

import { appendFileSync, mkdirSync } from "node:fs";
import { dirname } from "node:path";
import { formatCodexHookEvent } from "../dist/src/hooks/formatCodexHookEvent.js";
import { getDefaultCodexEventLogPath } from "../dist/src/runtime/codexPaths.js";

const LOG_PATH =
  process.env.CODEX_EVENT_LOG_PATH || getDefaultCodexEventLogPath();

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
  // Malformed payload: still respond below so Codex is never blocked, just skip logging.
}

if (payload !== null) {
  try {
    const { logLine, response } = formatCodexHookEvent(
      payload,
      new Date().toISOString(),
    );
    mkdirSync(dirname(LOG_PATH), { recursive: true });
    appendFileSync(LOG_PATH, logLine);
    process.stdout.write(response);
  } catch {
    // Filesystem error: still respond so Codex is never blocked, just skip logging.
    process.stdout.write(JSON.stringify({ continue: true }));
  }
} else {
  process.stdout.write(JSON.stringify({ continue: true }));
}

process.exit(0);
