//
// Copyright (C) 2026 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _RscBracket_h_
#define _RscBracket_h_

// SYSTEM INCLUDES
#include <stdio.h>

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <os/OsTask.h>
#include <utl/UtlString.h>
#include <utl/UtlRscTrace.h>

/// Bracket an operation with resource snapshots and report what it left
/// behind, as one "RSC <label> n=<ops>: ..." line on stdout and in the
/// sipX log.
///
/// Usage:
///   doOperation();                  // warm-up: one-time setup stays out
///   RscBracket b("call setup/hangup");
///   for (int i = 0; i < n; i++) doOperation();
///   b.report(n, 1000);              // settle 1 s, then report
///
/// The counts are process-wide, so background activity adds noise. Run
/// the bracket at two sizes (say n and 2n): a real leak grows with n,
/// noise does not. report() returns the summed delta of all counted
/// types, so a later stage can assert that it is zero.
class RscBracket
{
public:
   explicit RscBracket(const char* label) : mLabel(label ? label : "") {}

   /// Take the "before" snapshot again, now.
   void restart() { mBefore.take(); }

   /// Wait settleMs (for asynchronous teardown to finish), take the
   /// "after" snapshot, print and log the delta, and return the summed
   /// delta of all counted types.
   long report(int ops, int settleMs = 0)
   {
      if (settleMs > 0)
      {
         OsTask::delay(settleMs);
      }
      UtlRscSnapshot after;
      char label[256];
      snprintf(label, sizeof(label), "%s n=%d", mLabel.data(), ops);
      UtlString line;
      after.formatDelta(mBefore, label, line, ops);
      printf("%s\n", line.data());
      fflush(stdout);
      OsSysLog::add(FAC_KERNEL, PRI_NOTICE, "%s", line.data());
      return after.totalDelta(mBefore);
   }

private:
   UtlString mLabel;
   UtlRscSnapshot mBefore;

   RscBracket(const RscBracket&);
   RscBracket& operator=(const RscBracket&);
};

#endif // _RscBracket_h_
