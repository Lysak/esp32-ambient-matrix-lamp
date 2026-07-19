import type { AgentFamily, NormalizedStatus } from "./status.js";

export type SessionLifecycleState =
  | "active"
  | "completed"
  | "failed"
  | "idle"
  | "stopped"
  | "waiting";

export interface SourceSessionSnapshot {
  family: AgentFamily;
  sourceSessionId: string;
  label: string;
  status: NormalizedStatus;
  updatedAt: string;
  lifecycleState?: SessionLifecycleState;
  rawEventType?: string;
  turnId?: string;
}
