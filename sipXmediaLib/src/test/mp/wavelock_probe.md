# wavelock_probe

A standalone diagnostic for Windows WinMM audio lock and deadlock behaviour.
It is not part of the sipXtapi build and is not run by the test suite. It
exists to answer questions about how WinMM behaves under device removal and
thread contention, so that fixes in `MpidWinMM` and `MpodWinMM` are designed
against measured behaviour rather than assumption.

## Why it exists

A customer reported `sipxUnInitialize` hanging for one to two minutes and
then crashing, after a virtual audio device (Sanas) shut down during a call.
Their crash dump showed two threads deadlocked inside WinMM:

```
tid A   MpidWinMM::disableDevice -> waveInReset
          -> holds a winmmbase critical section
          -> blocked in wdmaud waiting for outstanding buffers

tid B   WinMM callback thread -> MpidWinMM::processAudioInput
          -> waveInAddBuffer
          -> blocked waiting for that same critical section
```

`waveInReset` waits for buffers to be returned. Buffers are returned by the
callback. The callback needs the lock that `waveInReset` holds. Neither
proceeds.

Microsoft documents this. From the `waveInProc` reference:

> Applications should not call any system-defined functions from inside a
> callback function, except for `EnterCriticalSection`,
> `LeaveCriticalSection`, ... `SetEvent`, ... **Calling other wave functions
> will cause deadlock.**

`MpidWinMM::processAudioInput` calls `waveInAddBuffer` from inside the
callback. `MpodWinMM` does not -- its callback only pushes to an SLIST and
signals an event, and the wave calls happen on its own `ThreadMMProc`. That
asymmetry is why the output side has never shown this failure.

## What it proves

Measured on the build VM against a real device removal, not a simulation:

| Configuration | `waveInReset` after removal |
| --- | --- |
| `waveInAddBuffer` called in the callback (what sipX does today) | **never returns** -- measured to 300 s |
| `waveInAddBuffer` called on a thread we own (the `MpodWinMM` pattern) | **returns in 0.3 ms** |

Repeated twice each. After a wedge, the process's WinMM input path is
unusable: subsequent calls into WinMM from that process block indefinitely,
reproduced three times.

This is the evidence that restructuring `MpidWinMM` to defer the wave call
prevents the deadlock outright, rather than merely making it survivable.

## Building

From a VS2019 x64 native tools prompt:

```
cl /EHsc /W3 wavelock_probe.cpp winmm.lib cfgmgr32.lib advapi32.lib ole32.lib
```

`ole32.lib` is needed for the MMDevice enumeration in `--method list`.
`advapi32.lib` for the elevation check.

To build from the Cygwin ssh shell in one line, without fighting quoting
across the bash-to-cmd boundary, use `cygpath -d` to get the DOS 8.3 short
form of the vcvars path. It contains no spaces, parentheses or quotes, so
nothing needs escaping:

```bash
cd ~/dev/sipXtapi/sipXmediaLib/src/test/mp && cmd /C "call $(cygpath -d '/cygdrive/c/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Auxiliary/Build/vcvars64.bat') && cl /EHsc /W3 wavelock_probe.cpp winmm.lib cfgmgr32.lib advapi32.lib ole32.lib"
```

If `cygpath -d` returns the long path unchanged, 8.3 name generation is
disabled on the volume and this will not work; fall back to a small `.bat`
file invoked by name.

## Modes

```
wavelock_probe --dir in|out --method <mode> [--device N] [--iterations N]
               [--reset-timeout MS] [--instance "<PnP instance id>"]
```

**`list`** -- prints the WinMM device list and the MMDevice endpoint list
side by side, with endpoint states and what `WAVE_MAPPER` resolves to. Opens
no device. Run this first to get device indices.

**`hold`** -- opens a device and reports callback counts once a second, then
closes. No provocation. Use it to watch what an externally driven device
change does to a live stream. `--iterations` is the number of seconds.

**`removed`** -- waits for an external device removal (see below), then times
`waveInReset` and tries to reopen. This is the mode that reproduces the
customer's hang.

