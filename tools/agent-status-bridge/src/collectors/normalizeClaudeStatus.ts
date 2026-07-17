import type { NormalizedStatus } from "../types/status.js";

/**
 * "busy", "idle", and "waiting" (with a `waitingFor` reason such as
 * "input needed") have been observed from `claude agents --json --all`.
 * Any other raw status is unconfirmed and maps to "error" so it surfaces
 * loudly instead of being silently treated as idle.
 */
export function normalizeClaudeStatus(rawStatus: string): NormalizedStatus {
  switch (rawStatus) {
    case "busy":
      return "thinking";
    case "idle":
      return "idle";
    case "waiting":
      return "needs_user";
    default:
      return "error";
  }
}
