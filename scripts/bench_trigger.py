#!/usr/bin/env python3
#
# Copyright (C) 2026 SIPez LLC.  All rights reserved.
#
# bench_trigger.py -- make the bench audio device go away and come back.
#
#   bench_trigger.py status | disconnect | connect
#
# One line of output, and an exit code the tests and the soak act on:
#   0  done
#   1  failed (the line says why)
#   2  bench not configured (config or password file missing/unusable,
#      or trigger = none); unit tests skip loudly, the soak refuses to run
#   3  device not in the expected state after the bounded wait
#
# Configuration lives OUTSIDE the repo so it cannot be committed by
# accident: ~/.sipx_bench/bench.conf (see scripts/bench.conf.example).
# The password file it names must be mode 600.
#
# Implementations:
#   vmware_usb  reconfigure the VM over the vSphere API as a user whose
#               only right is USB add/remove on this one VM. Works with
#               ESXi 5.5 (TLS 1.0) via pyvmomi 8.x on Python 3.9.
#   btaudio     drive btaudio_ctl.exe for a Bluetooth device (laptop).

import configparser
import ctypes
import fcntl
import os
import ssl
import subprocess
import sys
import time

CONF_DIR = os.path.expanduser("~/.sipx_bench")
CONF_PATH = os.path.join(CONF_DIR, "bench.conf")
LOCK_PATH = os.path.join(CONF_DIR, "trigger.lock")
STAMP_PATH = os.path.join(CONF_DIR, "last_trigger")

EXIT_OK, EXIT_FAILED, EXIT_NOT_CONFIGURED, EXIT_WRONG_STATE = 0, 1, 2, 3


def out(msg, code):
    print(msg)
    sys.stdout.flush()
    sys.exit(code)


# ---------------------------------------------------------------- config

def load_config():
    if not os.path.isfile(CONF_PATH):
        out("BENCH NOT CONFIGURED: missing %s" % CONF_PATH, EXIT_NOT_CONFIGURED)
    cfg = configparser.ConfigParser()
    cfg.read(CONF_PATH)
    trigger = cfg.get("bench", "trigger", fallback="none").strip().lower()
    if trigger == "none":
        out("BENCH NOT CONFIGURED: trigger = none in %s" % CONF_PATH,
            EXIT_NOT_CONFIGURED)
    if trigger not in ("vmware_usb", "btaudio"):
        out("BENCH NOT CONFIGURED: unknown trigger '%s' in %s" % (trigger, CONF_PATH),
            EXIT_NOT_CONFIGURED)
    return cfg, trigger


def read_password(cfg):
    path = os.path.expanduser(cfg.get("vmware", "password_file", fallback=""))
    if not path or not os.path.isfile(path):
        out("BENCH NOT CONFIGURED: missing password file %s" % (path or "(unset)"),
            EXIT_NOT_CONFIGURED)
    mode = os.stat(path).st_mode & 0o777
    if mode & 0o077:
        out("BENCH NOT CONFIGURED: %s must be mode 600 (is %o)" % (path, mode),
            EXIT_NOT_CONFIGURED)
    with open(path) as f:
        pw = f.read().strip()
    if not pw:
        out("BENCH NOT CONFIGURED: %s is empty" % path, EXIT_NOT_CONFIGURED)
    return pw


# ---------------------------------------------------------------- guest view

def guest_capture_names():
    """Names of WinMM capture devices as the guest sees them right now."""
    try:
        winmm = ctypes.CDLL("winmm.dll")
    except OSError:
        return None
    n = winmm.waveInGetNumDevs()
    names = []
    for i in range(n):
        caps = ctypes.create_string_buffer(256)
        # WAVEINCAPSA: wMid(2) wPid(2) vDriverVersion(4) szPname[32] ...
        if winmm.waveInGetDevCapsA(ctypes.c_size_t(i), caps, 256) == 0:
            names.append(caps.raw[8:40].split(b"\0", 1)[0].decode("latin-1"))
    return names


def guest_has(match):
    names = guest_capture_names()
    if names is None:
        return None
    return any(match in nm for nm in names)


def wait_guest(match, want_present, timeout_s):
    deadline = time.time() + timeout_s
    while True:
        present = guest_has(match)
        if present is None:
            return None
        if present == want_present:
            return True
        if time.time() >= deadline:
            return False
        time.sleep(0.5)

# ---------------------------------------------------------------- readiness

WAVE_FORMAT_PCM = 1
CALLBACK_EVENT = 0x00050000
WHDR_DONE = 0x00000001
WAIT_OBJECT_0 = 0


