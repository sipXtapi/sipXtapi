#!/usr/bin/env python3
#
# sipx_test_runner.py - Cross-platform test runner for sipXtapi unit tests
#
# Copyright (C) 2005-2026 SIPez LLC.  All rights reserved.
#
# Runs unit tests for sipXtapi projects, captures results, and produces
# JSON output compatible with the existing Jenkins CI test_summary_to_json.sh
# format.  Works on both Linux and Windows, locally and in GitHub Actions.
#
# Requires Python 3.8+
#
# Usage examples:
#   python sipx_test_runner.py
#   python sipx_test_runner.py --config Debug
#   python sipx_test_runner.py --projects sipXportLib sipXmediaLib
#   python sipx_test_runner.py --filter OsFileTest
#   python sipx_test_runner.py --exclude OsEventTest::testTimedEvent
#   python sipx_test_runner.py --test-list sipXportLib_test_list.txt
#   python sipx_test_runner.py --timeout 120

import argparse
import json
import os
import platform
import re
import signal
import subprocess
import sys
import time

# Project list in canonical order (must match test_summary_to_json.sh)
PROJECTS = [
    "sipXportLib",
    "sipXsdpLib",
    "sipXtackLib",
    "sipXmediaLib",
    "sipXmediaAdapterLib",
    "sipXcallLib",
    "sipXtapi",
]

# Default timeout per test in seconds
DEFAULT_TIMEOUT = 90

# Windows crash exit codes that indicate an abort/segfault
WINDOWS_CRASH_CODES = {
    0xC0000005,  # ACCESS_VIOLATION
    0xC0000374,  # HEAP_CORRUPTION
    0xC0000409,  # STATUS_STACK_BUFFER_OVERRUN
    0xC0000602,  # unknown exception
    0x80000003,  # BREAKPOINT
    0x80000004,  # SINGLE_STEP
}

IS_WINDOWS = sys.platform.startswith("win") or sys.platform == "cygwin"


def find_repo_root():
    """Walk up from cwd looking for a directory containing sipXportLib/."""
    check = os.path.abspath(os.getcwd())
    while True:
        if os.path.isdir(os.path.join(check, "sipXportLib")):
            return check
        parent = os.path.dirname(check)
        if parent == check:
            break
        check = parent
    return os.path.abspath(os.getcwd())


def get_test_exe_path(repo_root, project, config):
    """Return the path to the test executable for a project."""
    if IS_WINDOWS:
        # sipXtapi test exe is named sipXtapiTest, others follow pattern
        exe_name = project + "Test.exe"
        return os.path.join(repo_root, "x64", config, exe_name)
    else:
        # Linux: testsuite in the test source directory
        test_dir = get_linux_test_dir(repo_root, project)
        return os.path.join(test_dir, "testsuite")


def get_linux_test_dir(repo_root, project):
    """Return the test directory for a project on Linux."""
    if project == "sipXmediaAdapterLib":
        return os.path.join(
            repo_root, project,
            "sipXmediaMediaProcessing", "src", "test"
        )
    elif project == "sipXtapi":
        return os.path.join(
            repo_root, "sipXcallLib", "src", "test", "tapi"
        )
    else:
        return os.path.join(repo_root, project, "src", "test")


def get_test_working_dir(repo_root, project, config):
    """Return the working directory for running tests."""
    if IS_WINDOWS:
        return os.path.join(repo_root, "x64", config)
    else:
        return get_linux_test_dir(repo_root, project)


def discover_tests(exe_path):
    """Run the test executable with --list and return list of test names."""
    try:
        result = subprocess.run(
            [exe_path, "--list"],
            capture_output=True,
            text=True,
            timeout=30,
        )
        tests = []
        for line in result.stdout.splitlines():
            line = line.strip()
            if line and "::" in line:
                tests.append(line)
        return tests
    except (subprocess.TimeoutExpired, FileNotFoundError, OSError) as exc:
        print("  Warning: could not list tests from %s: %s" % (exe_path, exc))
        return []


def load_test_list(path):
    """Load a test list file.  Returns dict mapping test name to expected
    outcome string ('' means expect success, 'hangs', 'aborts', or a
    failure count string)."""
    test_map = {}
    with open(path, "r") as fh:
        for raw_line in fh:
            line = raw_line.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split()
            test_name = parts[0]
            expected = parts[1] if len(parts) > 1 else ""
            test_map[test_name] = expected
    return test_map


