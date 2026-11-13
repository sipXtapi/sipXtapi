//
// Copyright (C) 2026 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _UtlRscTrace_h_
#define _UtlRscTrace_h_

// Resource accounting, two levels.
//
// Level 1 (always compiled in): a live count per resource type, raised in
// each counted class's constructor and lowered in its destructor, plus a
// monotonic count of tasks that had to be force-terminated. Counting is one
// interlocked add; nothing is logged unless asked. UtlRscSnapshot captures
// every count at once together with process-level numbers (handles or file
// descriptors, private memory, threads) and the names of live named tasks
// and message queues, so a test or soak can diff before and after an
// operation and see exactly what it left behind.
//
// This header must stay free of os/ includes: OsBSem.h and OsMutex.h
// include it to count in their inline base-class constructors.
//
// Level 2 (RSC_TEST, compiled out): the original 2004 per-object store,
// recording address, name and owning task per allocation. It predates
// 64-bit builds (addresses are int) and its call sites were removed long
// ago; it is kept for reference and does not build as is.

#define RSC_TEST
#undef RSC_TEST

// FORWARD DECLARATIONS
class UtlSList;
class UtlString;

class UtlRscTrace;

/// Level 1 resource counts: see the comment at the top of this file.
class UtlRscCount
{
public:
   /// Counted resource types. Values match the legacy UtlRscTrace::RscType.
   enum RscType
   {
      NONE      = 0,
      MEMORY    = 1,   ///< not counted at level 1
      OSSOCKET  = 2,   ///< not counted at level 1
      OSMSGQ    = 3,
      OSBSEM    = 4,
      OSCSEM    = 5,
      OSMUTEX   = 6,
      OSRWMUTEX = 7,
      OSTIMER   = 8,
      OSTASK    = 9,
      NUM_TYPES = 10
   };

   /// Count one object of this type created.
   static void created(RscType type);

   /// Count one object of this type destroyed.
   static void destroyed(RscType type);

   /// Live objects of this type (created minus destroyed).
   static long live(RscType type);

   /// Count one task force-terminated (TerminateThread or pthread_cancel).
   static void taskTerminated();

   /// Tasks force-terminated since process start (never decreases).
   static long tasksTerminated();

   /// Short name of a type, e.g. "OsBSem"; NULL for an uncounted type.
   static const char* typeName(RscType type);
};

/// The counts one module holds (see UtlRscSnapshot::setSource).
struct UtlRscRawCounts
{
   long live[UtlRscCount::NUM_TYPES];
   long tasksTerminated;
};

/// Read this module's counts, and the names of its live named tasks and
/// message queues as '\n'-separated name-database keys (e.g.
/// "Task.MpMediaTask") into names, truncated to fit namesLen.
void utlRscReadLocal(UtlRscRawCounts& counts, char* names, int namesLen);

/// A function that reads counts and names, like utlRscReadLocal.
typedef void (*UtlRscSource)(UtlRscRawCounts& counts, char* names, int namesLen);

/// Process-level numbers, read from the operating system.
/// Each field is -1 when the platform cannot supply it.
struct UtlRscProcessStats
{
   long handles;     ///< Windows: open handles. Linux: open file descriptors.
   long privateKB;   ///< Windows: private bytes. Linux: resident set. In KB.
   long threads;     ///< Threads in the process.
};

/// Fill stats for the current process. Implemented per platform in
/// os/Wnt/OsProcessStatsWnt.cpp and os/linux/OsProcessStatsLinux.cpp.
void utlRscGetProcessStats(UtlRscProcessStats& stats);

/// One moment's view of every level 1 count, the process numbers and the
/// names of live named tasks and message queues.
class UtlRscSnapshot
{
public:
   /// Takes the snapshot now.
   UtlRscSnapshot();

   /// Where every snapshot reads counts and names; NULL restores the
   /// default, utlRscReadLocal. Each module that links sipXportLib
   /// statically has its own counters: on Windows, sipXtapi.dll and a
   /// test executable each have a copy, and the objects a call creates
   /// are counted in the DLL's. A sipXtapi test therefore points this at
   /// sipxDiagReadResourceCounts. Process numbers are process-wide and
   /// are not affected.
   static void setSource(UtlRscSource source);

   ~UtlRscSnapshot();

   /// Re-take the snapshot now.
   void take();

   /// Live count of one type at the time of the snapshot.
   long live(UtlRscCount::RscType type) const { return mLive[type]; }

   long tasksTerminated() const { return mTasksTerminated; }

   const UtlRscProcessStats& process() const { return mProcess; }

   /// Format "this minus before" as one line, starting with "RSC <label>:".
   /// Every counted type and process number is listed, so lines from
   /// different runs line up; names of tasks and queues present now but
   /// not before (and the reverse) follow, at most maxNames of each.
   /// When perOp > 0 each delta is also shown divided by perOp.
   void formatDelta(const UtlRscSnapshot& before, const char* label,
                    UtlString& line, int perOp = 0, int maxNames = 8) const;

   /// Sum of the deltas of all counted types (not process numbers).
   long totalDelta(const UtlRscSnapshot& before) const;

private:
   long mLive[UtlRscCount::NUM_TYPES];
   long mTasksTerminated;
   UtlRscProcessStats mProcess;
   UtlSList* mpNames;   ///< UtlString keys, e.g. "Task.MpMediaTask"

