import {
  existsSync,
  mkdirSync,
  readFileSync,
  rmSync,
  writeFileSync,
} from "node:fs";
import { homedir } from "node:os";
import { dirname, join } from "node:path";

export function getDefaultSingletonLockPath(): string {
  return join(
    homedir(),
    "Library",
    "Application Support",
    "ambient-matrix-agent-status",
    "bridge.pid",
  );
}

export function getSingletonLockPath(env: NodeJS.ProcessEnv): string {
  return env.AGENT_STATUS_BRIDGE_LOCK_PATH || getDefaultSingletonLockPath();
}

export interface SingletonLockDeps {
  pathExists: (path: string) => boolean;
  readFile: (path: string) => string;
  writeFile: (path: string, contents: string) => void;
  removeFile: (path: string) => void;
  makeDir: (path: string) => void;
  isProcessAlive: (pid: number) => boolean;
}

export const defaultSingletonLockDeps: SingletonLockDeps = {
  pathExists: existsSync,
  readFile: (path) => readFileSync(path, "utf8"),
  writeFile: (path, contents) => writeFileSync(path, contents),
  removeFile: (path) => rmSync(path, { force: true }),
  makeDir: (path) => mkdirSync(path, { recursive: true }),
  isProcessAlive: (pid) => {
    try {
      process.kill(pid, 0);
      return true;
    } catch (error) {
      return (error as NodeJS.ErrnoException).code === "EPERM";
    }
  },
};

export type AcquireSingletonLockResult =
  | { acquired: true }
  | { acquired: false; existingPid: number };

/**
 * Refuses to start a second bridge instance: a stale lock file (owner pid no
 * longer alive) is reclaimed, but a live owner blocks startup. Without this,
 * two independently-running bridges would each detect the same session
 * finishing and each fire the Home Assistant flash button, doubling (or
 * tripling) the lamp notification for a single agent turn.
 */
export function acquireSingletonLock(
  lockPath: string,
  ownPid: number,
  deps: SingletonLockDeps = defaultSingletonLockDeps,
): AcquireSingletonLockResult {
  if (deps.pathExists(lockPath)) {
    const existingPid = Number.parseInt(deps.readFile(lockPath).trim(), 10);

    if (Number.isFinite(existingPid) && deps.isProcessAlive(existingPid)) {
      return { acquired: false, existingPid };
    }
  }

  deps.makeDir(dirname(lockPath));
  deps.writeFile(lockPath, `${ownPid}\n`);
  return { acquired: true };
}

/**
 * Only removes the lock if it still names this process, so a second
 * instance's failed startup can never delete the running owner's lock.
 */
export function releaseSingletonLock(
  lockPath: string,
  ownPid: number,
  deps: SingletonLockDeps = defaultSingletonLockDeps,
): void {
  if (!deps.pathExists(lockPath)) {
    return;
  }

  const existingPid = Number.parseInt(deps.readFile(lockPath).trim(), 10);

  if (existingPid === ownPid) {
    deps.removeFile(lockPath);
  }
}
