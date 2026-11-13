//
// Copyright (C) 2026 SIPez LLC  All rights reserved.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipxPortUnitTestFaultReport_h_
#define _SipxPortUnitTestFaultReport_h_

/// Make test executables explain their own crashes and hangs. Test-only;
/// called once from the sipxportunit main() before any test runs.
///
/// Crash (Windows): a vectored exception handler prints, for a fatal
/// exception on any thread, the exception code, the thread, the test that
/// was running and the faulting thread's stack with symbols (dbghelp and
/// the PDBs next to the executable). It then lets the exception continue,
/// so the existing signal handling, ProcDump or Windows error reporting
/// behave as before. At most 3 reports per process. Because it sees
/// exceptions first-chance, a report may name an exception that some code
/// then handles; the header says so. On Linux the existing signal handler
/// already prints a backtrace.
///
/// Hang (both): if the environment variable SIPX_TEST_WATCHDOG_SEC is a
/// positive number, a watchdog thread reports when no test method has
/// started and no test point has been recorded for that many seconds. It
/// prints the test that was running; on Windows it then writes a full
/// minidump (to SIPX_TEST_DUMP_DIR, default the current directory, named
/// sipx_hang_<pid>_<n>.dmp) and prints every thread's stack. It reports
/// once per stall and never ends the process.
void sipxUnitInstallFaultReporting();

#endif // _SipxPortUnitTestFaultReport_h_