**`removed-deferred`** -- as `removed`, but the wave call runs on a thread
the probe owns instead of in the callback. This is the mode that shows the
restructure working.

**`suspend`** -- suspends the recycling thread and times the reset. Repeatable
on output; wedges the process on input. A proxy for removal, superseded by
`removed`, and retained because it needs no external harness.

**`sweep`** -- as `suspend`, walking the settle delay to show the shape of the
window.

**`reopen`** / **`deferred`** -- wedge via `SuspendThread`, then test whether
fresh opens still deliver audio. Superseded by `removed` / `removed-deferred`.

**`parent`** -- disable and re-enable a devnode per cycle. Needs elevation and
`--instance`. **This mode has never successfully executed past the disable
step:** every audio devnode tested vetoes removal while a stream is open.
Retained for reference; see "What does not work" below.

## Arguments that matter

**`--device N`** -- WinMM device index, default 0. `-1` is `WAVE_MAPPER`.

Do not use `WAVE_MAPPER` for removal testing. The mapper follows the system
default device and re-routes when it changes, which silently masks a removal.
sipX opens a specific index, so a specific index is what should be tested.

**`--reset-timeout MS`** -- how long to wait for `waveInReset` before
abandoning it, default 3000. The default truncates the answer: a wedge
reported as "3006 ms" is the harness giving up, not WinMM returning. Raise it
when measuring how long a wedge actually lasts.

## Reproducing the deadlock

Requires `swdevice_audio.exe`, a companion tool that creates a virtual audio
device node via `SwDeviceCreate` and destroys it on exit. That destruction
sends no `IRP_MN_QUERY_REMOVE_DEVICE`, so the audio driver is never asked for
permission and cannot refuse -- which is what makes it equivalent to hardware
being unplugged, or to a virtual driver such as Sanas shutting down.

```bash
cd ~/dev/sipXtapi/sipXmediaLib/src/test/mp

powershell.exe -ExecutionPolicy Bypass \
  -File 'C:\Users\dpetr\dev\sipXtapi\vbcable\vbcable_devnode.ps1' disable
sleep 3

./swdevice_audio.exe --hold 15 > /tmp/swdev.log 2>&1 &
sleep 3
./wavelock_probe.exe --dir in --method removed --iterations 10 --device 1 --reset-timeout 10000
wait

powershell.exe -ExecutionPolicy Bypass \
  -File 'C:\Users\dpetr\dev\sipXtapi\vbcable\vbcable_devnode.ps1' enable
```

The `disable` step is required: VB-Cable free refuses a second instance, so
its own root devnode must be disabled before `SwDeviceCreate` can start one.

Confirm the device index with `--method list` while `swdevice_audio` is
running -- do not assume it is 1.

The `removed` run will hang after printing the wedge result, because the
process's WinMM path is by then unusable. Ctrl-C is expected. Substitute
`removed-deferred` for the case that completes cleanly.

## What does not work, and why

Every mechanism Windows offers for removing a device through PnP was tested
against a live capture stream. All of them failed, for the same reason: they
send `IRP_MN_QUERY_REMOVE_DEVICE`, and portcls refuses it while pins are
open. That refusal is the mechanism working as designed.

| Target | Method | Result |
| --- | --- | --- |
| MEDIA devnode | `CM_Disable_DevNode` | `CR_REMOVE_VETOED` |
| MEDIA devnode | `CM_Query_And_Remove_SubTree` | `CR_REMOVE_VETOED` |
| MEDIA devnode | `pnputil /remove-device` | forces through, destroys the devnode, needs recovery |
| USB composite parent | `CM_Disable_DevNode` | `CR_REMOVE_VETOED` |
| USB composite parent | `Disable-PnpDevice` | generic WMI failure |
| `SWD\MMDEVAPI` endpoint | `Disable-PnpDevice` | succeeds, but an open stream is unaffected |
| VB-Cable driver service | `sc stop` | `ERROR_INVALID_SERVICE_CONTROL` |

