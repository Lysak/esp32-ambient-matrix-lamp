import type { RawCodexEvent } from "../types/codexEvent.js";
import type { SourceSessionSnapshot } from "../types/source.js";
import { normalizeCodexStatus } from "./normalizeCodexStatus.js";
import type { Collector } from "./types.js";

/**
 * A session with no new hook event in this long is dropped from the poll
 * result entirely, not just treated as idle. The event log is machine-wide
 * and never pruned, so without this an abandoned session whose last event
 * was e.g. `PermissionRequest` would pin the codex family (and therefore
 * `aggregateGlobalStatus`) at `needs_user` forever.
 */
export const STALE_CODEX_SESSION_MS = 30 * 60 * 1000;

export interface CodexCollectorDeps {
  /** Reads the raw hook event log: one JSON-encoded `RawCodexEvent` per line. */
  readCodexEventLog: () => Promise<string>;
  /** Injectable for tests; defaults to the real clock. */
  now?: () => number;
}

export function createCodexCollector(
  deps: CodexCollectorDeps,
): Collector<SourceSessionSnapshot> {
  const now = deps.now ?? (() => Date.now());

  return {
    async poll() {
      const raw = await deps.readCodexEventLog();
      const events: RawCodexEvent[] = raw
        .split("\n")
        .filter((line) => line.length > 0)
        .flatMap((line) => {
          try {
            return [JSON.parse(line) as RawCodexEvent];
          } catch {
            return [];
          }
        });

      const latestBySession = new Map<string, RawCodexEvent>();
      for (const event of events) {
        if (!event.sessionId) {
          // A hook fired with an empty/missing sessionId (observed from a
          // malformed Codex hook payload). Without this guard it becomes a
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
            currentTime - Date.parse(event.timestamp) <= STALE_CODEX_SESSION_MS,
        )
        .map((event) => ({
          family: "codex" as const,
          sourceSessionId: event.sessionId,
          label: event.label,
          status: normalizeCodexStatus(event.type),
          updatedAt: event.timestamp,
          rawEventType: event.type,
          turnId: event.turnId,
        }));
    },
  };
}
