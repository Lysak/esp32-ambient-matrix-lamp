import type { NormalizedStatus } from "../types/status.js";
import { mapToHomeAssistantStatus } from "./mapToHomeAssistantStatus.js";

export interface HomeAssistantPublisherDeps {
  entityId: string;
  setOption: (entityId: string, option: string) => Promise<void>;
}

export interface HomeAssistantPublisher {
  publishGlobalStatus(status: NormalizedStatus): Promise<void>;
}

export function createHomeAssistantPublisher(
  deps: HomeAssistantPublisherDeps,
): HomeAssistantPublisher {
  return {
    async publishGlobalStatus(status: NormalizedStatus) {
      const option = mapToHomeAssistantStatus(status);
      await deps.setOption(deps.entityId, option);
    },
  };
}

/**
 * Calls Home Assistant's REST API to set an `input_select` entity's current
 * option. Not unit-tested directly — it is a thin wrapper around the
 * platform `fetch`, exercised via the `main.ts` manual verification step
 * against a real Home Assistant instance instead.
 */
export async function setInputSelectOption(
  baseUrl: string,
  token: string,
  entityId: string,
  option: string,
): Promise<void> {
  const response = await fetch(
    `${baseUrl}/api/services/input_select/select_option`,
    {
      method: "POST",
      headers: {
        Authorization: `Bearer ${token}`,
        "Content-Type": "application/json",
      },
      body: JSON.stringify({ entity_id: entityId, option }),
    },
  );

  if (!response.ok) {
    throw new Error(
      `Home Assistant request failed: ${response.status} ${response.statusText}`,
    );
  }
}
