#!/bin/bash
# Runs once at login (see src/runtime/healthCheckAgent.ts) and checks the
# three things that have each independently broken before: the bridge
# LaunchAgent not being bootstrapped by launchd, and the Codex/Claude global
# hook registrations going missing (e.g. wiped by a settings.json rewrite).
# Plain bash on purpose: it must still be able to report a broken bridge even
# if the bridge's own dist/ build is stale or missing.
set -euo pipefail

failures=()

if ! launchctl print "gui/$(id -u)/com.ambientmatrix.agentstatusbridge" >/dev/null 2>&1; then
  failures+=("LaunchAgent not running")
fi

if ! grep -q "codex-hook-capture.mjs" "$HOME/.codex/hooks.json" 2>/dev/null; then
  failures+=("Codex hooks missing")
fi

if ! grep -q "claude-hook-capture.mjs" "$HOME/.claude/settings.json" 2>/dev/null; then
  failures+=("Claude hooks missing")
fi

if [ ${#failures[@]} -gt 0 ]; then
  message=$(IFS=", "; echo "${failures[*]}")
  osascript -e "display notification \"$message\" with title \"Ambient lamp bridge\" sound name \"Basso\""
fi
