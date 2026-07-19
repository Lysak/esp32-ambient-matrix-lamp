import { existsSync, mkdirSync, readFileSync, writeFileSync } from "node:fs";
import { homedir } from "node:os";
import { dirname, join, resolve } from "node:path";

export interface CodexHooksConfig {
  hooks: Record<
    string,
    Array<{
      matcher: string;
      hooks: Array<{
        type: "command";
        command: string;
        timeout: number;
      }>;
    }>
  >;
}

export const MANAGED_CODEX_HOOK_EVENTS = [
  "SessionStart",
  "UserPromptSubmit",
  "PreToolUse",
  "PostToolUse",
  "SubagentStart",
  "SubagentStop",
  "PreCompact",
  "PostCompact",
  "PermissionRequest",
  "Stop",
] as const;

export function getCodexHookScriptCommand(scriptPath: string): string {
  return `${JSON.stringify(process.execPath)} ${JSON.stringify(scriptPath)}`;
}

export function buildGlobalCodexHooksConfig(
  scriptPath: string,
  existingConfig?: Partial<CodexHooksConfig>,
): CodexHooksConfig {
  const command = getCodexHookScriptCommand(scriptPath);
  const preservedHooks =
    typeof existingConfig?.hooks === "object" && existingConfig.hooks !== null
      ? { ...existingConfig.hooks }
      : {};

  for (const eventName of MANAGED_CODEX_HOOK_EVENTS) {
    preservedHooks[eventName] = [
      {
        matcher: ".*",
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

  return { hooks: preservedHooks };
}

export function getGlobalCodexHooksConfigPath(): string {
  return join(homedir(), ".codex", "hooks.json");
}

export function getDefaultCodexHookScriptPath(scriptDir: string): string {
  return resolve(scriptDir, "codex-hook-capture.mjs");
}

export function readExistingHooksConfig(
  configPath: string,
): Partial<CodexHooksConfig> {
  if (!existsSync(configPath)) {
    return {};
  }

  try {
    return JSON.parse(
      readFileSync(configPath, "utf8"),
    ) as Partial<CodexHooksConfig>;
  } catch (error) {
    throw new Error(
      `Failed to parse existing hooks config at ${configPath}: ${String(error)}`,
    );
  }
}

export function installGlobalCodexHooks({
  configPath = getGlobalCodexHooksConfigPath(),
  scriptPath,
}: {
  configPath?: string;
  scriptPath: string;
}): { configPath: string; scriptPath: string } {
  const existingConfig = readExistingHooksConfig(configPath);
  const config = buildGlobalCodexHooksConfig(scriptPath, existingConfig);
  mkdirSync(dirname(configPath), { recursive: true });
  writeFileSync(configPath, `${JSON.stringify(config, null, 2)}\n`);
  return { configPath, scriptPath };
}
