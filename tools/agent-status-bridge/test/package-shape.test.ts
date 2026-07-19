import { readFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { describe, expect, it } from "vitest";

const packageJsonPath = fileURLToPath(
  new URL("../package.json", import.meta.url),
);
const packageJson = JSON.parse(readFileSync(packageJsonPath, "utf8")) as {
  engines: { node: string };
  scripts: Record<string, string>;
};

describe("package shape", () => {
  it("pins the expected runtime and tooling scripts", () => {
    expect(packageJson.engines.node).toBe("24.x");
    expect(packageJson.scripts.dev).toBe("tsx watch src/main.ts");
    expect(packageJson.scripts.build).toBe("tsc -p tsconfig.json");
    expect(packageJson.scripts.test).toBe("vitest run");
    expect(packageJson.scripts["install:codex-hooks"]).toBe(
      "node scripts/install-global-codex-hooks.mjs",
    );
    expect(packageJson.scripts.check).toBe(
      "biome check src test scripts package.json tsconfig.json biome.json vitest.config.ts README.md .gitignore .env.example && tsc -p tsconfig.json --noEmit",
    );
  });
});