class WAVEFORMATEX(ctypes.Structure):
    _pack_ = 1
    _fields_ = [("wFormatTag", ctypes.c_ushort), ("nChannels", ctypes.c_ushort),
                ("nSamplesPerSec", ctypes.c_uint), ("nAvgBytesPerSec", ctypes.c_uint),
                ("nBlockAlign", ctypes.c_ushort), ("wBitsPerSample", ctypes.c_ushort),
                ("cbSize", ctypes.c_ushort)]


class WAVEHDR(ctypes.Structure):
    _fields_ = [("lpData", ctypes.c_void_p), ("dwBufferLength", ctypes.c_uint),
                ("dwBytesRecorded", ctypes.c_uint), ("dwUser", ctypes.c_void_p),
                ("dwFlags", ctypes.c_uint), ("dwLoops", ctypes.c_uint),
                ("lpNext", ctypes.c_void_p), ("reserved", ctypes.c_void_p)]


def guest_capture_index(match):
    names = guest_capture_names()
    if not names:
        return -1
    for i, nm in enumerate(names):
        if match in nm:
            return i
    return -1


def guest_capture_streams(match, timeout_s):
    """'streaming' if the device delivers a buffer within timeout_s,
    'silent' if it opens but delivers nothing, 'absent' if not enumerated,
    'open-failed' otherwise. Enumerated is not the same as usable."""
    idx = guest_capture_index(match)
    if idx < 0:
        return "absent"
    try:
        winmm = ctypes.CDLL("winmm.dll")
        k32 = ctypes.CDLL("kernel32.dll")
    except OSError:
        return "open-failed"
    k32.CreateEventA.restype = ctypes.c_void_p
    k32.WaitForSingleObject.argtypes = [ctypes.c_void_p, ctypes.c_uint]
    k32.CloseHandle.argtypes = [ctypes.c_void_p]
    winmm.waveInOpen.argtypes = [ctypes.POINTER(ctypes.c_void_p), ctypes.c_uint,
                                 ctypes.POINTER(WAVEFORMATEX), ctypes.c_void_p,
                                 ctypes.c_void_p, ctypes.c_uint]
    for fn in ("waveInPrepareHeader", "waveInAddBuffer", "waveInUnprepareHeader"):
        getattr(winmm, fn).argtypes = [ctypes.c_void_p, ctypes.POINTER(WAVEHDR), ctypes.c_uint]
    for fn in ("waveInStart", "waveInReset", "waveInClose"):
        getattr(winmm, fn).argtypes = [ctypes.c_void_p]

    fmt = WAVEFORMATEX(WAVE_FORMAT_PCM, 1, 8000, 16000, 2, 16, 0)
    evt = k32.CreateEventA(None, 0, 0, None)
    h = ctypes.c_void_p()
    if winmm.waveInOpen(ctypes.byref(h), idx, ctypes.byref(fmt), evt, None,
                        CALLBACK_EVENT) != 0:
        k32.CloseHandle(evt)
        return "open-failed"
    bufs = [ctypes.create_string_buffer(320) for _ in range(4)]
    hdrs = [WAVEHDR() for _ in range(4)]
    for b, hd in zip(bufs, hdrs):
        hd.lpData = ctypes.cast(b, ctypes.c_void_p)
        hd.dwBufferLength = 320
        winmm.waveInPrepareHeader(h, ctypes.byref(hd), ctypes.sizeof(WAVEHDR))
        winmm.waveInAddBuffer(h, ctypes.byref(hd), ctypes.sizeof(WAVEHDR))
    winmm.waveInStart(h)
    deadline = time.time() + timeout_s
    got = False
    while time.time() < deadline and not got:
        k32.WaitForSingleObject(evt, 200)
        got = any(hd.dwFlags & WHDR_DONE for hd in hdrs)
    winmm.waveInReset(h)
    for hd in hdrs:
        winmm.waveInUnprepareHeader(h, ctypes.byref(hd), ctypes.sizeof(WAVEHDR))
    winmm.waveInClose(h)
    k32.CloseHandle(evt)
    return "streaming" if got else "silent"


def wait_streaming(match, timeout_s):
    deadline = time.time() + timeout_s
    while True:
        state = guest_capture_streams(match, 2.0)
        if state == "streaming" or time.time() >= deadline:
            return state
        time.sleep(1.0)

# ---------------------------------------------------------------- pacing/lock

def pace(cfg):
    min_gap = cfg.getfloat("bench", "min_interval_s", fallback=5.0)
    try:
        last = float(open(STAMP_PATH).read().strip())
    except (OSError, ValueError):
        last = 0.0
    wait = min_gap - (time.time() - last)
    if wait > 0:
        time.sleep(wait)


