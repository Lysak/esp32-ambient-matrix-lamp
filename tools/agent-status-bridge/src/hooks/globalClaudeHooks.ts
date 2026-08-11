import { existsSync, readFileSync, writeFileSync } from "node:fs";
import { homedir } from "node:os";
import { join, resolve } from "node:path";
import { getCodexHookScriptCommand } from "./globalCodexHooks.js";

interface ClaudeHookEntry {
  matcher?: string;
  hooks: Array<{
    type: "command";
    command: string;
    timeout?: number;
  }>;
}

export interface ClaudeSettings {
  hooks: Record<string, ClaudeHookEntry[]>;
  [otherKey: string]: unknown;
}

/**
 * Every event fires our hook unconditionally (`matcher: ".*"`) except
 * `Notification`, which Claude Code also emits for non-actionable
 * notifications (e.g. idle-timeout nudges). Restricting it to
 * `permission_prompt` keeps `Notification` events in the log a confirmed
 * "needs user" signal, matching `normalizeClaudeHookStatus`.
 */
const MANAGED_CLAUDE_HOOK_EVENTS: ReadonlyArray<{
  event: string;
  matcher: string;
}> = [
  { event: "SessionStart", matcher: ".*" },
  { event: "UserPromptSubmit", matcher: ".*" },
  { event: "PreToolUse", matcher: ".*" },
  { event: "PostToolUse", matcher: ".*" },
  { event: "Stop", matcher: ".*" },
  { event: "Notification", matcher: "permission_prompt" },
];

export function buildGlobalClaudeHooksConfig(
  scriptPath: string,
  existingConfig?: Partial<ClaudeSettings>,
): ClaudeSettings {
  const command = getCodexHookScriptCommand(scriptPath);
  const { hooks: existingHooks, ...otherKeys } = existingConfig ?? {};
  const preservedHooks =
    typeof existingHooks === "object" && existingHooks !== null
      ? { ...existingHooks }
      : {};

  for (const { event, matcher } of MANAGED_CLAUDE_HOOK_EVENTS) {
    const existingEventHooks = preservedHooks[event] ?? [];
    const unrelatedEventHooks = existingEventHooks.filter(
      ({ hooks }) =>
        !hooks.some(({ command: existingCommand }) =>
          existingCommand.includes(scriptPath),
        ),
    );

    preservedHooks[event] = [
      ...unrelatedEventHooks,
      {
        matcher,
        hooks: [
          {
            type: "command",
            command,
            timeout: 5,
          },
        ],
      },
    ];
  }

  return { ...otherKeys, hooks: preservedHooks };
}

export function getGlobalClaudeHooksConfigPath(): string {
  return join(homedir(), ".claude", "settings.json");
}

export function getDefaultClaudeHookScriptPath(scriptDir: string): string {
  return resolve(scriptDir, "claude-hook-capture.mjs");
}

export function readExistingClaudeSettings(
  configPath: string,
): Partial<ClaudeSettings> {
  if (!existsSync(configPath)) {
    return {};
  }

  try {
    return JSON.parse(
      readFileSync(configPath, "utf8"),
    ) as Partial<ClaudeSettings>;
  } catch (error) {
    throw new Error(
      `Failed to parse existing Claude settings at ${configPath}: ${String(error)}`,
    );
  }
}

export function installGlobalClaudeHooks({
  configPath = getGlobalClaudeHooksConfigPath(),
  scriptPath,
}: {
  configPath?: string;
  scriptPath: string;
}): { configPath: string; scriptPath: string } {
  const existingConfig = readExistingClaudeSettings(configPath);
  const config = buildGlobalClaudeHooksConfig(scriptPath, existingConfig);
  writeFileSync(configPath, `${JSON.stringify(config, null, 2)}\n`);
  return { configPath, scriptPath };
}
