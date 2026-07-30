import type { SourceSessionSnapshot } from "../types/source.js";

/**
 * Returns the sessions that just started waiting on the user: their
 * normalized status crossed into `needs_user` between the previous poll and
 * the current snapshots. This is the per-session "agent is asking me
 * something" edge, mirroring `detectFinishedSessions`.
 */
export function detectNeedsUserSessions(
  previous: Map<string, SourceSessionSnapshot>,
  current: SourceSessionSnapshot[],
): SourceSessionSnapshot[] {
  return current.filter((snapshot) => {
    if (snapshot.status !== "needs_user") {
      return false;
    }

    const prior = previous.get(snapshot.sourceSessionId);
    return prior !== undefined && prior.status !== "needs_user";
  });
}