def stamp():
    with open(STAMP_PATH, "w") as f:
        f.write("%f\n" % time.time())


class Lock:
    def __enter__(self):
        os.makedirs(CONF_DIR, exist_ok=True)
        self.f = open(LOCK_PATH, "w")
        deadline = time.time() + 60
        while True:
            try:
                fcntl.flock(self.f, fcntl.LOCK_EX | fcntl.LOCK_NB)
                return self
            except OSError:
                if time.time() >= deadline:
                    out("FAILED: another trigger has held %s for 60 s" % LOCK_PATH,
                        EXIT_FAILED)
                time.sleep(0.5)

    def __exit__(self, *a):
        fcntl.flock(self.f, fcntl.LOCK_UN)
        self.f.close()


# ---------------------------------------------------------------- vmware_usb

def vmware_connect(cfg):
    try:
        from pyVim.connect import SmartConnect
        from pyVmomi import vim
    except ImportError:
        out("BENCH NOT CONFIGURED: pyvmomi not installed for this python",
            EXIT_NOT_CONFIGURED)
    host = cfg.get("vmware", "host")
    user = cfg.get("vmware", "user")
    pw = read_password(cfg)
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    ctx.check_hostname = False
    ctx.verify_mode = ssl.CERT_NONE
    ctx.minimum_version = ssl.TLSVersion.TLSv1   # ESXi 5.5
    ctx.set_ciphers("DEFAULT@SECLEVEL=0")
    try:
        si = SmartConnect(host=host, user=user, pwd=pw, sslContext=ctx)
    except Exception as e:  # noqa: BLE001
        out("FAILED: cannot connect to %s as %s: %s" % (host, user, e), EXIT_FAILED)
    return si, vim


def vmware_find(cfg, si, vim):
    name = cfg.get("vmware", "vm_name")
    backing = cfg.get("vmware", "usb_backing")
    view = si.content.viewManager.CreateContainerView(
        si.content.rootFolder, [vim.VirtualMachine], True)
    vm = next((v for v in view.view if v.name == name), None)
    view.Destroy()
    if vm is None:
        out("FAILED: VM '%s' not visible to this user" % name, EXIT_FAILED)
    dev = next((d for d in vm.config.hardware.device
                if isinstance(d, vim.vm.device.VirtualUSB)
                and getattr(d.backing, "deviceName", None) == backing), None)
    return vm, dev, backing


def vmware_wait_task(task, what):
    from pyVmomi import vim
    deadline = time.time() + 60
    while task.info.state in (vim.TaskInfo.State.queued, vim.TaskInfo.State.running):
        if time.time() >= deadline:
            out("FAILED: %s task did not finish in 60 s" % what, EXIT_FAILED)
        time.sleep(0.5)
    if task.info.state != vim.TaskInfo.State.success:
        err = task.info.error.msg if task.info.error else "unknown error"
        out("FAILED: %s: %s" % (what, err), EXIT_FAILED)


def vmware_status(cfg):
    si, vim = vmware_connect(cfg)
    vm, dev, backing = vmware_find(cfg, si, vim)
    match = cfg.get("vmware", "guest_match", fallback=backing)
    hyp = "attached" if dev is not None else "detached"
    g = guest_capture_streams(match, 2.0) if dev is not None else "absent"
    code = EXIT_OK if (dev is not None and g == "streaming") else EXIT_WRONG_STATE
    out("STATUS vmware_usb: hypervisor %s, guest %s, match %s" % (hyp, g, match), code)


def vmware_disconnect(cfg):
    si, vim = vmware_connect(cfg)
    vm, dev, backing = vmware_find(cfg, si, vim)
    match = cfg.get("vmware", "guest_match", fallback=backing)
    if dev is None:
        out("STATUS vmware_usb: already detached", EXIT_WRONG_STATE)
    spec = vim.vm.ConfigSpec()
    change = vim.vm.device.VirtualDeviceSpec()
    change.operation = vim.vm.device.VirtualDeviceSpec.Operation.remove
    change.device = dev
    spec.deviceChange = [change]
    vmware_wait_task(vm.ReconfigVM_Task(spec=spec), "detach")
    stamp()
    gone = wait_guest(match, False, cfg.getfloat("bench", "settle_s", fallback=10.0))
    if gone is False:
        out("DETACHED at hypervisor but guest still enumerates '%s'" % match,
            EXIT_WRONG_STATE)
    out("DETACHED %s" % backing, EXIT_OK)


