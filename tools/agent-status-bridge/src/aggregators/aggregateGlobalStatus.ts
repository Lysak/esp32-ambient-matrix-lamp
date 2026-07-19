import {
  type NormalizedStatus,
  STATUS_PRIORITY_ASCENDING,
} from "../types/status.js";

export function aggregateGlobalStatus(
  statuses: NormalizedStatus[],
): NormalizedStatus {
  if (statuses.length === 0) {
    return "idle";
  }

  let highestStatus: NormalizedStatus = "idle";

  for (const status of statuses) {
    if (
      STATUS_PRIORITY_ASCENDING.indexOf(status) >
      STATUS_PRIORITY_ASCENDING.indexOf(highestStatus)
    ) {
      highestStatus = status;
    }
  }

  return highestStatus;
}
