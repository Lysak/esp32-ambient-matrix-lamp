import type { RawCodexEvent } from "../types/codexEvent.js";
import type { SourceSessionSnapshot } from "../types/source.js";
import { normalizeCodexStatus } from "./normalizeCodexStatus.js";
import type { Collector } from "./types.js";

export interface CodexCollectorDeps {
  /** Reads the raw hook event log: one JSON-encoded `RawCodexEvent` per line. */
  readCodexEventLog: () => Promise<string>;
}

export function createCodexCollector(
  deps: CodexCollectorDeps,
): Collector<SourceSessionSnapshot> {
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
        const current = latestBySession.get(event.sessionId);
        if (!current || event.timestamp > current.timestamp) {
          latestBySession.set(event.sessionId, event);
        }
      }

      return [...latestBySession.values()].map((event) => ({
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
