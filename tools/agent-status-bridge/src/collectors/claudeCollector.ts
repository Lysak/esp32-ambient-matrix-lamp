import { execFile } from "node:child_process";
import { promisify } from "node:util";
import type { SourceSessionSnapshot } from "../types/source.js";
import { normalizeClaudeStatus } from "./normalizeClaudeStatus.js";
import type { Collector } from "./types.js";

const execFileAsync = promisify(execFile);

interface RawClaudeSession {
  sessionId: string;
  name: string;
  status: string;
}

export interface ClaudeCollectorDeps {
  runClaudeAgentsJson: () => Promise<string>;
}

export function createClaudeCollector(
  deps: ClaudeCollectorDeps,
): Collector<SourceSessionSnapshot> {
  return {
    async poll() {
      const raw = await deps.runClaudeAgentsJson();
      const sessions: RawClaudeSession[] = JSON.parse(raw);
      const polledAt = new Date().toISOString();

      return sessions.map((session) => ({
        family: "claude",
        sourceSessionId: session.sessionId,
        label: session.name,
        status: normalizeClaudeStatus(session.status),
        updatedAt: polledAt,
      }));
    },
  };
}

export async function runClaudeAgentsJson(): Promise<string> {
  const { stdout } = await execFileAsync("claude", [
    "agents",
    "--json",
    "--all",
  ]);
  return stdout;
}