   UtlRscSnapshot(const UtlRscSnapshot&);
   UtlRscSnapshot& operator=(const UtlRscSnapshot&);
};

#ifdef RSC_TEST

// SYSTEM INCLUDES
#include "utl/UtlRscStore.h"
#include <os/OsMutex.h>

// APPLICATION INCLUDES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:A class used to keep track of the number of resource allocations and frees
// This class is used in conjunction with instrumented versions of the global
// new and delete operators.
class UtlRscTrace
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static unsigned long sAllocCnt; // Track the number of memory allocs
   static unsigned long sFreeCnt;  // Track the number of memory frees
   static int           sTraceFlag;// If non-zero, print a line of info
                                   //  for every call to new()/delete()
   static unsigned long sStartTime;  // 
   static UtlRscStore mResourceStore;

   enum RscType
   {
          NONE          = 0,
      MEMORY    = 1,   
      OSSOCKET  = 2,       
      OSMSGQ    = 3,  
      OSBSEM    = 4,
          OSCSEM        = 5,
          OSMUTEX       = 6,
          OSRWMUTEX     = 7,
          OSTIMER       = 8,
          OSTASK        = 9
   };


/* ============================ CREATORS ================================== */

   UtlRscTrace();
     //:Default constructor (and take a checkpoint)

   UtlRscTrace(const UtlRscTrace& rRscTrace);
     //:Copy constructor

   virtual
   ~UtlRscTrace();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   UtlRscTrace& operator=(const UtlRscTrace& rhs);
     //:Assignment operator

   virtual void checkpoint();
     //:Remember the count of outstanding memory allocations

   static void setTraceFlag(int flag) { sTraceFlag = flag; };
    //: Set trace flag, if non-zero, print a line of info

   static int enableMemTracking(int taskId = 0);
    //: Set trace flag, if non-zero, print a line of info

   static int enableMsgQTracking(int taskId = 0);
    //: Set trace flag, if non-zero, print a line of info

   static int enableBSemTracking(int taskId = 0);
    //: Set trace flag, if non-zero, print a line of info

   static int enableCSemTracking(int taskId = 0);
    //: Set trace flag, if non-zero, print a line of info

   static int enableMutexTracking(int taskId = 0);
    //: Set trace flag, if non-zero, print a line of info

   static int enableRWMutexTracking(int taskId = 0);
    //: Set trace flag, if non-zero, print a line of info

   static int enableTimerTracking(int taskId = 0);
    //: Set trace flag, if non-zero, print a line of info

   static int enableTaskTracking(int taskId = 0);
    //: Set trace flag, if non-zero, print a line of info

   static int enableSocketTracking(int taskId = 0);
    //: Set trace flag, if non-zero, print a line of info

   static int disableTracking();
    //: Set trace flag, if non-zero, print a line of info

   static void addAllocCnt(int addr,
                                                  int taskId);
    //: Add allocCnt for the task, used for OsSocket tracking

   static void addAllocCnt(int size,
                                                  int addr,
                                                  int taskId);
    //: Add allocCnt for the task, used for memory/OsMutex/OsRWMutex tracking

        static void addAllocCnt(int size,
                                                          int addr,
                                                          const char* name, 
                                                          int pArg, 
                                                          int priority, 
                                                          int options,
                                                          int taskId);
    //: Add allocCnt for the task, used for OsTask tracking

        static void addAllocCnt(int state,
                                                          int addr,
                                                          int timerId, 
                                                          int type, 
                                                          int taskId);
    //: Add allocCnt for the task, used for OsTimer/OsCSem tracking

        static void addAllocCnt(int options,
                                                          int addr,
                                                          int state, 
                                                          int taskId);
    //: Add allocCnt for the task, used for OsBSem tracking

        static void addAllocCnt(int addr,
                                                          const char* name, 
                                                          int taskId);
    //: Add allocCnt for the task, used for OsMsgQ tracking

        static void addFreeCnt(int addr, int taskId = 0);
    //: Add freeCnt for the task

/* ============================ ACCESSORS ================================= */

   static void showMem(int taskId = 0);

   static int delta();
     //:Return the change to the number of outstanding memory allocations
     //:since the last checkpoint.

   static int delta(int taskId);
     //:Return the change to the number of outstanding memory allocations
     //:since the last checkpoint.

   static int allocCnt(int taskId = 0);
     //:Return the number of memory allocations (monotonically increasing)

   static int rscStatus();
     //:Return the number of memory frees (monotonically increasing)

   static int freeCnt(int taskId = 0);
     //:Return the number of memory frees (monotonically increasing)

   static int netAllocCnt();
     //:Return the net number of allocations (allocCnt - freeCnt)

   static int netAllocCnt(int taskId);
     //:Return the net number of allocations (allocCnt - freeCnt)

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        static long mCheckpoint; // Used to remember the net number of 
                     // allocations at time of checkpoint
        static OsMutex    *mpResourceStoreLock;

        static UtlRscStore mUtlRscStore;

        static int mTaskId;
};


/* ============================ INLINE METHODS ============================ */
#endif // RSC_TEST

#endif  // _UtlRscTrace_h_

