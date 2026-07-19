import type { RawCodexEvent } from "../types/codexEvent.js";

export interface FormatCodexHookEventResult {
  event: RawCodexEvent;
  logLine: string;
  response: string;
}

function asString(value: unknown): string {
  return typeof value === "string" ? value : "";
}

/**
 * Codex hook payloads document `session_id`, `cwd`, `hook_event_name` (plus
 * `model` and event-specific fields we don't need here). No timestamp field
 * is documented, so the caller supplies "now" for testability.
 */
export function formatCodexHookEvent(
  payload: unknown,
  nowIso: string,
): FormatCodexHookEventResult {
  const record =
    typeof payload === "object" && payload !== null
      ? (payload as Record<string, unknown>)
      : {};

  const event: RawCodexEvent = {
    sessionId: asString(record.session_id),
    label: asString(record.cwd),
    type: asString(record.hook_event_name),
    timestamp: nowIso,
    turnId: asString(record.turn_id),
  };

  return {
    event,
    logLine: `${JSON.stringify(event)}\n`,
    response: JSON.stringify({ continue: true }),
  };
}
