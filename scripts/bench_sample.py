#!/usr/bin/env python3
#
# Copyright (C) 2026 SIPez LLC.  All rights reserved.
#
# bench_sample.py -- one sample of a test process, for the soak.
#
#   bench_sample.py <pid> <log file> <state file>
#
# Prints one line:
#
#   SAMPLE handles=N threads=N private_kb=N working_kb=N crit_new=N crit_total=N
#
# and, when crit_new > 0, a second line with the first new CRIT log line.
#
#   handles, threads, private_kb, working_kb
#       the process's open handle count, thread count, private bytes and
#       working set, read with the Win32 API through ctypes (Windows only).
#   crit_new, crit_total
#       CRIT lines added to <log file> since the previous call, and in all
#       since the first. sipX logs every fire-escape, retired driver and
#       stuck worker at CRIT, so for the soak any new one is a hard failure.
#       The read position and running total live in <state file>; the
#       first call (no state file) starts counting from the current end
#       of the log and reports crit_new=0.
#
# The soak calls this from inside the test process at each cycle
# boundary, passing its own pid, so samples line up with cycles. The
# pipe it reads this through is one extra handle in every sample, so it
# does not affect a trend.
#
# Exit codes:
#   0  sampled
#   1  failed (the line says why)
#   2  not supported on this platform

import ctypes
import os
import re
import sys

EXIT_OK, EXIT_FAILED, EXIT_UNSUPPORTED = 0, 1, 2

CRIT_RE = re.compile(r'^"[^"]*":\d+:[A-Z_]+:CRIT:')


def out(msg, code):
    print(msg)
    sys.stdout.flush()
    sys.exit(code)


class PROCESS_MEMORY_COUNTERS_EX(ctypes.Structure):
    _fields_ = [("cb", ctypes.c_uint32),
                ("PageFaultCount", ctypes.c_uint32),
                ("PeakWorkingSetSize", ctypes.c_size_t),
                ("WorkingSetSize", ctypes.c_size_t),
                ("QuotaPeakPagedPoolUsage", ctypes.c_size_t),
                ("QuotaPagedPoolUsage", ctypes.c_size_t),
                ("QuotaPeakNonPagedPoolUsage", ctypes.c_size_t),
                ("QuotaNonPagedPoolUsage", ctypes.c_size_t),
                ("PagefileUsage", ctypes.c_size_t),
                ("PeakPagefileUsage", ctypes.c_size_t),
                ("PrivateUsage", ctypes.c_size_t)]


class THREADENTRY32(ctypes.Structure):
    _fields_ = [("dwSize", ctypes.c_uint32),
                ("cntUsage", ctypes.c_uint32),
                ("th32ThreadID", ctypes.c_uint32),
                ("th32OwnerProcessID", ctypes.c_uint32),
                ("tpBasePri", ctypes.c_int32),
                ("tpDeltaPri", ctypes.c_int32),
                ("dwFlags", ctypes.c_uint32)]


PROCESS_QUERY_INFORMATION = 0x0400
PROCESS_VM_READ = 0x0010
TH32CS_SNAPTHREAD = 0x00000004
INVALID_HANDLE = ctypes.c_void_p(-1).value


def process_counts(pid):
    try:
        k32 = ctypes.CDLL("kernel32.dll")
    except OSError:
        out("FAILED: not supported here (no kernel32.dll)", EXIT_UNSUPPORTED)
    k32.OpenProcess.restype = ctypes.c_void_p
    k32.OpenProcess.argtypes = [ctypes.c_uint32, ctypes.c_int, ctypes.c_uint32]
    k32.CloseHandle.argtypes = [ctypes.c_void_p]
    k32.GetProcessHandleCount.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint32)]
    k32.K32GetProcessMemoryInfo.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_uint32]
    k32.CreateToolhelp32Snapshot.restype = ctypes.c_void_p
    k32.CreateToolhelp32Snapshot.argtypes = [ctypes.c_uint32, ctypes.c_uint32]
    k32.Thread32First.argtypes = [ctypes.c_void_p, ctypes.POINTER(THREADENTRY32)]
    k32.Thread32Next.argtypes = [ctypes.c_void_p, ctypes.POINTER(THREADENTRY32)]

    h = k32.OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, pid)
    if not h:
        out("FAILED: cannot open process %d" % pid, EXIT_FAILED)
    try:
        handles = ctypes.c_uint32(0)
        if not k32.GetProcessHandleCount(h, ctypes.byref(handles)):
            out("FAILED: GetProcessHandleCount on %d" % pid, EXIT_FAILED)
        mem = PROCESS_MEMORY_COUNTERS_EX()
        mem.cb = ctypes.sizeof(mem)
        if not k32.K32GetProcessMemoryInfo(h, ctypes.byref(mem), mem.cb):
            out("FAILED: GetProcessMemoryInfo on %d" % pid, EXIT_FAILED)
    finally:
        k32.CloseHandle(h)

    snap = k32.CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0)
    if not snap or snap == INVALID_HANDLE:
        out("FAILED: CreateToolhelp32Snapshot", EXIT_FAILED)
    threads = 0
    try:
        te = THREADENTRY32()
        te.dwSize = ctypes.sizeof(te)
        ok = k32.Thread32First(snap, ctypes.byref(te))
        while ok:
            if te.th32OwnerProcessID == pid:
                threads += 1
            ok = k32.Thread32Next(snap, ctypes.byref(te))
    finally:
        k32.CloseHandle(snap)

    return (handles.value, threads, mem.PrivateUsage // 1024, mem.WorkingSetSize // 1024)


def new_crit_lines(log_path, state_path):
    """(new count, total count, first new line) since the last call."""
    if not os.path.isfile(log_path):
        out("FAILED: log file %s not found from %s" % (log_path, os.getcwd()), EXIT_FAILED)
    size = os.path.getsize(log_path)
    offset, total, first_call = size, 0, True
    if os.path.isfile(state_path):
        try:
            with open(state_path) as f:
                offset, total = [int(x) for x in f.read().split()[:2]]
            first_call = False
        except (OSError, ValueError):
            out("FAILED: unreadable state file %s" % state_path, EXIT_FAILED)
    if size < offset:
        # The log was truncated or replaced: read it from the start.
        offset = 0
    new, first = 0, None
    if not first_call:
        with open(log_path, "rb") as f:
            f.seek(offset)
            for raw in f:
                ln = raw.decode("latin-1", "replace")
                if CRIT_RE.match(ln):
                    new += 1
                    if first is None:
                        first = ln.strip()[:300]
            offset = f.tell()
    total += new
    with open(state_path, "w") as f:
        f.write("%d %d\n" % (offset, total))
    return new, total, first


def main():
    if len(sys.argv) != 4:
        out("usage: bench_sample.py <pid> <log file> <state file>", EXIT_FAILED)
    try:
        pid = int(sys.argv[1])
    except ValueError:
        out("FAILED: bad pid %s" % sys.argv[1], EXIT_FAILED)
    handles, threads, private_kb, working_kb = process_counts(pid)
    new, total, first = new_crit_lines(sys.argv[2], sys.argv[3])
    print("SAMPLE handles=%d threads=%d private_kb=%d working_kb=%d crit_new=%d crit_total=%d"
          % (handles, threads, private_kb, working_kb, new, total))
    if first:
        print("FIRST_NEW_CRIT %s" % first)
    sys.stdout.flush()
    sys.exit(EXIT_OK)


if __name__ == "__main__":
    main()
