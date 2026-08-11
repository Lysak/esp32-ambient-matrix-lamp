import type { SourceSessionSnapshot } from "../types/source.js";
/**
 * Returns the sessions that just finished a unit of work: their normalized
 * state crossed the "turn finished" edge between the previous poll and the
 * current snapshots. This is the per-session "agent finished" edge.
 *
 * Both families are hook-log based (`rawEventType` set): a newly observed
 * `Stop` hook event for the session counts as finished, even if the poll
 * loop missed the intermediate `thinking` snapshot. Codex additionally
 * carries a `turnId`, so it's compared too when present. Any snapshot
 * without `rawEventType` (none currently) falls back to the clean
 * `thinking` -> `idle` edge.
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

    if (snapshot.rawEventType !== undefined) {
      return (
        snapshot.rawEventType === "Stop" &&
        (snapshot.turnId !== prior.turnId ||
          snapshot.updatedAt !== prior.updatedAt)
      );
    }

    return snapshot.status === "idle" && prior.status === "thinking";
  });
}
