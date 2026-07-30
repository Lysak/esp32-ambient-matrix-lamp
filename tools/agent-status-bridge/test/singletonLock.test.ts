import { describe, expect, it } from "vitest";
import {
  acquireSingletonLock,
  getSingletonLockPath,
  releaseSingletonLock,
  type SingletonLockDeps,
} from "../src/runtime/singletonLock.js";

function createFakeDeps(
  initialFiles: Record<string, string> = {},
  isProcessAlive: (pid: number) => boolean = () => false,
): SingletonLockDeps & { files: Record<string, string> } {
  const files: Record<string, string> = { ...initialFiles };

  return {
    files,
    pathExists: (path) => path in files,
    readFile: (path) => files[path] ?? "",
    writeFile: (path, contents) => {
      files[path] = contents;
    },
    removeFile: (path) => {
      delete files[path];
    },
    makeDir: () => {},
    isProcessAlive,
  };
}

describe("singletonLock", () => {
  it("prefers AGENT_STATUS_BRIDGE_LOCK_PATH override", () => {
    expect(
      getSingletonLockPath({
        AGENT_STATUS_BRIDGE_LOCK_PATH: "/tmp/custom-bridge.pid",
      } as NodeJS.ProcessEnv),
    ).toBe("/tmp/custom-bridge.pid");
  });

  it("returns the macOS machine-wide default path", () => {
    expect(getSingletonLockPath({} as NodeJS.ProcessEnv)).toContain(
      "Library/Application Support/ambient-matrix-agent-status/bridge.pid",
    );
  });

  it("acquires the lock when no lock file exists", () => {
    const deps = createFakeDeps();

    const result = acquireSingletonLock("/lock", 111, deps);

    expect(result).toEqual({ acquired: true });
    expect(deps.files["/lock"]).toBe("111\n");
  });

  it("refuses to acquire when the lock owner is still alive", () => {
    const deps = createFakeDeps({ "/lock": "222\n" }, (pid) => pid === 222);

    const result = acquireSingletonLock("/lock", 111, deps);

    expect(result).toEqual({ acquired: false, existingPid: 222 });
    expect(deps.files["/lock"]).toBe("222\n");
  });

  it("reclaims a stale lock whose owner is no longer running", () => {
    const deps = createFakeDeps({ "/lock": "222\n" }, () => false);

    const result = acquireSingletonLock("/lock", 111, deps);

    expect(result).toEqual({ acquired: true });
    expect(deps.files["/lock"]).toBe("111\n");
  });

  it("releases the lock when it still names the calling process", () => {
    const deps = createFakeDeps({ "/lock": "111\n" });

    releaseSingletonLock("/lock", 111, deps);

    expect(deps.files["/lock"]).toBeUndefined();
  });

  it("does not release a lock owned by a different (newer) process", () => {
    const deps = createFakeDeps({ "/lock": "333\n" });

    releaseSingletonLock("/lock", 111, deps);

    expect(deps.files["/lock"]).toBe("333\n");
  });
});
