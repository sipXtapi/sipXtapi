#!/usr/bin/env python3
#
# compare_test_runs.py - Compare two sipXtapi test result JSON files
#
# Copyright (C) 2005-2026 SIPez LLC.  All rights reserved.
#
# Usage: ./compare_test_runs.py <run1.json> <run2.json>

import json
import sys


def main():
    if len(sys.argv) != 3:
        print("Usage: %s <run1.json> <run2.json>" % sys.argv[0])
        return 1

    r1 = json.load(open(sys.argv[1]))
    r2 = json.load(open(sys.argv[2]))
    p1 = r1["projects"]
    p2 = r2["projects"]

    print("Comparing: %s vs %s" % (sys.argv[1], sys.argv[2]))
    print("")

    diffs = False
    for proj in p1:
        f1 = set(p1[proj].get("testFailures", {}).keys())
        f2 = set(p2.get(proj, {}).get("testFailures", {}).keys())
        new_failures = f2 - f1
        fixed = f1 - f2
        common = f1 & f2
        changed = []
        for t in sorted(common):
            v1 = p1[proj]["testFailures"][t]
            v2 = p2[proj]["testFailures"][t]
            if str(v1) != str(v2):
                changed.append((t, v1, v2))

        if new_failures or fixed or changed:
            diffs = True
            print("=== %s ===" % proj)
            for t in sorted(new_failures):
                print("  NEW FAILURE: %s  %s" % (t, p2[proj]["testFailures"][t]))
            for t in sorted(fixed):
                print("  FIXED:       %s" % t)
            for t, v1, v2 in changed:
                print("  CHANGED:     %s  %s -> %s" % (t, v1, v2))
            print("")

    if not diffs:
        print("No test status changes between runs.")

    return 0


if __name__ == "__main__":
    sys.exit(main())