The `SWD\MMDEVAPI` row is worth understanding: an audio endpoint node is an
MMDevice-layer object, and an open `waveIn` handle runs through `wdmaud` to
the KS filter beneath it. Disabling the endpoint affects enumeration and
future opens only. This was verified with the probe opening a specific device
index rather than `WAVE_MAPPER`, so the mapper's re-routing was not masking
the result.

`0x80041001` from `Disable-PnpDevice` is `WBEM_E_FAILED`, a generic WMI
error that swallows the underlying `CONFIGRET`. Do not read it as a confirmed
veto without checking with `CM_Disable_DevNode` directly.

## Using it to test a hypothesis

The probe is built around one pattern: put a device into a known state, do
something to it, and measure what a specific WinMM call does. To test a new
hypothesis, the usual approach is:

1. Add a mode that sets up the state you care about.
2. Run the call under test on a worker thread with `WaitForSingleObject` and
   a timeout, so a block is reported rather than hanging the harness. See
   `resetProc` and `closeInputCtx`.
3. Verify audio positively rather than inferring it. `InCtx` counts callbacks,
   non-empty buffers, and buffers whose content differs from the previous one.
   "Callbacks arriving" alone does not prove capture is working -- a wedged
   path can deliver empty buffers indefinitely.
4. Give each open its own `InCtx`. Contexts are passed to the callback through
   `dwInstance`, so an abandoned context can never touch a later iteration's
   state. An earlier version shared globals and produced results that looked
   like a real pattern but were accumulated damage.

## Cautions

**Wedging is destructive to the process.** A wedged `waveInReset` leaves a
thread inside WinMM forever, and the handle, buffers and thread are
deliberately leaked. The process must be treated as unusable afterwards.

**Destroying a VB-Cable device node under an open stream bugchecks the host.**
This happened twice, and the second crash was during a `removed-deferred` run
that wedges nothing and exits cleanly -- so it is not caused by abandoned
handles or wedged threads. It is what the removal harness does by design.

The faulting module was identified from `C:\Windows\Minidump`:

```
BugCheck 0x3B  SYSTEM_SERVICE_EXCEPTION
  0xc0000005              access violation
  0xfffff80645db5953      faulting instruction
  0xffffdd8b4d5daa70      trap frame

module base 0xfffff80645db0000  size 0x22000  fault at +0x5953
MODULE: vbaudio_cable64_win10.sys
```

Not `portcls.sys`, not `ks.sys`. The bug is in VB-Cable's own teardown path,
which dereferences a bad pointer when its device node is destroyed while a
capture stream is open. sipX neither causes it nor can prevent it.

Two consequences. A stress harness built on VB-Cable removal is not viable --
hundreds of cycles would mean hundreds of bugchecks. And a different virtual
audio driver may well be fine, since this is an implementation bug rather
than something inherent in abrupt removal. Virtual-Audio-Driver is the
obvious candidate, being open source and explicitly built for instances to be
created and destroyed.

If another driver is tried and also bugchecks, compare the fault offset
against the one above: a different offset in a different module is a
different bug, and worth reporting upstream.

Note that `C:\Windows\MEMORY.DMP` is deleted automatically unless the volume
has 25 GB free, which a 47 GB build VM will never have. The small dumps in
`C:\Windows\Minidump` are not subject to that and are what to look for.

**Recovering audio on the VM**, if the device list gets into a bad state:

```powershell
Disable-PnpDevice -InstanceId "<USB composite parent>" -Confirm:$false
Start-Sleep 3
Enable-PnpDevice  -InstanceId "<USB composite parent>" -Confirm:$false
Restart-Service -Name AudioEndpointBuilder -Force
```

Then confirm with:

```
scripts/sipx_test_runner.py --projects sipXmediaLib --filter MpInputDeviceDriverTest
```

`pnputil /remove-device` on an audio devnode is recoverable without a reboot
by this route.

**`testSetup` reports frame-timing derivatives** and is sensitive to host
load and disk pressure. A run immediately after a reboot or with a nearly
full disk will show more failures than a settled machine. It is not a
reliable indicator of audio health on its own.