def is_crash_code(return_code):
    """Check if a return code indicates a crash/abort."""
    if not IS_WINDOWS:
        # Linux: killed by signal shows as negative return code
        # SIGSEGV = -11, SIGABRT = -6
        # Or as 128+signal: 139 = SIGSEGV, 134 = SIGABRT
        if return_code < 0:
            return True
        if return_code in (139, 134):
            return True
        return False
    else:
        # Windows: crash codes are large unsigned values
        # Python on Windows returns them as negative signed int32
        unsigned = return_code & 0xFFFFFFFF if return_code < 0 else return_code
        return unsigned in WINDOWS_CRASH_CODES


def parse_succeeded_line(output, test_name):
    """Parse the 'ClassName: succeeded N/M tests' line from test output.
    Returns (passed, total) or None if not found."""
    class_name = test_name.split("::")[0]
    pattern = re.escape(class_name) + r":\s+\d+\s+test\s+methods,\s+(\d+)/(\d+)\s+test\s+points"
    match = re.search(pattern, output)
    if match:
        return int(match.group(1)), int(match.group(2))
    return None


def run_single_test(exe_path, test_name, work_dir, timeout):
    """Run a single test and return a result dict."""
    result = {
        "test": test_name,
        "outcome": "success",
        "failures": 0,
        "passed": 0,
        "ran": 0,
        "stdout": "",
    }

    try:
        proc = subprocess.Popen(
            [exe_path, test_name],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            cwd=work_dir,
            start_new_session=True,
        )
        try:
            stdout_bytes, _ = proc.communicate(timeout=timeout)
        except subprocess.TimeoutExpired:
            if IS_WINDOWS:
                proc.kill()
            else:
                try:
                    os.killpg(os.getpgid(proc.pid), signal.SIGKILL)
                except OSError:
                    proc.kill()
            try:
                stdout_bytes, _ = proc.communicate(timeout=5)
            except subprocess.TimeoutExpired:
                stdout_bytes = b""
            result["stdout"] = stdout_bytes.decode("utf-8", errors="replace") if stdout_bytes else ""
            if "ABORT: due to caught signal" in result["stdout"]:
                result["outcome"] = "aborts"
                result["ran"] = 1
                result["passed"] = 0
                return result
            result["outcome"] = "hangs"
            result["ran"] = 1
            result["passed"] = 0
            return result

        result["stdout"] = stdout_bytes.decode("utf-8", errors="replace") if stdout_bytes else ""

        if proc.returncode == 0:
            result["outcome"] = "success"
        elif is_crash_code(proc.returncode):
            result["outcome"] = "aborts"
        else:
            result["outcome"] = "fail"

        # Parse the succeeded line for pass/total counts
        counts = parse_succeeded_line(result["stdout"], test_name)
        if counts:
            result["passed"] = counts[0]
            result["ran"] = counts[1]
        elif result["outcome"] == "success":
            # Test passed but no summary line found; count as 1/1
            result["passed"] = 1
            result["ran"] = 1
        else:
            result["ran"] = 1
            result["passed"] = 0

        if result["outcome"] == "fail":
            result["failures"] = result["ran"] - result["passed"]

    except (FileNotFoundError, OSError) as exc:
        result["outcome"] = "error"
        result["stdout"] = str(exc)
        result["ran"] = 1
        result["passed"] = 0

    return result


def apply_filters(test_list, include_filter, exclude_list):
    """Filter test list by include substring and exclude list."""
    filtered = test_list
    if include_filter:
        filtered = [t for t in filtered if include_filter in t]
    if exclude_list:
        filtered = [t for t in filtered if t not in exclude_list]
    return filtered


def remove_sipxtapi_log(work_dir):
    """Delete the shared sipXtapi log file before the first sipXtapi test runs.

    The sipXtapi test suite hardcodes its log filename to sipXtapiTests.txt
    (set via sipxConfigSetLogFile in sipXtapiTestSuite::setUp), and unlike
    the other projects' per-class hooks, nothing truncates this file
    between runs.  Without this cleanup the file grows without bound
    across runs (observed at 1+ GB).

    Removing it once per run, before the first sipXtapi test, gives a
    fresh log for the run while preserving log output across all the
    sipXtapi tests within that single run.
    """
    log_path = os.path.join(work_dir, "sipXtapiTests.txt")
    try:
        if os.path.isfile(log_path):
            os.remove(log_path)
            print("  Removed stale sipXtapi log: %s" % log_path)
    except OSError as exc:
        print("  Warning: could not remove %s: %s" % (log_path, exc))

