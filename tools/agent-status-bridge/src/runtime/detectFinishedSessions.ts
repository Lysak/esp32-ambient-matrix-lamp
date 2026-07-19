import type { SourceSessionSnapshot } from "../types/source.js";
/**
 * Returns the sessions that just finished a unit of work: their normalized
 * state crossed the family-specific "turn finished" edge between the previous
 * poll and the current snapshots. This is the per-session "agent finished"
 * edge.
 *
 * Claude: the clean `thinking` -> `idle` edge.
 * Codex: a newly observed `Stop` hook event for the session, even if the poll
 * loop missed the intermediate `thinking` snapshot.
 */
export function detectFinishedSessions(
  previous: Map<string, SourceSessionSnapshot>,
  current: SourceSessionSnapshot[],
): SourceSessionSnapshot[] {
  return current.filter((snapshot) => {
    const prior = previous.get(snapshot.sourceSessionId);
    if (!prior) {
      return false;
    }

    if (snapshot.family === "codex") {
      return (
        snapshot.rawEventType === "Stop" &&
        (snapshot.turnId !== prior.turnId ||
          snapshot.updatedAt !== prior.updatedAt)
      );
    }

    return snapshot.status === "idle" && prior.status === "thinking";
  });
}
