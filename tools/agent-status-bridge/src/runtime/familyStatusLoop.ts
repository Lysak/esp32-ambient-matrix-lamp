import type { Collector } from "../collectors/types.js";
import type { SourceSessionSnapshot } from "../types/source.js";
import type { NormalizedStatus } from "../types/status.js";

export interface FamilyStatusLoopDeps {
  collector: Collector<SourceSessionSnapshot>;
  aggregate: (snapshots: SourceSessionSnapshot[]) => NormalizedStatus;
  intervalMs: number;
  onStatus: (status: NormalizedStatus) => void;
}

export type StopFamilyStatusLoop = () => void;

export function startFamilyStatusLoop(
  deps: FamilyStatusLoopDeps,
): StopFamilyStatusLoop {
  let lastStatus: NormalizedStatus | undefined;

  const timer = setInterval(async () => {
    const snapshots = await deps.collector.poll();
    const status = deps.aggregate(snapshots);

    if (status !== lastStatus) {
      lastStatus = status;
      deps.onStatus(status);
    }
  }, deps.intervalMs);

  return () => clearInterval(timer);
}
