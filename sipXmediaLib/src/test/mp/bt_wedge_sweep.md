# bt_wedge_sweep

Copyright (C) 2026 SIPez LLC.  All rights reserved.

Drives `wavelock_probe` through repeated real Bluetooth device departures
and reports, per configuration cell, whether teardown wedged and whether
WinMM was usable afterwards. This is the harness that validated the
`MpidWinMM` restructure against the customer's failure mode on real
hardware (2026-09-08 matrix; results in `wavelock_probe.md`).

## Prerequisites

- The laptop, not the build VM: a physical Bluetooth radio, the SB510
  headset (or equivalent), `btaudio_ctl.exe`, and a second, wired capture
  device for the health check (default index 0).
- `btaudio_ctl.exe` needs no elevation.
- The probe built from the same directory (see `wavelock_probe.md`).
- The headset powered on and paired. It powers itself off when idle or
  after repeated disconnects; the sweep aborts rather than accumulate void
  runs, but it cannot turn the device back on. Stay near it.

## What a run does

For each cell x offset x repetition: bring the device to ACTIVE capture
(retrying, aborting the sweep if it will not return), start the probe,
disconnect the device at the configured offset after callbacks falter,
collect the reset and `health:` lines, reconnect, and run the probe's
`--method health` reopen check against the returned device.

## The cells

| Cell | Probe flags | Question it answers |
| --- | --- | --- |
| deferred+joined | `--callback-shape deferred --reset-context joined` | Does the fix's exact teardown sequence survive a real departure? |
| deferred+inflight | `--callback-shape deferred --reset-context inflight` | Is it the join that matters, or moving the wave call off the callback? |
| skip(deferred) | `--skip-teardown` | Is the fire-escape path (no teardown at all) safe? |
| reopen+inflight | `--callback-shape reopen --reset-context inflight` | In-session baseline: what sipX does today. Expected to wedge. |

Cells run least-likely-to-wedge first, so a device that stops
reconnecting late in the session costs baseline runs, not the decisive
ones.

## Usage

    ./bt_wedge_sweep.sh <winmm capture index> <name match> [runs per cell]

Get the capture index from `wavelock_probe --dir in --method list`.
Example: `./bt_wedge_sweep.sh 2 SB510`

Offsets, timeouts and the cell list are variables at the top of the
script. If the script was pasted through a Windows editor:
`sed -i 's/\r$//' bt_wedge_sweep.sh`

## Reading the output

One line per run in `summary.txt` (and echoed live), plus a second line
per run for the post-reconnect reopen check:

    deferred+joined  off  250 run 1  reset returned after 0.4 ms  [numdevs OK 0.3 ms (3 devices) open2 OK 36.7 ms audio=1 ]
    deferred+joined  off  250 run 1  health: reopen OK 57.9 ms firstAudioMs=700

- `reset returned` vs `reset WEDGED`: whether teardown completed.
- `numdevs` / `open2`: WinMM health in the same process after the
  trigger -- `BLOCKED` means the WinMM lock was poisoned.
- `health: reopen ... firstAudioMs=N`: the reconnected device delivers
  verified audio N ms after a fresh open. FAIL with the right device name
  on the identity line means no audio within 5 s -- investigate before
  trusting the cell.
- `void: disconnect missed the window`: nothing was measured; the run
  retried once automatically. Repeated voids usually mean the device
  reconnected without the capture profile or the index moved.
- `[process unrecoverable]`: the probe could not exit and was killed.

The per-cell summary at the end counts wedged, clean, void and
unrecoverable runs from the logs.

## Cautions

- The WinMM capture index can move when devices come and go. The
  `--method health` identity line reports what the index resolves to; if
  it is not the expected device, rerun `--method list` and restart with
  the new index.
- A wedged baseline run leaves a thread inside WinMM until the process
  exits; the script kills what cannot exit. Reconnection after a wedge is
  slower and less reliable, which is why the readiness check retries and
  why it demands ACTIVE **capture**, not just any ACTIVE endpoint.
- Runs report per-cell counts, not certainty: with a ~5-10 percent
  natural clean rate in the baseline, treat 3 runs per offset as a
  screen and 10 as decisive.

