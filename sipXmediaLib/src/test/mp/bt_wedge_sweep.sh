#!/bin/bash
#
# Copyright (C) 2026 SIPez LLC.  All rights reserved.
#
# bt_wedge_sweep.sh -- matrix sweep: does the proposed fix's threading
# model survive a real device departure, measured against in-session
# baselines, with WinMM health checked after every run.
#
# Cells, in run order (least likely to wedge first, so a dying device
# late in the session cannot cost the decisive early cells):
#
#   deferred+joined    the fix: recycle on worker, join worker, then
#                      reset.  Predicted clean.
#   deferred+inflight  worker recycles, reset issued without joining.
#                      Isolates join vs thread placement.
#   skip               no reset/close at all; straight to health.
#                      Pre-answers the leak path if the fix wedges.
#   reopen+inflight    sipX today.  In-session baseline, replaces the
#                      Sept 4 cross-session comparison.  Expected wedge.
#
# ASSUMED PROBE INTERFACE -- probe edits must match or this script is
# the spec to reconcile against:
#
#   --callback-shape reopen|deferred   where waveInAddBuffer runs
#   --reset-context  inflight|joined   joined = park+join the wave-
#                                      calling thread before reset
#   --skip-teardown                    on trigger: no reset, no close,
#                                      no unprepare, no free
#   health phase, every run, printed as single lines:
#       health: numdevs OK <ms>   | health: numdevs BLOCKED
#       health: open2 OK <ms>     | health: open2 FAIL <mmres>
#   post-reconnect reopen check, invoked separately:
#       wavelock_probe --dir in --method health --device <N>
#     prints:  health: reopen OK <ms> | health: reopen FAIL <mmres>
#
# A wedged probe never exits (abandoned reset holds the WinMM lock and
# the health numdevs blocks).  The script waits a grace period past the
# reset timeout and kills it; the kill is itself part of the result.
#
# Usage: ./bt_wedge_sweep.sh <winmm capture index> <name match> [runs]
#
# Get the capture index from:  ./wavelock_probe.exe --dir in --method list
#
# If the script was pasted through a Windows editor:  sed -i 's/\r$//'

DEVICE=${1:?winmm capture index, from wavelock_probe --method list}
MATCH=${2:?part of the device name, e.g. SB510}
RUNS=${3:-3}

# shape:context:skip  in run order
CELLS="deferred:joined:0 deferred:inflight:0 deferred:joined:1 reopen:inflight:0"

OFFSETS="250 5000"
RESET_TIMEOUT_MS=60000
GRACE_SECONDS=90
LOGDIR="sweep_$(date +%Y%m%d_%H%M)"
SUMMARY="$LOGDIR/summary.txt"

mkdir -p "$LOGDIR"
echo "device $DEVICE  match '$MATCH'  runs $RUNS per cell  logs in $LOGDIR"
echo

# Bring the device to ACTIVE or die.  $1 = log file.
ensure_active() {
  local log=$1 attempt
  for attempt in 1 2 3; do
    ./btaudio_ctl.exe --connect "$MATCH" --timeout 45 --quiet >> "$log" 2>&1
    sleep 3
    if ./btaudio_ctl.exe --status "$MATCH" 2>/dev/null | grep '^ACTIVE' | grep -q 'capture'; then
      return 0
    fi
    echo "  connect attempt $attempt did not bring the device back" >> "$log"
    sleep 10
  done
  return 1
}

# One measured run.  Sets $result and $void.  $1..$5 = shape ctx skip offset log
one_run() {
  local shape=$1 ctx=$2 skip=$3 offset=$4 log=$5
  local extra="" probe waited killed

  [ "$skip" = "1" ] && extra="--skip-teardown"

  ./wavelock_probe.exe --dir in --method removed --device "$DEVICE" \
      --callback-shape "$shape" --reset-context "$ctx" $extra \
      --close-after "$offset" --reset-timeout "$RESET_TIMEOUT_MS" \
      --iterations 1 >> "$log" 2>&1 &
  probe=$!

  # The probe measures the callback rate for 2 s before it will look
  # for the trigger, so do not disconnect before then.
  sleep 4
  ./btaudio_ctl.exe --disconnect "$MATCH" --quiet >> "$log" 2>&1

  waited=0
  while kill -0 $probe 2>/dev/null && [ $waited -lt $GRACE_SECONDS ]; do
    sleep 2
    waited=$((waited + 2))
  done

  killed=0
  if kill -0 $probe 2>/dev/null; then
    kill -9 $probe 2>/dev/null
    wait $probe 2>/dev/null
    killed=1
    echo "  (probe killed: wedged and unable to exit)" >> "$log"
  else
    wait $probe 2>/dev/null
  fi

  void=0
  if [ "$skip" = "1" ]; then
    result="teardown skipped"
  else
    result=$(grep -m1 '  reset ' "$log" | tr -d '\r' | sed 's/^ *//')
    if [ -z "$result" ]; then
      if grep -q 'never faltered' "$log"; then
        result="void: disconnect missed the window"
        void=1
      else
        result="no reset line -- see $log"
      fi
    fi
  fi

  # In-probe health lines (numdevs, second-device open).  A killed
  # probe may have printed neither, which is itself the answer.
  local h
  h=$(grep 'health:' "$log" | tr -d '\r' | sed 's/^ *health: //' | tr '\n' ' ')
  [ -n "$h" ] && result="$result  [$h]"
  if [ $killed -eq 1 ]; then
    result="$result  [process unrecoverable]"
  fi
}

