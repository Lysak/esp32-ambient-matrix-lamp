import type { SourceSessionSnapshot } from "../types/source.js";
import type { NormalizedStatus } from "../types/status.js";

/**
 * Returns the sessions that just finished a unit of work: their normalized
 * status transitioned `thinking` -> `idle` between the previous poll and the
 * current snapshots. This is the per-session "agent finished" edge.
 *
 * MVP scope: only the clean `thinking` -> `idle` edge counts. Brand new
 * sessions, sessions that disappear, and `needs_user`/`error` transitions do
 * not fire. See docs/superpowers/specs/2026-07-19-agent-finished-lamp-signal-design.md.
 */
export function detectFinishedSessions(
  previous: Map<string, NormalizedStatus>,
  current: SourceSessionSnapshot[],
): SourceSessionSnapshot[] {
  return current.filter(
    (snapshot) =>
      snapshot.status === "idle" &&
      previous.get(snapshot.sourceSessionId) === "thinking",
  );
}
