import type { Collector } from "../collectors/types.js";
import type { SourceSessionSnapshot } from "../types/source.js";
import type { NormalizedStatus } from "../types/status.js";

export interface FamilyStatusLoopDeps {
  collector: Collector<SourceSessionSnapshot>;
  aggregate: (snapshots: SourceSessionSnapshot[]) => NormalizedStatus;
  intervalMs: number;
  onStatus: (status: NormalizedStatus) => void;
  /**
   * Called with the raw snapshots on every poll (not just on change), so
   * callers can run per-session edge detection without polling the source a
   * second time. Optional; the aggregated `onStatus` behavior is unaffected.
   */
  onSnapshots?: (snapshots: SourceSessionSnapshot[]) => void;
}

export type StopFamilyStatusLoop = () => void;

export function startFamilyStatusLoop(
  deps: FamilyStatusLoopDeps,
): StopFamilyStatusLoop {
  let lastStatus: NormalizedStatus | undefined;

  const timer = setInterval(async () => {
    const snapshots = await deps.collector.poll();

    deps.onSnapshots?.(snapshots);

    const status = deps.aggregate(snapshots);

    if (status !== lastStatus) {
      lastStatus = status;
      deps.onStatus(status);
    }
  }, deps.intervalMs);

  return () => clearInterval(timer);
}