def run_project_tests(
    repo_root, project, config, timeout,
    include_filter, exclude_list, test_list_map
):
    """Run all tests for a project.  Returns a project result dict."""
    exe_path = get_test_exe_path(repo_root, project, config)
    work_dir = get_test_working_dir(repo_root, project, config)

    proj_result = {
        "testFailures": {},
        "ran": 0,
        "passed": 0,
        "failed": 0,
        "aborts": 0,
        "hangs": 0,
    }

    if not os.path.isfile(exe_path):
        print("  %s test executable not found: %s" % (project, exe_path))
        print("  Skipping %s" % project)
        return proj_result

    print("\n=== %s ===" % project)
    print("  Executable: %s" % exe_path)

    # Discover or load tests
    if test_list_map is not None:
        all_tests = list(test_list_map.keys())
    else:
        all_tests = discover_tests(exe_path)

    if not all_tests:
        print("  No tests found for %s" % project)
        return proj_result

    # Apply filters
    tests_to_run = apply_filters(all_tests, include_filter, exclude_list)

    if project == "sipXtapi" and tests_to_run:
        remove_sipxtapi_log(work_dir)

    print("  Tests to run: %d (of %d discovered)" % (
        len(tests_to_run), len(all_tests)
    ))

    for test_name in tests_to_run:
        # Check expected outcome from test list
        expected = ""
        if test_list_map is not None:
            expected = test_list_map.get(test_name, "")

        # Skip tests expected to hang or abort
        if expected in ("hangs", "aborts"):
            print("  %s  [skip: expected %s]" % (test_name, expected))
            continue

        # Print test name without newline (like run_tests.sh)
        sys.stdout.write("  %s" % test_name)
        sys.stdout.flush()

        test_result = run_single_test(exe_path, test_name, work_dir, timeout)

        outcome = test_result["outcome"]
        if outcome not in ("hangs", "aborts"):
            proj_result["ran"] += test_result["ran"]
            proj_result["passed"] += test_result["passed"]

        if outcome == "success":
            print("  [OK]")
        else:
            if outcome == "hangs":
                print("  [HANGS]")
                proj_result["hangs"] += 1
                proj_result["testFailures"][test_name] = "hangs"
            elif outcome == "aborts":
                print("  [ABORTS]")
                proj_result["aborts"] += 1
                proj_result["testFailures"][test_name] = "aborts"
            elif outcome == "fail":
                print("  [FAIL: %s]" % test_result["failures"])
                proj_result["testFailures"][test_name] = test_result["failures"]
            else:
                print("  [ERROR]")
                proj_result["testFailures"][test_name] = "error"

            # Print test output for any non-success outcome
            if test_result["stdout"]:
                for out_line in test_result["stdout"].splitlines():
                    if not out_line.rstrip().endswith("succeeded"):
                        print("    | %s" % out_line)

        # If outcome differs from expected, save stdout for debugging
        if expected:
            expected_val = expected
        else:
            expected_val = "0"

        actual_val = str(test_result["failures"]) if outcome == "fail" else outcome
        if outcome == "success":
            actual_val = "0"

        if expected_val != actual_val:
            dump_name = test_name.replace("::", "__") + ".stdout"
            dump_path = os.path.join(work_dir, dump_name)
            try:
                with open(dump_path, "w") as fh:
                    fh.write(test_result["stdout"])
            except OSError:
                pass

    proj_result["failed"] = proj_result["ran"] - proj_result["passed"]
    return proj_result


def get_host_info():
    """Gather host information for the JSON output."""
    info = {
        "hostname": platform.node(),
        "hostarch": platform.machine(),
        "hostos": platform.system(),
        "hostkernel": platform.release(),
        "hostdistro": "",
        "build_toolchain": "",
    }

    if IS_WINDOWS:
        info["hostdistro"] = platform.platform()
    else:
        # Try to read /etc/os-release for PRETTY_NAME
        try:
            with open("/etc/os-release", "r") as fh:
                for line in fh:
                    if line.startswith("PRETTY_NAME="):
                        val = line.split("=", 1)[1].strip().strip('"')
                        info["hostdistro"] = val
                        break
        except OSError:
            info["hostdistro"] = platform.platform()

    if IS_WINDOWS:
        vs_paths = [
            ("msvc-2022", r"C:\Program Files\Microsoft Visual Studio\2022"),
            ("msvc-2019", r"C:\Program Files (x86)\Microsoft Visual Studio\2019"),
        ]
        for label, path in vs_paths:
            if os.path.isdir(path):
                info["build_toolchain"] = label
                break
    else:
        try:
            gcc_out = subprocess.check_output(
                ["gcc", "--version"],
                text=True, stderr=subprocess.DEVNULL
            ).splitlines()[0]
            gcc_ver = gcc_out.split()[-1]
            info["build_toolchain"] = "gcc-" + gcc_ver
        except (FileNotFoundError, subprocess.CalledProcessError, IndexError):
            pass

    return info


