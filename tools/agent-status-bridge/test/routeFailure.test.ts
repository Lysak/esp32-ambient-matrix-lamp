import { describe, expect, it } from "vitest";
import { isRouteFailureError } from "../src/runtime/routeFailure.js";

describe("isRouteFailureError", () => {
  it("detects EHOSTUNREACH on the error itself", () => {
    expect(isRouteFailureError({ code: "EHOSTUNREACH" })).toBe(true);
  });

  it("detects ENETUNREACH on the error itself", () => {
    expect(isRouteFailureError({ code: "ENETUNREACH" })).toBe(true);
  });

  it("detects the code nested one level under cause (fetch's TypeError wrapper)", () => {
    const error = new TypeError("fetch failed");
    (error as { cause?: unknown }).cause = { code: "EHOSTUNREACH" };
    expect(isRouteFailureError(error)).toBe(true);
  });

  it("detects the code nested two levels under cause", () => {
    const error = new TypeError("fetch failed");
    (error as { cause?: unknown }).cause = new TypeError("nested", {
      cause: { code: "ENETUNREACH" },
    });
    expect(isRouteFailureError(error)).toBe(true);
  });

  it("returns false for an unrelated error code", () => {
    expect(isRouteFailureError({ code: "ECONNREFUSED" })).toBe(false);
  });

  it("returns false for a plain application error with no code", () => {
    expect(
      isRouteFailureError(new Error("Home Assistant request failed: 401")),
    ).toBe(false);
  });

  it("returns false for non-object values", () => {
    expect(isRouteFailureError("nope")).toBe(false);
    expect(isRouteFailureError(undefined)).toBe(false);
  });
});
