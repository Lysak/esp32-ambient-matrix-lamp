import type { SourceSessionSnapshot } from "../types/source.js";
import {
  type NormalizedStatus,
  STATUS_PRIORITY_ASCENDING,
} from "../types/status.js";

export function aggregateFamilyStatus(
  snapshots: SourceSessionSnapshot[],
): NormalizedStatus {
  if (snapshots.length === 0) {
    return "idle";
  }

  let highestStatus: NormalizedStatus = "idle";

  for (const snapshot of snapshots) {
    if (
      STATUS_PRIORITY_ASCENDING.indexOf(snapshot.status) >
      STATUS_PRIORITY_ASCENDING.indexOf(highestStatus)
    ) {
      highestStatus = snapshot.status;
    }
  }

  return highestStatus;
}
