import type { NormalizedStatus } from "../types/status.js";

/**
 * Codex has no single poll-based status field like `claude agents --json`.
 * Instead it emits lifecycle hook events (documented at
 * developers.openai.com/codex/hooks). For parity with the current
 * `normalizeClaudeStatus` scope, this only distinguishes "working" from
 * "not working", plus "needs_user" since PermissionRequest makes that free.
 * Any other raw event is unconfirmed and maps to "error" so it surfaces
 * loudly instead of being silently treated as idle.
 */
export function normalizeCodexStatus(rawEventType: string): NormalizedStatus {
  switch (rawEventType) {
    case "SessionStart":
    case "Stop":
      return "idle";
    case "UserPromptSubmit":
    case "PreToolUse":
    case "PostToolUse":
    case "SubagentStart":
    case "SubagentStop":
    case "PreCompact":
    case "PostCompact":
      return "thinking";
    case "PermissionRequest":
      return "needs_user";
    default:
      return "error";
  }
}
