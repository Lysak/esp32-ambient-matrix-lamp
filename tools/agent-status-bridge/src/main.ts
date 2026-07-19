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
  pressHomeAssistantButton,
  setInputSelectOption,
} from "./publishers/homeAssistantPublisher.js";
import { detectFinishedSessions } from "./runtime/detectFinishedSessions.js";
import { startFamilyStatusLoop } from "./runtime/familyStatusLoop.js";
import type { SourceSessionSnapshot } from "./types/source.js";
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
const haFinishedButtonEntityId =
  process.env.HA_FINISHED_BUTTON_ENTITY_ID ??
  "button.ambient_matrix_lamp_agent_finished_flash";

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

/**
 * Tracks the last seen per-session status for one family, presses the Home
 * Assistant flash button for each session that just went `thinking` -> `idle`,
 * then records the new statuses for the next poll.
 */
function makeFinishedSessionHandler(): (
  snapshots: SourceSessionSnapshot[],
) => void {
  const lastSnapshotBySession = new Map<string, SourceSessionSnapshot>();

  return (snapshots) => {
    const finished = detectFinishedSessions(lastSnapshotBySession, snapshots);

    for (const session of finished) {
      console.log(
        `session_finished: ${session.family}/${session.label} (${session.sourceSessionId})`,
      );

      if (haUrl && haToken) {
        pressHomeAssistantButton(
          haUrl,
          haToken,
          haFinishedButtonEntityId,
        ).catch((error: unknown) => {
          console.error("Home Assistant button press failed:", error);
        });
      }
    }

    lastSnapshotBySession.clear();
    for (const snapshot of snapshots) {
      lastSnapshotBySession.set(snapshot.sourceSessionId, snapshot);
    }
  };
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
  onSnapshots: makeFinishedSessionHandler(),
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
  onSnapshots: makeFinishedSessionHandler(),
});

console.log("agent-status-bridge is running");
