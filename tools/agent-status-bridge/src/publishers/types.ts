import type { AgentFamily, NormalizedStatus } from "../types/status.js";

export interface StatusUpdate {
  family: AgentFamily;
  status: NormalizedStatus;
  changedAt: string;
}

export interface StatusPublisher {
  publish(update: StatusUpdate): Promise<void>;
}