for cell in $CELLS; do
  shape=${cell%%:*}
  rest=${cell#*:}
  ctx=${rest%%:*}
  skip=${rest#*:}
  tag="${shape}+${ctx}"
  [ "$skip" = "1" ] && tag="skip(${shape})"

  for offset in $OFFSETS; do
    for run in $(seq 1 "$RUNS"); do
      log="$LOGDIR/${tag}_off${offset}_run${run}.log"

      # A run that starts on an UNPLUGGED device is void: nothing
      # falters, so nothing is measured.  After a wedged run the
      # reconnect can take well past the tool's default timeout, so
      # retry, and stop the sweep rather than let it produce twenty
      # void runs that all look like results.
      if ! ensure_active "$log"; then
        echo "$tag off $offset run $run  ABORTED: device would not reconnect" \
            | tee -a "$SUMMARY"
        echo "check that the device is powered on, then rerun"
        exit 1
      fi

      one_run "$shape" "$ctx" "$skip" "$offset" "$log"

      # One automatic retry for a void run, so a missed window does
      # not consume a matrix slot.
      if [ $void -eq 1 ]; then
        echo "  (void, retrying once)" >> "$log"
        if ensure_active "$log"; then
          one_run "$shape" "$ctx" "$skip" "$offset" "$log"
        fi
      fi

      printf "%-18s off %5s run %d  %s\n" "$tag" "$offset" "$run" "$result" \
          | tee -a "$SUMMARY"

      # Reconnect now so the post-reconnect reopen check runs against
      # a returned device; also positions the next run.
      ./btaudio_ctl.exe --connect "$MATCH" --timeout 45 --quiet >> "$log" 2>&1
      sleep 5
      if ./btaudio_ctl.exe --status "$MATCH" 2>/dev/null | grep -q '^ACTIVE'; then
        ./wavelock_probe.exe --dir in --method health --device "$DEVICE" \
            >> "$log" 2>&1
        reopen=$(grep 'health: reopen' "$log" | tail -1 | tr -d '\r' | sed 's/^ *//')
        [ -n "$reopen" ] && printf "%-18s off %5s run %d  %s\n" \
            "$tag" "$offset" "$run" "$reopen" | tee -a "$SUMMARY"
      else
        printf "%-18s off %5s run %d  reopen SKIPPED: not ACTIVE yet\n" \
            "$tag" "$offset" "$run" | tee -a "$SUMMARY"
      fi
    done
  done
done

echo
echo "summary by cell"
for cell in $CELLS; do
  shape=${cell%%:*}
  rest=${cell#*:}
  ctx=${rest%%:*}
  skip=${rest#*:}
  tag="${shape}+${ctx}"
  [ "$skip" = "1" ] && tag="skip(${shape})"
  wedged=$(grep -l 'WEDGED' "$LOGDIR/${tag}"_*.log 2>/dev/null | wc -l)
  clean=$(grep -l 'reset returned' "$LOGDIR/${tag}"_*.log 2>/dev/null | wc -l)
  voids=$(grep -l 'never faltered' "$LOGDIR/${tag}"_*.log 2>/dev/null | wc -l)
  killed=$(grep -l 'unable to exit' "$LOGDIR/${tag}"_*.log 2>/dev/null | wc -l)
  printf "  %-18s wedged %2d  clean %2d  void %2d  unrecoverable %2d\n" \
      "$tag" "$wedged" "$clean" "$voids" "$killed"
done
echo
echo "full per-run lines in $SUMMARY"
