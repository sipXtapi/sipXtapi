#!/usr/bin/env python3
#
# test_summary.py - Display summary from a sipXtapi test result JSON file
#
# Copyright (C) 2005-2026 SIPez LLC.  All rights reserved.
#
# Usage: ./test_summary.py <results.json>

import json
import sys


def main():
    if len(sys.argv) != 2:
        print("Usage: %s <results.json>" % sys.argv[0])
        return 1

    data = json.load(open(sys.argv[1]))
    projects = data["projects"]

    print("Host: %s  OS: %s  Arch: %s" % (
        data.get("hostname", ""), data.get("hostos", ""),
        data.get("hostarch", "")))
    print("Date: %s  Time: %s" % (
        data.get("builddate", ""), data.get("buildtime", "")))
    print("")

    total_ran = total_passed = total_failed = total_hangs = total_aborts = 0

    for proj, vals in projects.items():
        r = vals.get("ran", 0)
        p = vals.get("passed", 0)
        f = vals.get("failed", 0)
        h = vals.get("hangs", 0)
        a = vals.get("aborts", 0)
        total_ran += r
        total_passed += p
        total_failed += f
        total_hangs += h
        total_aborts += a
        status = "PASS" if f == 0 and h == 0 and a == 0 else "FAIL"
        print("  %-25s %7d ran  %7d passed  %4d failed  %d hangs  %d aborts  [%s]" % (
            proj, r, p, f, h, a, status))
        for test, failure in vals.get("testFailures", {}).items():
            print("    %s  %s" % (test, failure))

    print("-" * 60)
    print("  %-25s %7d ran  %7d passed  %4d failed  %d hangs  %d aborts" % (
        "TOTAL", total_ran, total_passed, total_failed,
        total_hangs, total_aborts))

    return 0


if __name__ == "__main__":
    sys.exit(main())

