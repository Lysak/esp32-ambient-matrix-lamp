import type { RawClaudeEvent } from "../types/claudeEvent.js";
import type { SourceSessionSnapshot } from "../types/source.js";
import { normalizeClaudeHookStatus } from "./normalizeClaudeHookStatus.js";
import type { Collector } from "./types.js";

/**
 * A session with no new hook event in this long is dropped from the poll
 * result entirely, not just treated as idle. The event log is machine-wide
 * and never pruned, so without this an abandoned session whose last event
 * was e.g. `Notification` would pin the claude family (and therefore
 * `aggregateGlobalStatus`) at `needs_user` forever.
 */
export const STALE_CLAUDE_SESSION_MS = 30 * 60 * 1000;

/**
 * CodexBar spawns a real, throwaway `claude` CLI session (whose hooks still
 * fire) from this directory every time it falls back from the claude.ai web
 * API (e.g. to check auth, or when web cookies are unavailable). That
 * session is not a user session — it shouldn't move the lamp through its
 * normal thinking/needs_user churn. But if it ends up in `error`, that's
 * CodexBar genuinely failing to reach Claude, which is worth surfacing on
 * the lamp, so only non-error statuses from this cwd are dropped.
 */
const CODEXBAR_CLAUDE_PROBE_CWD_MARKER = "CodexBar/ClaudeProbe";

export interface ClaudeCollectorDeps {
  /** Reads the raw hook event log: one JSON-encoded `RawClaudeEvent` per line. */
  readClaudeEventLog: () => Promise<string>;
  /** Injectable for tests; defaults to the real clock. */
  now?: () => number;
}

export function createClaudeCollector(
  deps: ClaudeCollectorDeps,
): Collector<SourceSessionSnapshot> {
  const now = deps.now ?? (() => Date.now());
  const latestBySession = new Map<string, RawClaudeEvent>();

  return {
    async poll() {
      const raw = await deps.readClaudeEventLog();
      const events: RawClaudeEvent[] = raw
        .split("\n")
        .filter((line) => line.length > 0)
        .flatMap((line) => {
          try {
            return [JSON.parse(line) as RawClaudeEvent];
          } catch {
            return [];
          }
        });

      for (const event of events) {
        if (!event.sessionId) {
          // A hook fired with an empty/missing sessionId (observed from a
          // malformed Claude hook payload). Without this guard it becomes a
          // permanent phantom "session" with status "error" that never gets
          // a follow-up event, poisoning aggregateFamilyStatus forever.
          continue;
        }

        const current = latestBySession.get(event.sessionId);
        if (!current || event.timestamp > current.timestamp) {
          latestBySession.set(event.sessionId, event);
        }
      }

      const currentTime = now();

      return [...latestBySession.values()]
        .filter(
          (event) =>
            currentTime - Date.parse(event.timestamp) <=
            STALE_CLAUDE_SESSION_MS,
        )
        .map((event) => ({
          family: "claude" as const,
          sourceSessionId: event.sessionId,
          label: event.label,
          status: normalizeClaudeHookStatus(event.type),
          updatedAt: event.timestamp,
          rawEventType: event.type,
        }))
        .filter(
          (snapshot) =>
            snapshot.status === "error" ||
            !snapshot.label.includes(CODEXBAR_CLAUDE_PROBE_CWD_MARKER),
        );
    },
  };
}
