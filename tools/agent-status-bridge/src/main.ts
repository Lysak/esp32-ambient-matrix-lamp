import { readFile } from "node:fs/promises";
import { join } from "node:path";
import { aggregateFamilyStatus } from "./aggregators/aggregateFamilyStatus.js";
import {
  createClaudeCollector,
  runClaudeAgentsJson,
} from "./collectors/claudeCollector.js";
import { createCodexCollector } from "./collectors/codexCollector.js";
import { startFamilyStatusLoop } from "./runtime/familyStatusLoop.js";

const CLAUDE_POLL_INTERVAL_MS = 2000;
const CODEX_POLL_INTERVAL_MS = 2000;
const CODEX_EVENT_LOG_PATH = join(
  import.meta.dirname,
  "..",
  ".runtime",
  "codex-events.log",
);

async function readCodexEventLog(): Promise<string> {
  try {
    return await readFile(CODEX_EVENT_LOG_PATH, "utf8");
  } catch (error) {
    if ((error as NodeJS.ErrnoException).code === "ENOENT") {
      return "";
    }
    throw error;
  }
}

const claudeCollector = createClaudeCollector({ runClaudeAgentsJson });
const codexCollector = createCodexCollector({ readCodexEventLog });

startFamilyStatusLoop({
  collector: claudeCollector,
  aggregate: aggregateFamilyStatus,
  intervalMs: CLAUDE_POLL_INTERVAL_MS,
  onStatus: (status) => {
    console.log(`claude_status: ${status}`);
  },
});

startFamilyStatusLoop({
  collector: codexCollector,
  aggregate: aggregateFamilyStatus,
  intervalMs: CODEX_POLL_INTERVAL_MS,
  onStatus: (status) => {
    console.log(`codex_status: ${status}`);
  },
});

console.log(
  "agent-status-bridge is running; Home Assistant publisher is not implemented yet",
);
