# OpenAI Work Louder Palette

Reference page:

- `https://openai.com/uk-UA/supply/co-lab/work-louder/`

Reference section:

- `Ваші агенти — у кольорі`

Source note:

- The page exposes the five status meanings in the UI.
- The page does not publish official RGB values in readable page text.
- The values below were sampled from the visible color swatches on the live page on `2026-07-16` using Playwright screenshot capture plus pixel sampling.
- Treat these values as the project reference palette for lamp parity with the page.

## Status palette

| Status | UI label on page | Hex | RGB | Meaning for lamp MVP |
|---|---|---|---|---|
| Inactive | `Неактивно` | `#E0E0E0` | `rgb(224, 224, 224)` | No active agent work |
| Unread chat | `Непрочитаний чат` | `#9BF396` | `rgb(155, 243, 150)` | New agent output or unread attention state |
| Thinking | `Thinking` | `#9CD5FE` | `rgb(156, 213, 254)` | Agent is actively thinking / processing |
| User approval needed | `Потрібне схвалення користувача / Питання` | `#FFD0B8` | `rgb(255, 208, 184)` | Waiting for user input, approval, or answer |
| Error | `Помилка` | `#FF7373` | `rgb(255, 115, 115)` | Agent failed or needs intervention |

## Lamp mapping notes

- MVP should preserve these colors exactly.
- Brightness can still be capped by lamp firmware and power limits.
- If WS2812 rendering shifts the visual result, keep the hex values as the input target and correct output later with LED calibration, not by changing the reference palette.

## Future idea

Not part of MVP:

- add a second mapping layer from `status -> effect + color`
- examples: steady color for inactive, pulse for thinking, blink for approval needed, alert flash for error
