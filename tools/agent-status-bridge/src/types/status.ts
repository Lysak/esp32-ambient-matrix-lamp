export const AGENT_FAMILIES = ["codex", "claude"] as const;

export type AgentFamily = (typeof AGENT_FAMILIES)[number];

export const NORMALIZED_STATUSES = [
  "idle",
  "thinking",
  "needs_user",
  "done",
  "error",
] as const;

export type NormalizedStatus = (typeof NORMALIZED_STATUSES)[number];

export const STATUS_PRIORITY_ASCENDING = [
  "idle",
  "done",
  "thinking",
  "needs_user",
  "error",
] as const satisfies readonly NormalizedStatus[];
