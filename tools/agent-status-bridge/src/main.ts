import { readFile } from "node:fs/promises";
import { join } from "node:path";
import { aggregateFamilyStatus } from "./aggregators/aggregateFamilyStatus.js";
import { aggregateGlobalStatus } from "./aggregators/aggregateGlobalStatus.js";
import {
  createClaudeCollector,
  runClaudeAgentsJson,
} from "./collectors/claudeCollector.js";
import { createCodexCollector } from "./collectors/codexCollector.js";
import {
  createHomeAssistantPublisher,
  setInputSelectOption,
} from "./publishers/homeAssistantPublisher.js";
import { startFamilyStatusLoop } from "./runtime/familyStatusLoop.js";
import type { NormalizedStatus } from "./types/status.js";

try {
  process.loadEnvFile();
} catch {
  // .env is optional: the bridge must run without Home Assistant configured.
}

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

const haUrl = process.env.HA_URL;
const haToken = process.env.HA_TOKEN;
const haEntityId = process.env.HA_ENTITY_ID ?? "input_select.agent_status";

const publisher =
  haUrl && haToken
    ? createHomeAssistantPublisher({
        entityId: haEntityId,
        setOption: (entityId, option) =>
          setInputSelectOption(haUrl, haToken, entityId, option),
      })
    : undefined;

if (publisher) {
  console.log(`Home Assistant publishing enabled: ${haEntityId} at ${haUrl}`);
} else {
  console.log(
    "HA_URL/HA_TOKEN not set: Home Assistant publishing disabled, logging only",
  );
}

let latestClaudeStatus: NormalizedStatus = "idle";
let latestCodexStatus: NormalizedStatus = "idle";
let lastPublishedGlobalStatus: NormalizedStatus | undefined;

function publishGlobalStatusIfChanged(): void {
  const globalStatus = aggregateGlobalStatus([
    latestClaudeStatus,
    latestCodexStatus,
  ]);

  if (globalStatus === lastPublishedGlobalStatus) {
    return;
  }

  lastPublishedGlobalStatus = globalStatus;
  console.log(`global_status: ${globalStatus}`);

  if (publisher) {
    publisher.publishGlobalStatus(globalStatus).catch((error: unknown) => {
      console.error("Home Assistant publish failed:", error);
    });
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
    latestClaudeStatus = status;
    publishGlobalStatusIfChanged();
  },
});

startFamilyStatusLoop({
  collector: codexCollector,
  aggregate: aggregateFamilyStatus,
  intervalMs: CODEX_POLL_INTERVAL_MS,
  onStatus: (status) => {
    console.log(`codex_status: ${status}`);
    latestCodexStatus = status;
    publishGlobalStatusIfChanged();
  },
});

console.log("agent-status-bridge is running");
