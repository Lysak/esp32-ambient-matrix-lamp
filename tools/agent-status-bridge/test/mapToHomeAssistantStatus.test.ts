import { describe, expect, it } from "vitest";
import { mapToHomeAssistantStatus } from "../src/publishers/mapToHomeAssistantStatus.js";

describe("mapToHomeAssistantStatus", () => {
  it("maps 'idle' to 'inactive'", () => {
    expect(mapToHomeAssistantStatus("idle")).toBe("inactive");
  });

  it("maps 'thinking' to 'thinking'", () => {
    expect(mapToHomeAssistantStatus("thinking")).toBe("thinking");
  });

  it("maps 'needs_user' to 'needs_user'", () => {
    expect(mapToHomeAssistantStatus("needs_user")).toBe("needs_user");
  });

  it("maps 'error' to 'error'", () => {
    expect(mapToHomeAssistantStatus("error")).toBe("error");
  });

  it("maps the not-yet-produced 'done' to 'inactive' as a provisional default", () => {
    expect(mapToHomeAssistantStatus("done")).toBe("inactive");
  });
});
