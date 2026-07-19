import { afterEach, beforeEach, describe, expect, it, vi } from "vitest";
import { aggregateFamilyStatus } from "../src/aggregators/aggregateFamilyStatus.js";
import type { Collector } from "../src/collectors/types.js";
import { startFamilyStatusLoop } from "../src/runtime/familyStatusLoop.js";
import type { SourceSessionSnapshot } from "../src/types/source.js";

const makeSnapshot = (
  status: SourceSessionSnapshot["status"],
): SourceSessionSnapshot => ({
  family: "claude",
  sourceSessionId: "session-1",
  label: "session-1",
  status,
  updatedAt: "2026-07-16T00:00:00.000Z",
});

function fakeCollector(
  pollResults: SourceSessionSnapshot[][],
): Collector<SourceSessionSnapshot> {
  let call = 0;
  return {
    async poll() {
      const result = pollResults[Math.min(call, pollResults.length - 1)] ?? [];
      call += 1;
      return result;
    },
  };
}

describe("startFamilyStatusLoop", () => {
  beforeEach(() => {
    vi.useFakeTimers();
  });

  afterEach(() => {
    vi.useRealTimers();
  });

  it("reports the aggregated status on each poll tick", async () => {
    const collector = fakeCollector([[makeSnapshot("thinking")]]);
    const onStatus = vi.fn();

    startFamilyStatusLoop({
      collector,
      aggregate: aggregateFamilyStatus,
      intervalMs: 2000,
      onStatus,
    });

    await vi.advanceTimersByTimeAsync(2000);

    expect(onStatus).toHaveBeenCalledWith("thinking");
  });

  it("does not call onStatus again when the aggregated status has not changed", async () => {
    const collector = fakeCollector([[makeSnapshot("thinking")]]);
    const onStatus = vi.fn();

    startFamilyStatusLoop({
      collector,
      aggregate: aggregateFamilyStatus,
      intervalMs: 2000,
      onStatus,
    });

    await vi.advanceTimersByTimeAsync(2000);
    await vi.advanceTimersByTimeAsync(2000);

    expect(onStatus).toHaveBeenCalledTimes(1);
  });

  it("stops polling once stop() is called", async () => {
    const collector = fakeCollector([[makeSnapshot("thinking")]]);
    const onStatus = vi.fn();

    const stop = startFamilyStatusLoop({
      collector,
      aggregate: aggregateFamilyStatus,
      intervalMs: 2000,
      onStatus,
    });

    stop();
    await vi.advanceTimersByTimeAsync(2000);

    expect(onStatus).not.toHaveBeenCalled();
  });
});
