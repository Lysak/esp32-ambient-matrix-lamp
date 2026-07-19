import { homedir } from "node:os";
import { join } from "node:path";

export function getDefaultCodexEventLogPath(): string {
  return join(
    homedir(),
    "Library",
    "Application Support",
    "ambient-matrix-agent-status",
    "codex-events.log",
  );
}

export function getCodexEventLogPath(env: NodeJS.ProcessEnv): string {
  return env.CODEX_EVENT_LOG_PATH || getDefaultCodexEventLogPath();
}