def get_git_info():
    """Gather git repo information for the JSON output."""
    info = {
        "branch": "unknown",
        "commit": "unknown",
        "commit_full": "unknown",
        "dirty": False,
        "dirty_files": [],
        "ci": os.environ.get("CI", "false").lower() == "true",
        "run_id": os.environ.get("GITHUB_RUN_ID"),
        "annotation": "",
    }

    try:
        info["branch"] = subprocess.check_output(
            ["git", "rev-parse", "--abbrev-ref", "HEAD"],
            text=True, stderr=subprocess.DEVNULL
        ).strip()

        if info["branch"] == "HEAD":
            env_branch = os.environ.get("GITHUB_REF_NAME", "")
            if env_branch:
                info["branch"] = env_branch

        info["commit_full"] = subprocess.check_output(
            ["git", "rev-parse", "HEAD"],
            text=True, stderr=subprocess.DEVNULL
        ).strip()
        info["commit"] = info["commit_full"][:7]

        dirty_rc = subprocess.call(
            ["git", "diff", "--quiet"],
            stderr=subprocess.DEVNULL
        )
        staged_rc = subprocess.call(
            ["git", "diff", "--cached", "--quiet"],
            stderr=subprocess.DEVNULL
        )
        info["dirty"] = (dirty_rc != 0) or (staged_rc != 0)

        if info["dirty"]:
            diff_output = subprocess.check_output(
                ["git", "diff", "--name-only"],
                text=True, stderr=subprocess.DEVNULL
            ).strip()
            staged_output = subprocess.check_output(
                ["git", "diff", "--cached", "--name-only"],
                text=True, stderr=subprocess.DEVNULL
            ).strip()
            files = set()
            if diff_output:
                files.update(diff_output.splitlines())
            if staged_output:
                files.update(staged_output.splitlines())
            info["dirty_files"] = sorted(files)

    except (FileNotFoundError, subprocess.CalledProcessError):
        print("Warning: not in a git repo or git not available, "
              "git metadata will not be included")

    return info


def build_json_output(project_results, host_info, git_info, date_label, time_label):
    """Build the JSON output dict matching test_summary_to_json.sh schema."""
    output = {
        "builddate": date_label,
        "buildtime": time_label,
        "hostname": host_info["hostname"],
        "hostarch": host_info["hostarch"],
        "hostos": host_info["hostos"],
        "hostdistro": host_info["hostdistro"],
        "hostkernel": host_info["hostkernel"],
        "build_toolchain": host_info["build_toolchain"],
        "branch": git_info["branch"],
        "commit": git_info["commit"],
        "commit_full": git_info["commit_full"],
        "dirty": git_info["dirty"],
        "dirty_files": git_info["dirty_files"],
        "ci": git_info["ci"],
        "run_id": git_info["run_id"],
        "annotation": git_info["annotation"],
        "projects": project_results,
    }
    return output


def print_summary(project_results):
    """Print a human-readable summary to stdout."""
    print("\n" + "=" * 60)
    print("TEST SUMMARY")
    print("=" * 60)

    total_ran = 0
    total_passed = 0
    total_failed = 0
    total_hangs = 0
    total_aborts = 0

    for proj_name in PROJECTS:
        if proj_name not in project_results:
            continue
        proj = project_results[proj_name]
        total_ran += proj["ran"]
        total_passed += proj["passed"]
        total_failed += proj["failed"]
        total_hangs += proj["hangs"]
        total_aborts += proj["aborts"]

        status = "PASS" if proj["failed"] == 0 and proj["hangs"] == 0 and proj["aborts"] == 0 else "FAIL"
        print("  %-25s %4d ran  %4d passed  %4d failed  %d hangs  %d aborts  [%s]" % (
            proj_name,
            proj["ran"], proj["passed"], proj["failed"],
            proj["hangs"], proj["aborts"],
            status,
        ))

        # List individual failures
        for test_name, failure in proj["testFailures"].items():
            print("    %s  %s" % (test_name, failure))

    print("-" * 60)
    print("  %-25s %4d ran  %4d passed  %4d failed  %d hangs  %d aborts" % (
        "TOTAL",
        total_ran, total_passed, total_failed,
        total_hangs, total_aborts,
    ))
    print("=" * 60)

    return total_failed == 0 and total_hangs == 0 and total_aborts == 0