def vmware_connect_dev(cfg):
    si, vim = vmware_connect(cfg)
    vm, dev, backing = vmware_find(cfg, si, vim)
    match = cfg.get("vmware", "guest_match", fallback=backing)
    if dev is not None:
        out("STATUS vmware_usb: already attached", EXIT_WRONG_STATE)
    usb = vim.vm.device.VirtualUSB()
    usb.backing = vim.vm.device.VirtualUSB.USBBackingInfo()
    usb.backing.deviceName = backing
    usb.connectable = vim.vm.device.VirtualDevice.ConnectInfo()
    usb.connectable.startConnected = True
    usb.connectable.connected = True
    usb.connectable.allowGuestControl = False
    change = vim.vm.device.VirtualDeviceSpec()
    change.operation = vim.vm.device.VirtualDeviceSpec.Operation.add
    change.device = usb
    spec = vim.vm.ConfigSpec()
    spec.deviceChange = [change]
    vmware_wait_task(vm.ReconfigVM_Task(spec=spec), "attach")
    stamp()
    settle = cfg.getfloat("bench", "settle_s", fallback=10.0)
    state = wait_streaming(match, settle)
    if state != "streaming":
        out("ATTACHED at hypervisor but guest device is %s after %.0f s" % (state, settle),
            EXIT_WRONG_STATE)
    out("ATTACHED %s (streaming)" % backing, EXIT_OK)


# ---------------------------------------------------------------- btaudio

def bt_ctl(cfg, args):
    ctl = os.path.expanduser(cfg.get("btaudio", "ctl_path"))
    if not os.path.isfile(ctl):
        out("BENCH NOT CONFIGURED: btaudio ctl not found at %s" % ctl,
            EXIT_NOT_CONFIGURED)
    r = subprocess.run([ctl] + args, capture_output=True, text=True, timeout=120)
    return r.returncode, r.stdout


def bt_capture_active(cfg):
    _, text = bt_ctl(cfg, ["--status", cfg.get("btaudio", "match")])
    return any(ln.startswith("ACTIVE") and "capture" in ln for ln in text.splitlines())


def bt_status(cfg):
    match = cfg.get("btaudio", "match")
    state = guest_capture_streams(match, 2.0) if bt_capture_active(cfg) else "absent"
    out("STATUS btaudio: capture %s, match %s" % (state, match),
        EXIT_OK if state == "streaming" else EXIT_WRONG_STATE)


def bt_disconnect(cfg):
    match = cfg.get("btaudio", "match")
    rc, _ = bt_ctl(cfg, ["--disconnect", match, "--quiet"])
    stamp()
    deadline = time.time() + cfg.getfloat("bench", "settle_s", fallback=20.0)
    while bt_capture_active(cfg):
        if time.time() >= deadline:
            out("DISCONNECT sent but capture still ACTIVE for '%s'" % match,
                EXIT_WRONG_STATE)
        time.sleep(1)
    out("DETACHED %s" % match, EXIT_OK)


def bt_connect(cfg):
    match = cfg.get("btaudio", "match")
    timeout = int(cfg.getfloat("bench", "settle_s", fallback=45.0))
    rc, _ = bt_ctl(cfg, ["--connect", match, "--timeout", str(timeout), "--quiet"])
    stamp()
    state = wait_streaming(match, timeout) if bt_capture_active(cfg) else "absent"
    if state != "streaming":
        out("CONNECT sent but capture is %s for '%s'" % (state, match), EXIT_WRONG_STATE)
    out("ATTACHED %s" % match, EXIT_OK)


# ---------------------------------------------------------------- main

def main():
    if len(sys.argv) != 2 or sys.argv[1] not in ("status", "disconnect", "connect", "info"):
        print("usage: bench_trigger.py status | disconnect | connect | info")
        sys.exit(EXIT_FAILED)
    action = sys.argv[1]
    cfg, trigger = load_config()
    if action == "info":
        # What the tests need to pick the device and size their loops.
        section = "vmware" if trigger == "vmware_usb" else "btaudio"
        match = cfg.get(section, "guest_match", fallback=cfg.get(section, "match", fallback=""))
        out("INFO trigger=%s match=%s runs=%d offsets_ms=%s" % (
            trigger, match, cfg.getint("bench", "runs", fallback=3),
            cfg.get("bench", "offsets_ms", fallback="0,250,5000")), EXIT_OK)
    table = {
        "vmware_usb": (vmware_status, vmware_disconnect, vmware_connect_dev),
        "btaudio": (bt_status, bt_disconnect, bt_connect),
    }
    status, disconnect, connect = table[trigger]
    if action == "status":
        status(cfg)
    with Lock():
        pace(cfg)
        (disconnect if action == "disconnect" else connect)(cfg)


if __name__ == "__main__":
    main()

