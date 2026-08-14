const ROUTE_FAILURE_CODES = new Set(["EHOSTUNREACH", "ENETUNREACH"]);

/**
 * `fetch` wraps a network-layer error in a `TypeError` whose `.cause` is
 * Node's underlying error (sometimes itself wrapped again), so the actual
 * `EHOSTUNREACH`/`ENETUNREACH` code can be nested a few levels deep.
 */
export function isRouteFailureError(error: unknown): boolean {
  let current: unknown = error;
  for (let depth = 0; depth < 5 && current != null; depth++) {
    const code = (current as { code?: unknown }).code;
    if (typeof code === "string" && ROUTE_FAILURE_CODES.has(code)) {
      return true;
    }
    current = (current as { cause?: unknown }).cause;
  }
  return false;
}
