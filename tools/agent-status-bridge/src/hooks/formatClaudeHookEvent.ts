import type { RawClaudeEvent } from "../types/claudeEvent.js";

export interface FormatClaudeHookEventResult {
  event: RawClaudeEvent;
  logLine: string;
  response: string;
}

function asString(value: unknown): string {
  return typeof value === "string" ? value : "";
}

/**
 * Claude Code hook payloads document `session_id`, `cwd`, and
 * `hook_event_name` (plus event-specific fields we don't need here). No
 * timestamp field is documented, so the caller supplies "now" for
 * testability.
 */
export function formatClaudeHookEvent(
  payload: unknown,
  nowIso: string,
): FormatClaudeHookEventResult {
  const record =
    typeof payload === "object" && payload !== null
      ? (payload as Record<string, unknown>)
      : {};

  const event: RawClaudeEvent = {
    sessionId: asString(record.session_id),
    label: asString(record.cwd),
    type: asString(record.hook_event_name),
    timestamp: nowIso,
  };

  return {
    event,
    logLine: `${JSON.stringify(event)}\n`,
    response: JSON.stringify({ continue: true }),
  };
}
