# Home Assistant MVP Plan

## Goal

Send agent status changes to the lamp through Home Assistant while preserving the exact OpenAI x Work Louder reference colors.

## MVP contract

Input:

- some upstream process on the Mac detects agent state changes
- that process sends a normalized status value into Home Assistant

Output:

- Home Assistant updates the lamp to one solid status color
- the lamp remains controllable through the existing ESPHome / Home Assistant path

## Normalized MVP statuses

Use this exact internal set:

- `inactive`
- `unread`
- `thinking`
- `needs_user`
- `error`

These map directly to:

- `inactive` -> `#E0E0E0`
- `unread` -> `#9BF396`
- `thinking` -> `#9CD5FE`
- `needs_user` -> `#FFD0B8`
- `error` -> `#FF7373`

## Recommended architecture

1. Mac-side bridge detects agent state changes.
2. Bridge publishes a single normalized status into Home Assistant.
3. Home Assistant automation translates that status into a lamp command.
4. Lamp shows a solid color status state.

## Recommended Home Assistant shape

Prefer one logical state carrier instead of several booleans.

Candidate entities:

- `input_select.agent_status`
- or `input_text.agent_status`
- or an MQTT-backed sensor if the bridge already speaks MQTT

Current recommendation:

- start with `input_select.agent_status`

Reason:

- easy to inspect in UI
- easy to automate
- constrained allowed values
- no parsing needed inside Home Assistant

## Lamp-side MVP behavior

Prefer the simplest truthful output first:

- lamp turns on if needed
- lamp shows one solid color for the current status
- no animated effect logic in MVP
- no blending between statuses in MVP

This keeps the first version stable and easy to debug.

## Integration boundary

This document does not define yet:

- how the Mac bridge detects Codex / agent states
- whether the bridge reads terminal output, local logs, API events, or app state
- whether transport into Home Assistant is MQTT, webhook, CLI, AppleScript, or local API

That analysis belongs in a separate next-phase document.

## Next docs to add later

- `mac-agent-status-capture.md` — how to detect statuses on the Mac
- `status-effects-v2.md` — how to map statuses to effects and animation rules

## MVP success criteria

- one manually set Home Assistant status updates the lamp to the matching exact reference color
- all five statuses are reproducible on demand
- the lamp can return from status mode to normal ambient mode without rebooting
