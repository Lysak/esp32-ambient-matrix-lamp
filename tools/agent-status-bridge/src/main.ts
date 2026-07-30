import { readFile } from "node:fs/promises";
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
import { getCodexEventLogPath } from "./runtime/codexPaths.js";
import { detectFinishedSessions } from "./runtime/detectFinishedSessions.js";
import { detectNeedsUserSessions } from "./runtime/detectNeedsUserSessions.js";
import { startFamilyStatusLoop } from "./runtime/familyStatusLoop.js";
import {
  acquireSingletonLock,
  getSingletonLockPath,
  releaseSingletonLock,
} from "./runtime/singletonLock.js";
import type { SourceSessionSnapshot } from "./types/source.js";
import type { NormalizedStatus } from "./types/status.js";

try {
  process.loadEnvFile();
} catch {
  // .env is optional: the bridge must run without Home Assistant configured.
}

const singletonLockPath = getSingletonLockPath(process.env);
const singletonLockResult = acquireSingletonLock(
  singletonLockPath,
  process.pid,
);

if (!singletonLockResult.acquired) {
  console.error(
    `agent-status-bridge is already running (pid ${singletonLockResult.existingPid}); ` +
      `refusing to start a second instance to avoid duplicate Home Assistant notifications. ` +
      `Lock file: ${singletonLockPath}`,
  );
  process.exit(1);
}

process.on("exit", () => {
  releaseSingletonLock(singletonLockPath, process.pid);
});
process.on("SIGINT", () => process.exit(0));
process.on("SIGTERM", () => process.exit(0));

const CLAUDE_POLL_INTERVAL_MS = 2000;
const CODEX_POLL_INTERVAL_MS = 2000;
const CODEX_EVENT_LOG_PATH = getCodexEventLogPath(process.env);

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
const haNeedsUserButtonEntityId =
  process.env.HA_NEEDS_USER_BUTTON_ENTITY_ID ??
  "button.ambient_matrix_lamp_agent_needs_user_flash";

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
 * The physical flash script runs in `mode: restart`: a press while it's
 * still blinking cancels and restarts it. A burst of same-signal presses
 * (e.g. one Claude session flapping thinking/idle every poll) would then
 * never let it finish a single visible blink cycle. This cooldown drops
 * presses that arrive before the previous blink for that entity would have
 * finished, per entity, so unrelated signals (finished vs needs_user) never
 * block each other.
 */
const BUTTON_PRESS_COOLDOWN_MS = 2000;
const lastPressAtByEntityId = new Map<string, number>();

function pressButton(entityId: string): void {
  if (!haUrl || !haToken) {
    return;
  }

  const now = Date.now();
  const lastPressAt = lastPressAtByEntityId.get(entityId);
  if (
    lastPressAt !== undefined &&
    now - lastPressAt < BUTTON_PRESS_COOLDOWN_MS
  ) {
    return;
  }
  lastPressAtByEntityId.set(entityId, now);

  pressHomeAssistantButton(haUrl, haToken, entityId).catch((error: unknown) => {
    console.error("Home Assistant button press failed:", error);
  });
}

/**
 * Tracks the last seen per-session status for one family, presses the Home
 * Assistant flash button for each session that just went `thinking` -> `idle`
 * (green, "finished") or crossed into `needs_user` (yellow, "waiting on you"),
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
      pressButton(haFinishedButtonEntityId);
    }

    const needsUser = detectNeedsUserSessions(lastSnapshotBySession, snapshots);
    for (const session of needsUser) {
      console.log(
        `session_needs_user: ${session.family}/${session.label} (${session.sourceSessionId})`,
      );
      pressButton(haNeedsUserButtonEntityId);
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
