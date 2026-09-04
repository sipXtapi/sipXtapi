# btaudio_ctl

Connects and disconnects Bluetooth audio devices from the command line,
without touching the radio and without unpairing.

Companion to `wavelock_probe`.  The probe measures what WinMM does when
a device goes away; this decides when it goes away, and in which of the
two ways Windows offers.

## Why

Testing how sipXtapi reacts to an audio device going away needs a device
that can be taken away on cue.  The state Windows reports depends on how
the device goes away, not on which device it is:

| action | endpoint state |
| --- | --- |
| Bluetooth radio switched off | NOTPRESENT |
| device disconnected | UNPLUGGED |
| device powered off | UNPLUGGED |

The customer's virtual audio driver produces UNPLUGGED.  A test that
switches the radio off is exercising a different path, and every
mechanism tried before this one -- devnode disable, `pnputil
/remove-device`, `SwDeviceClose`, VMware USB detach -- produces
NOTPRESENT.

Windows offers no command for the disconnect.  The Settings page and the
Win+K panel are the only ways in, both of which need a human.

Measured on a SoundBot SB510: three endpoints in one container, all
three transition, both directions, in about two seconds.

## Building

From `~/dev/sipXtapi/sipXmediaLib/src/test/mp` in a Cygwin ssh shell on
the build VM, as one line:

```
V=$(cygpath -d '/cygdrive/c/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Auxiliary/Build/vcvars64.bat') && cmd /s /c "call $V && cl /nologo /EHsc /W3 btaudio_ctl.cpp ole32.lib oleaut32.lib"
```

The `cygpath -d` is what makes this work from bash.  It returns the 8.3
short path, which has no spaces and no parentheses, so nothing needs
quoting on the Windows side and Cygwin has nothing to escape.  Writing
the long path directly fails: Cygwin escapes the inner double quotes as
`\"`, which `cmd` does not understand, and the `(x86)` closes an open
parenthesis in any bracketed `if`.

`cmd /s` makes the strip of the outer quotes unconditional.

For a full VS install substitute `Community`; for Build Tools use
`BuildTools`.  The same form builds `wavelock_probe.cpp` and
`swdevice_audio.cpp`, with their own library lists.

No administrator rights are needed to build or to run.

## Using it

```
btaudio_ctl.exe --list
btaudio_ctl.exe --diag "SB510"
btaudio_ctl.exe --status "SB510"
btaudio_ctl.exe --disconnect "SB510"
btaudio_ctl.exe --connect "SB510"
btaudio_ctl.exe --disconnect "SB510" --timeout 20 --quiet
btaudio_ctl.exe --disconnect "SB510" --force
```

`<name>` is any part of the endpoint name, case insensitive.

### --list

Every audio endpoint, its state, and whether it accepts Bluetooth
control.  The BT column reads:

```
YES   the adapter answered for KSPROPSETID_BtAudio
no    the adapter answered, and does not support it
bth?  the adapter id looks like Bluetooth but did not answer
?     no control interface could be reached at all
```

Anything other than YES prints a reason line underneath naming the step
that failed and its HRESULT.

### --diag

The whole topology walk for one device with everything on: connector
counts, adapter device ids, every HRESULT, and both forms of the support
query.  Run this when `--list` says something unexpected, rather than
guessing.

### --disconnect and --connect

All endpoints of the matched device are acted on together.  A Bluetooth
device usually has several -- hands-free capture, hands-free render,
stereo render -- and Windows keeps the link until every one of them is
disconnected, so acting on only the endpoint whose name matched would
appear to work and do nothing.

The driver acknowledges before the link has actually gone, so the tool
polls until the endpoints reach the wanted state or the timeout expires.
Reconnects on real hardware took between 3 and 13 seconds when driven by
the radio; through this path they are quicker, but the default timeout
is 20 seconds.

### Exit codes

```
0  did what was asked
1  bad arguments
2  COM would not start
3  no endpoint matched
4  matched, but no Bluetooth control could be reached
5  the property request failed
6  request accepted but the state did not change in time
7  already in the requested state, nothing to do
```

