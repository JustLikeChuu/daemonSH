#!/usr/bin/env bash
# tests/integration/test_dcheck.sh
#
# Verifies that dcheck correctly counts live dspawn daemons:
#   - Reports 0 when none are running.
#   - Reports the correct count after spawning some.
#
# Runs both dspawn and dcheck through the shell so this also exercises
# PATH resolution for these two programs, not just the standalone binaries.

set -euo pipefail

# Make sure no daemons are lingering from a previous run/test.
pkill -9 -x dspawn 2>/dev/null || true
sleep 1
rm -f dspawn.log

# Baseline: zero daemons running.
OUTPUT=$(printf "dcheck\nexit\n" | timeout 3s ./daemonshell | sed -r "s/\x1b\[[0-9;]*m//g")
if ! echo "$OUTPUT" | grep -F "Live dspawn daemons: 0" > /dev/null; then
  echo "FAIL: expected 0 live daemons with none spawned"
  echo "----- shell output -----"
  echo "$OUTPUT"
  echo "------------------------"
  exit 1
fi

# Spawn two daemons in one shell session, then exit. dspawn returns almost
# instantly; the daemons themselves keep running independently afterward.
printf "dspawn\ndspawn\nexit\n" | timeout 3s ./daemonshell > /dev/null
sleep 1

# Check the count from a fresh shell session.
OUTPUT=$(printf "dcheck\nexit\n" | timeout 3s ./daemonshell | sed -r "s/\x1b\[[0-9;]*m//g")
if ! echo "$OUTPUT" | grep -F "Live dspawn daemons: 2" > /dev/null; then
  echo "FAIL: expected 2 live daemons after spawning 2"
  echo "----- shell output -----"
  echo "$OUTPUT"
  echo "------------------------"
  pkill -9 -x dspawn 2>/dev/null || true
  exit 1
fi

# Clean up so these daemons don't linger for the rest of their ~100s cycle.
pkill -9 -x dspawn 2>/dev/null || true
rm -f dspawn.log

echo "PASS: dcheck correctly counts live dspawn daemons"