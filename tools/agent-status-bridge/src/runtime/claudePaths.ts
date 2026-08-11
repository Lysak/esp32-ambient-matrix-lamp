import { homedir } from "node:os";
import { join } from "node:path";

export function getDefaultClaudeEventLogPath(): string {
  return join(
    homedir(),
    "Library",
    "Application Support",
    "ambient-matrix-agent-status",
    "claude-events.log",
  );
}

export function getClaudeEventLogPath(env: NodeJS.ProcessEnv): string {
  return env.CLAUDE_EVENT_LOG_PATH || getDefaultClaudeEventLogPath();
}
