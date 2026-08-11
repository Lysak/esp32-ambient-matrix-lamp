import type { NormalizedStatus } from "../types/status.js";

/**
 * Claude Code, like Codex, has no single poll-based status field. Instead it
 * emits lifecycle hook events. `Notification` is only wired up for the
 * `permission_prompt` matcher (see `globalClaudeHooks.ts`), so any
 * `Notification` event observed here is confirmed to be a permission
 * request. Any other raw event is unconfirmed and maps to "error" so it
 * surfaces loudly instead of being silently treated as idle.
 */
export function normalizeClaudeHookStatus(
  rawEventType: string,
): NormalizedStatus {
  switch (rawEventType) {
    case "SessionStart":
    case "Stop":
      return "idle";
    case "UserPromptSubmit":
    case "PreToolUse":
    case "PostToolUse":
      return "thinking";
    case "Notification":
      return "needs_user";
    default:
      return "error";
  }
}