def parse_args():
    parser = argparse.ArgumentParser(
        description="sipXtapi cross-platform unit test runner"
    )
    parser.add_argument(
        "--config",
        default="Release",
        help="Build configuration: Release or Debug (default: Release)",
    )
    parser.add_argument(
        "--arch",
        default="x64",
        help="Build platform/architecture (default: x64, Windows only)",
    )
    parser.add_argument(
        "--projects",
        nargs="+",
        choices=PROJECTS,
        default=None,
        help="Run tests only for specified projects",
    )
    parser.add_argument(
        "--filter",
        default=None,
        help="Only run tests whose name contains this substring",
    )
    parser.add_argument(
        "--exclude",
        nargs="+",
        default=None,
        help="Exclude specific test names (exact match)",
    )
    parser.add_argument(
        "--test-list",
        default=None,
        help="Path to a test list file with expected outcomes",
    )
    parser.add_argument(
        "--timeout",
        type=int,
        default=DEFAULT_TIMEOUT,
        help="Timeout per test in seconds (default: %d)" % DEFAULT_TIMEOUT,
    )
    parser.add_argument(
        "--output",
        default=None,
        help="Output JSON file path (default: auto-generated with timestamp)",
    )
    parser.add_argument(
        "--repo-root",
        default=None,
        help="Path to the sipXtapi repo root (default: auto-detect)",
    )
    return parser.parse_args()


def main():
    args = parse_args()

    repo_root = args.repo_root if args.repo_root else find_repo_root()
    print("Repo root: %s" % repo_root)
    print("Platform:  %s" % platform.platform())
    print("Config:    %s" % args.config)
    print("Timeout:   %d seconds" % args.timeout)

    if IS_WINDOWS:
        codec_dir = os.path.join(repo_root, "x64", "bin")
        codec_pattern = "codec_"
        if os.path.isdir(codec_dir):
            codecs = [f for f in os.listdir(codec_dir)
                      if f.startswith(codec_pattern) and f.endswith(".dll")]
            print("Codecs:    %d found in %s" % (len(codecs), codec_dir))
            if not codecs:
                print("ERROR: No codec DLLs found in %s" % codec_dir)
                print("Copy codec_*.dll to %s before running tests" % codec_dir)
                return 1
        else:
            print("ERROR: Codec directory not found: %s" % codec_dir)
            print("Create %s and copy codec_*.dll there" % codec_dir)
            return 1

    # Override exe path pattern for non-default arch on Windows
    if IS_WINDOWS and args.arch != "x64":
        print("Arch:      %s" % args.arch)

    # Timestamp for file naming and JSON
    start_time = time.time()
    now = time.localtime()
    date_label = time.strftime("%Y%m%d", now)
    time_label = time.strftime("%H%M", now)

    # Determine which projects to run
    projects_to_run = args.projects if args.projects else PROJECTS

    # Load test list if specified (applies to all projects when used)
    test_list_map = None
    if args.test_list:
        if os.path.isfile(args.test_list):
            test_list_map = load_test_list(args.test_list)
            print("Test list: %s (%d tests)" % (
                args.test_list, len(test_list_map)
            ))
        else:
            print("Warning: test list file not found: %s" % args.test_list)

    exclude_set = set(args.exclude) if args.exclude else None

    # Run tests for each project
    project_results = {}
    for project in projects_to_run:
        project_results[project] = run_project_tests(
            repo_root,
            project,
            args.config,
            args.timeout,
            args.filter,
            exclude_set,
            test_list_map,
        )

    # Print summary
    all_passed = print_summary(project_results)

    # Build and write JSON
    host_info = get_host_info()
    git_info = get_git_info()
    json_output = build_json_output(
        project_results, host_info, git_info, date_label, time_label
    )

    if args.output:
        json_path = args.output
    else:
        json_path = os.path.join(
            repo_root,
            "test_summary_%s_%s.json" % (date_label, time_label),
        )

    with open(json_path, "w") as fh:
        json.dump(json_output, fh, indent=2)
    print("\nJSON results written to: %s" % json_path)

    elapsed = time.time() - start_time
    minutes = int(elapsed // 60)
    seconds = int(elapsed % 60)
    print("Total time: %d:%02d (%d seconds)" % (minutes, seconds, int(elapsed)))

    # Exit with non-zero if any tests failed
    return 0 if all_passed else 1


if __name__ == "__main__":
    sys.exit(main())