7 is distinct from 0 on purpose.  A test that wants to observe a
transition needs to know it did not get one.

## How it works

Windows connects Bluetooth audio through the audio driver rather than
through the Bluetooth API, so the control is in the Core Audio stack.
The adapter device behind an endpoint accepts `KSPROPSETID_BtAudio` with
`KSPROPERTY_ONESHOT_DISCONNECT` or `KSPROPERTY_ONESHOT_RECONNECT`.  This
is the route the Settings page uses.

The sequence is the one documented in "Using the IKsControl Interface to
Access Audio Properties":

1. endpoint `IMMDevice` -> `Activate(IDeviceTopology)`
2. `IDeviceTopology::GetConnector` -> `IConnector`
3. `IConnector::GetDeviceIdConnectedTo` -> the adapter device id
4. `IMMDeviceEnumerator::GetDevice(adapter id)` -> adapter `IMMDevice`
5. `adapter->Activate(IID_IKsControl, ...)`
6. `IKsControl::KsProperty` with the one-shot property

Step 5 is on the adapter device, not on the endpoint and not on the
connector's `IPart`.  `IPart::Activate` does not offer `IKsControl` at
all -- its documented list is `IAudioBass`, `IKsJackDescription` and the
like -- and asking it for one returns `E_NOINTERFACE` for every device on
the machine, which reads as "nothing here speaks Bluetooth".  That cost
an afternoon; the code comments say so at the point where it matters.

The idea came from https://github.com/m2jean/ToothTray (BSD-2-Clause).
No code is taken from it.

## Things that bite

**Identifying Bluetooth endpoints.**  By asking the adapter whether it
supports the property set, not by matching hardware id strings.  Windows
does not guarantee those strings, and ToothTray's author says outright
that he cheesed this part.  A non-Bluetooth adapter answers 0x80070492,
`ERROR_SET_NOT_FOUND`, which is a definitive negative.  The adapter id is
still recorded, because when the property query fails it is the only
evidence left about what we were talking to.

**Not every Bluetooth-looking dongle counts.**  A Plantronics BT600 is a
proprietary dongle: it pairs to its own headsets and presents them as USB
audio, so it answers `ERROR_SET_NOT_FOUND` and is not controllable this
way.  A generic adapter using the Windows Bluetooth stack enumerates
under BTHENUM and behaves like a built-in radio.

**The unknown-container placeholder.**
`{00000000-0000-0000-FFFF-FFFFFFFFFFFF}` is what Windows puts in
`PKEY_Device_ContainerId` when the container is not known, and every
device without one carries it.  On the test laptop six unrelated
endpoints shared it.  Grouping by it would mean an operation aimed at one
device hit the others, so it is treated as no container and such
endpoints are matched by name only.

**Re-pairing leaves ghosts.**  After removing and re-pairing a device,
the previous pairing's endpoints stay enumerable as NOTPRESENT with the
same container id, so they are selected alongside the live ones.  They
cannot transition and have no adapter to reach.  They are excluded from
the state poll and from the already-in-that-state check; without that,
every run reports a false timeout and exit code 7 is never seen.

**Profiles connect independently.**  A device can come back as
"Connected music" with A2DP only and no hands-free, in which case there
is no capture endpoint at all while the device looks connected.  A
reconnect from Settings does not necessarily fix it; removing and
re-pairing does.  Anything that brings a test bench up must assert that
the capture endpoint is ACTIVE, not merely that the device is connected.

**Two radios do not coexist.**  Plugging a second Bluetooth adapter in
while the built-in one is running leaves the new one at
`CM_PROB_FAILED_START`.  Disable the built-in first, and expect a
restart.

## Not covered

This produces the customer's event class.  It does not reproduce the
stall that follows it in the customer's dump, where `waveInReset` waits
in `wdmaud` for buffers the driver never completes.  Windows' own
Bluetooth stack tears the stream down cleanly.  See
`wavelock_probe.md` and the internal decisions record for what that
means for validation.

