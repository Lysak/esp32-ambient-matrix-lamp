import { appendFile, mkdtemp, rm, truncate, writeFile } from "node:fs/promises";
import { tmpdir } from "node:os";
import { join } from "node:path";
import { afterEach, describe, expect, it } from "vitest";
import { createEventLogTailer } from "../src/runtime/eventLogTailer.js";

const directories: string[] = [];

afterEach(async () => {
  await Promise.all(
    directories
      .splice(0)
      .map((directory) => rm(directory, { force: true, recursive: true })),
  );
});

describe("createEventLogTailer", () => {
  it("reads the initial log once, then only appended bytes", async () => {
    const directory = await mkdtemp(join(tmpdir(), "agent-status-bridge-"));
    directories.push(directory);
    const path = join(directory, "events.log");
    await writeFile(path, "first\n");

    const readNew = createEventLogTailer(path);

    expect(await readNew()).toBe("first\n");

    await appendFile(path, "second\n");
    expect(await readNew()).toBe("second\n");
    expect(await readNew()).toBe("");
  });

  it("restarts from the beginning after log truncation", async () => {
    const directory = await mkdtemp(join(tmpdir(), "agent-status-bridge-"));
    directories.push(directory);
    const path = join(directory, "events.log");
    await writeFile(path, "old\n");

    const readNew = createEventLogTailer(path);
    await readNew();
    await truncate(path, 0);
    await appendFile(path, "replacement\n");

    expect(await readNew()).toBe("replacement\n");
  });
});
