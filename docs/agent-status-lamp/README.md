# Agent Status Lamp

This document set defines the lamp status-indicator direction inspired by the OpenAI x Work Louder `Codex Micro` page.

Scope for this doc set:

- preserve the exact status colors sampled from the reference page
- plan the MVP status flow through Home Assistant
- keep Mac-side agent event capture as a separate analysis topic
- keep `effect + color` mapping as a future version, not part of the MVP

Files:

- [`openai-worklouder-palette.md`](openai-worklouder-palette.md) — exact status palette and meanings
- [`home-assistant-mvp-plan.md`](home-assistant-mvp-plan.md) — MVP plan for sending statuses to the lamp via Home Assistant
- [`mac-agent-status-bridge-design.md`](mac-agent-status-bridge-design.md) — Mac-side architecture, runtime choices, and phased bridge plan

Out of scope for now:

- implementing the Mac event-capture bridge
- implementing firmware changes for animated status effects
- replacing the sampled palette with a custom palette
