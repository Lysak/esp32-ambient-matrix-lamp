import type { NormalizedStatus } from "../types/status.js";

/**
 * Isolates the bridge's internal NormalizedStatus vocabulary from whatever
 * Home Assistant's `input_select.agent_status` entity expects, so either
 * side can grow independently — e.g. splitting "done" into its own HA
 * state later means editing only this table, nothing else in the bridge.
 *
 * "done" is not yet produced by any collector; treated as "inactive" until
 * a future HA state (e.g. "unread") is added for it.
 */
const STATUS_TO_HOME_ASSISTANT_OPTION: Record<NormalizedStatus, string> = {
  idle: "inactive",
  thinking: "thinking",
  needs_user: "needs_user",
  done: "inactive",
  error: "error",
};

export function mapToHomeAssistantStatus(status: NormalizedStatus): string {
  return STATUS_TO_HOME_ASSISTANT_OPTION[status];
}
