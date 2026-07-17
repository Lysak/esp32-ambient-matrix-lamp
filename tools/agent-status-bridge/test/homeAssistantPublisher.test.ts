import { describe, expect, it } from "vitest";
import { createHomeAssistantPublisher } from "../src/publishers/homeAssistantPublisher.js";

describe("createHomeAssistantPublisher", () => {
  it("maps the status and calls setOption with the configured entity id", async () => {
    const calls: Array<{ entityId: string; option: string }> = [];

    const publisher = createHomeAssistantPublisher({
      entityId: "input_select.agent_status",
      setOption: async (entityId, option) => {
        calls.push({ entityId, option });
      },
    });

    await publisher.publishGlobalStatus("needs_user");

    expect(calls).toEqual([
      { entityId: "input_select.agent_status", option: "needs_user" },
    ]);
  });

  it("maps 'idle' to the 'inactive' HA option before calling setOption", async () => {
    const calls: Array<{ entityId: string; option: string }> = [];

    const publisher = createHomeAssistantPublisher({
      entityId: "input_select.agent_status",
      setOption: async (entityId, option) => {
        calls.push({ entityId, option });
      },
    });

    await publisher.publishGlobalStatus("idle");

    expect(calls).toEqual([
      { entityId: "input_select.agent_status", option: "inactive" },
    ]);
  });

  it("propagates a rejection from setOption to the caller", async () => {
    const publisher = createHomeAssistantPublisher({
      entityId: "input_select.agent_status",
      setOption: async () => {
        throw new Error("network down");
      },
    });

    await expect(publisher.publishGlobalStatus("error")).rejects.toThrow(
      "network down",
    );
  });
});
