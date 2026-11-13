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

#include "utl/UtlRscTrace.h"

// ---------------------------------------------------------------------------
// Level 1: always compiled in. See the comment at the top of UtlRscTrace.h.
// ---------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include "os/OsNameDb.h"
#include "utl/UtlSList.h"
#include "utl/UtlSListIterator.h"
#include "utl/UtlString.h"
// After the sipX headers, which on Windows pull in winsock2.h before
// windows.h as they require.
#ifdef _WIN32
#  include <windows.h>
#endif

// Plain zero-initialized storage: no constructor runs, so objects created
// during static initialization (before main) are counted correctly
// whatever the order in which translation units initialize.
#ifdef _WIN32
static volatile LONG sRscLive[UtlRscCount::NUM_TYPES];
static volatile LONG sRscTasksTerminated;
static inline void rscAdd(volatile LONG* p, LONG v) { InterlockedExchangeAdd(p, v); }
static inline long rscRead(volatile LONG* p) { return (long)InterlockedCompareExchange(p, 0, 0); }
#else
static volatile long sRscLive[UtlRscCount::NUM_TYPES];
static volatile long sRscTasksTerminated;
static inline void rscAdd(volatile long* p, long v) { __atomic_add_fetch(p, v, __ATOMIC_RELAXED); }
static inline long rscRead(volatile long* p) { return __atomic_load_n(p, __ATOMIC_RELAXED); }
#endif

static const char* const sRscTypeNames[UtlRscCount::NUM_TYPES] =
{
   NULL,          // NONE
   NULL,          // MEMORY, not counted
   NULL,          // OSSOCKET, not counted
   "OsMsgQ",
   "OsBSem",
   "OsCSem",
   "OsMutex",
   "OsRWMutex",
   "OsTimer",
   "OsTask"
};

static inline bool rscValid(UtlRscCount::RscType type)
{
   return type > UtlRscCount::NONE && type < UtlRscCount::NUM_TYPES;
}

void UtlRscCount::created(RscType type)
{
   if (rscValid(type))
   {
      rscAdd(&sRscLive[type], 1);
   }
}

void UtlRscCount::destroyed(RscType type)
{
   if (rscValid(type))
   {
      rscAdd(&sRscLive[type], -1);
   }
}

long UtlRscCount::live(RscType type)
{
   return rscValid(type) ? rscRead(&sRscLive[type]) : 0;
}

void UtlRscCount::taskTerminated()
{
   rscAdd(&sRscTasksTerminated, 1);
}

long UtlRscCount::tasksTerminated()
{
   return rscRead(&sRscTasksTerminated);
}

const char* UtlRscCount::typeName(RscType type)
{
   return rscValid(type) ? sRscTypeNames[type] : NULL;
}

// Name-database prefixes of the named objects a snapshot lists. These are
// OsTaskBase::TASK_PREFIX and OsMsgQBase::MSGQ_PREFIX; spelled out here
// so a snapshot does not depend on those classes' static initialization.
static const char* const sRscNamePrefixes[] = { "Task.", "MsgQ." };

// Size of the buffer a snapshot reads names into.
#define RSC_NAMES_BUFFER_SIZE 65536

void utlRscReadLocal(UtlRscRawCounts& counts, char* names, int namesLen)
{
   for (int t = 0; t < UtlRscCount::NUM_TYPES; t++)
   {
      counts.live[t] = UtlRscCount::live((UtlRscCount::RscType)t);
   }
   counts.tasksTerminated = UtlRscCount::tasksTerminated();

   if (names == NULL || namesLen <= 0)
   {
      return;
   }
   names[0] = '\0';
   UtlSList keys;
   OsNameDb* pDb = OsNameDb::getNameDb();
   for (unsigned p = 0; p < sizeof(sRscNamePrefixes) / sizeof(sRscNamePrefixes[0]); p++)
   {
      pDb->getKeysWithPrefix(sRscNamePrefixes[p], keys);
   }
   int used = 0;
   UtlSListIterator it(keys);
   UtlString* pKey;
   while ((pKey = (UtlString*)it()) != NULL)
   {
      int len = (int)pKey->length();
      if (used + len + 2 > namesLen)
      {
         break;
      }
      memcpy(names + used, pKey->data(), len);
      used += len;
      names[used++] = '\n';
      names[used] = '\0';
   }
   keys.destroyAll();
}

static UtlRscSource sRscSource = NULL;

void UtlRscSnapshot::setSource(UtlRscSource source)
{
   sRscSource = source;
}

UtlRscSnapshot::UtlRscSnapshot()
: mTasksTerminated(0)
, mpNames(new UtlSList())
{
   take();
}

UtlRscSnapshot::~UtlRscSnapshot()
{
   mpNames->destroyAll();
   delete mpNames;
}

void UtlRscSnapshot::take()
{
   UtlRscRawCounts counts;
   char* names = new char[RSC_NAMES_BUFFER_SIZE];
   names[0] = '\0';
   (sRscSource ? sRscSource : utlRscReadLocal)(counts, names, RSC_NAMES_BUFFER_SIZE);

   for (int t = 0; t < UtlRscCount::NUM_TYPES; t++)
   {
      mLive[t] = counts.live[t];
   }
   mTasksTerminated = counts.tasksTerminated;
   utlRscGetProcessStats(mProcess);

   mpNames->destroyAll();
   const char* start = names;
   const char* end;
   while ((end = strchr(start, '\n')) != NULL)
   {
      mpNames->append(new UtlString(start, (size_t)(end - start)));
      start = end + 1;
   }
   delete[] names;
}

long UtlRscSnapshot::totalDelta(const UtlRscSnapshot& before) const
{
   long total = 0;
   for (int t = 0; t < UtlRscCount::NUM_TYPES; t++)
   {
      total += mLive[t] - before.mLive[t];
   }
   return total;
}

// Append " <name> <+delta>" and, when perOp > 0, "(<delta/perOp>/op)".
static void rscAppendDelta(UtlString& line, const char* name, long delta, int perOp)
{
   char buf[96];
   if (perOp > 0)
   {
      snprintf(buf, sizeof(buf), " %s %+ld(%+.2f/op)", name, delta,
               (double)delta / (double)perOp);
   }
   else
   {
      snprintf(buf, sizeof(buf), " %s %+ld", name, delta);
   }
   line.append(buf);
}

// Append " <tag>[a,b,...]" for names in "in" that are not in "notIn".
static void rscAppendNames(UtlString& line, const char* tag,
                           const UtlSList& in, const UtlSList& notIn, int maxNames)
{
   int found = 0;
   int shown = 0;
   UtlSListIterator it(in);
   UtlString* pName;
   while ((pName = (UtlString*)it()) != NULL)
   {
      if (notIn.contains(pName))
      {
         continue;
      }
      found++;
      if (shown < maxNames)
      {
         line.append(shown == 0 ? " " : ",");
         if (shown == 0)
         {
            line.append(tag);
            line.append("[");
         }
         line.append(*pName);
         shown++;
      }
   }
   if (found > 0)
   {
      if (found > shown)
      {
         char buf[32];
         snprintf(buf, sizeof(buf), ",...%d more", found - shown);
         line.append(buf);
      }
      line.append("]");
   }
}

void UtlRscSnapshot::formatDelta(const UtlRscSnapshot& before, const char* label,
                                 UtlString& line, int perOp, int maxNames) const
{
   line = "RSC ";
   line.append(label ? label : "");
   line.append(":");
   for (int t = 0; t < UtlRscCount::NUM_TYPES; t++)
   {
      const char* name = UtlRscCount::typeName((UtlRscCount::RscType)t);
      if (name)
      {
         rscAppendDelta(line, name, mLive[t] - before.mLive[t], perOp);
      }
   }
   rscAppendDelta(line, "killed", mTasksTerminated - before.mTasksTerminated, 0);
   if (mProcess.handles >= 0 && before.mProcess.handles >= 0)
   {
      rscAppendDelta(line, "handles", mProcess.handles - before.mProcess.handles, perOp);
   }
   if (mProcess.privateKB >= 0 && before.mProcess.privateKB >= 0)
   {
      rscAppendDelta(line, "privKB", mProcess.privateKB - before.mProcess.privateKB, perOp);
   }
   if (mProcess.threads >= 0 && before.mProcess.threads >= 0)
   {
      rscAppendDelta(line, "threads", mProcess.threads - before.mProcess.threads, perOp);
   }
   rscAppendNames(line, "new", *mpNames, *before.mpNames, maxNames);
   rscAppendNames(line, "gone", *before.mpNames, *mpNames, maxNames);
}

// ---------------------------------------------------------------------------
// Level 2: legacy per-object store, compiled out.
// ---------------------------------------------------------------------------

#ifdef RSC_TEST

// SYSTEM INCLUDES
#include <stdio.h>
#include <time.h>
#if defined(_WIN32)
#include <malloc.h>
#elif defined(_VXWORKS)
#include <stdLib.h>
#endif

// APPLICATION INCLUDES
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
#ifdef _VXWORKS
#include "mp/sa1100.h"
volatile int* osTimerCtr = (int*) SA1100_OSTIMER_COUNTER;
#endif

// EXTERNAL VARIABLES
// CONSTANTS
#define TIME_LENGTH             64;
#define HISTORY_LENGTH  1028;

// STATIC VARIABLE INITIALIZATIONS
unsigned long UtlRscTrace::sAllocCnt  = 0; // Number of memory allocs
unsigned long UtlRscTrace::sFreeCnt   = 0; // Number of memory frees
int           UtlRscTrace::sTraceFlag = 0; // If non-zero, trace calls to the
                                           //  new and delete operators
unsigned long UtlRscTrace::sStartTime  = 0; //
int           UtlRscTrace::mTaskId         = 0;

long              UtlRscTrace::mCheckpoint = 0L; // Used to remember the net number of 
OsMutex   *UtlRscTrace::mpResourceStoreLock = new OsMutex(OsMutex::Q_PRIORITY);
UtlRscStore UtlRscTrace::mUtlRscStore = UtlRscStore();

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlRscTrace::UtlRscTrace()
{
   mCheckpoint = UtlRscTrace::sAllocCnt - UtlRscTrace::sFreeCnt;
}

// Copy constructor
UtlRscTrace::UtlRscTrace(const UtlRscTrace& rUtlRscTrace)
{
   mCheckpoint = rUtlRscTrace.mCheckpoint;
}

// Destructor
UtlRscTrace::~UtlRscTrace()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
UtlRscTrace& 
UtlRscTrace::operator=(const UtlRscTrace& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   mCheckpoint = rhs.mCheckpoint;
   return *this;
}

// Remember the count of outstanding memory allocations
void UtlRscTrace::checkpoint()
{
   mCheckpoint = netAllocCnt();
}

void UtlRscTrace::addAllocCnt(int size,
                                                          int addr,
                                                          const char* name, 
                                                          int pArg, 
                                                          int priority, 
                                                          int options,
                                                          int taskId) 
{
        if (mTaskId && (mTaskId != taskId))
                return;

    mpResourceStoreLock->acquire();
        int traceFlag = sTraceFlag;
        sTraceFlag = 0;
        
        char buf[1024];
    long  ts;
#ifdef _VXWORKS
    ts = *osTimerCtr - sStartTime;
        sprintf(buf, "0x%08x 0x%08x %6d %12d %s %s %6d %6d %6d", taskId, addr, size, ts, taskName(taskId), 
                                                                                        name, pArg, priority, options);

#else 
        time( (time_t*)&ts );
        sprintf(buf, "0x%08x 0x%08x %6d %12d %s %6d %6d %6d", taskId, addr, size, ts, 
                                                                                        name, pArg, priority, options);
#endif

        if (OS_SUCCESS != mUtlRscStore.insert(addr, buf))
        {
//              printf("\nUtlRscStore.insert returned error.\n");
        }
        sTraceFlag = traceFlag;
        mpResourceStoreLock->release();
}

void UtlRscTrace::addAllocCnt(int addr,
                                                          const char* name, 
                                                          int taskId)
{
        if (mTaskId && (mTaskId != taskId))
                return;

    mpResourceStoreLock->acquire();
        int traceFlag = sTraceFlag;
        sTraceFlag = 0;
        
        char buf[1024];
    long  ts;
#ifdef _VXWORKS
    ts = *osTimerCtr - sStartTime;
        sprintf(buf, "0x%08x 0x%08x %12d %s %s", taskId, addr, ts, taskName(taskId), name);

#else 
        time( (time_t*)&ts );
        sprintf(buf, "0x%08x 0x%08x %12d %s", taskId, addr, ts, name);
#endif

        if (OS_SUCCESS != mUtlRscStore.insert(addr, buf))
        {
//              printf("\nUtlRscStore.insert returned error.\n");
        }
        sTraceFlag = traceFlag;
        mpResourceStoreLock->release();
}

void UtlRscTrace::addAllocCnt(int options,
                                                          int addr,
                                                          int state, 
                                                          int taskId)
{
        if (mTaskId && (mTaskId != taskId))
                return;

    mpResourceStoreLock->acquire();
        int traceFlag = sTraceFlag;
        sTraceFlag = 0;
        
        char buf[512];
    long  ts;
#ifdef _VXWORKS
    ts = *osTimerCtr - sStartTime;
        sprintf(buf, "0x%08x 0x%08x %6d %12d %s %6d", taskId, addr, options, ts, taskName(taskId), state);

#else 
        time( (time_t*)&ts );
        sprintf(buf, "0x%08x 0x%08x %6d %12d %6d %6d", taskId, addr, options, ts, state); 
#endif

        if (OS_SUCCESS != mUtlRscStore.insert(addr, buf))
        {
//              printf("\nUtlRscStore.insert returned error.\n");
        }
        sTraceFlag = traceFlag;
        mpResourceStoreLock->release();
}

void UtlRscTrace::addAllocCnt(int state,
                                                          int addr,
                                                          int timerId, 
                                                          int type, 
                                                          int taskId)
{
        if (mTaskId && (mTaskId != taskId))
                return;

    mpResourceStoreLock->acquire();
        int traceFlag = sTraceFlag;
        sTraceFlag = 0;
        
        char buf[512];
    long  ts;
#ifdef _VXWORKS
    ts = *osTimerCtr - sStartTime;
        sprintf(buf, "0x%08x 0x%08x %6d %12d %s %6d %6d", taskId, addr, state, ts, taskName(taskId), timerId, type);

#else 
        time( (time_t*)&ts );
        sprintf(buf, "0x%08x 0x%08x %6d %12d %6d %6d", taskId, addr, state, ts, timerId, type); 
#endif

        if (OS_SUCCESS != mUtlRscStore.insert(addr, buf))
        {
//              printf("\nUtlRscStore.insert returned error.\n");
        }
        sTraceFlag = traceFlag;
        mpResourceStoreLock->release();
}

void UtlRscTrace::addAllocCnt(int size,
                                                          int addr,
                                                          int taskId)
{
        if (mTaskId && (mTaskId != taskId))
                return;

    mpResourceStoreLock->acquire();
        int traceFlag = sTraceFlag;
        sTraceFlag = 0;
        
        char buf[512];
    long  ts;
#ifdef _VXWORKS
    ts = *osTimerCtr - sStartTime;
        sprintf(buf, "0x%08x 0x%08x %6d %12d %s", taskId, addr, size, ts, taskName(taskId));

#else 
        time( (time_t*)&ts );
        sprintf(buf, "0x%08x 0x%08x %6d %12d", taskId, addr, size, ts);
#endif

        if (OS_SUCCESS != mUtlRscStore.insert(addr, buf))
        {
//              printf("\nUtlRscStore.insert returned error.\n");
        }
        sTraceFlag = traceFlag;
        mpResourceStoreLock->release();
}
 
void UtlRscTrace::addAllocCnt(int addr,
                                                          int taskId)
{
        if (mTaskId && (mTaskId != taskId))
                return;

    mpResourceStoreLock->acquire();
        int traceFlag = sTraceFlag;
        sTraceFlag = 0;
        
        char buf[512];
    long  ts;
#ifdef _VXWORKS
    ts = *osTimerCtr - sStartTime;
        sprintf(buf, "0x%08x 0x%08x %12d %s", taskId, addr, ts, taskName(taskId));

#else 
        time( (time_t*)&ts );
        sprintf(buf, "0x%08x 0x%08x %12d", taskId, addr, ts);
#endif

        if (OS_SUCCESS != mUtlRscStore.insert(addr, buf))
        {
//              printf("\nUtlRscStore.insert returned error.\n");
        }
        sTraceFlag = traceFlag;
        mpResourceStoreLock->release();
}
 
void UtlRscTrace::addFreeCnt(int addr, int taskId)
{
        if (mTaskId && (mTaskId != taskId))
                return;

    mpResourceStoreLock->acquire();
        int traceFlag = sTraceFlag;
        sTraceFlag = 0;
        if (OS_SUCCESS != mUtlRscStore.remove(addr))
        {
//              printf("\nUtlRscStore.remove returned error: address - 0x%08x taskId - 0x%08x.\n", addr, taskId);
        }
        sTraceFlag = traceFlag;
        mpResourceStoreLock->release();
}


/* ============================ ACCESSORS ================================= */

// Return the change to the number of outstanding memory allocations
// since the last checkpoint.
int UtlRscTrace::delta()
{
   return netAllocCnt() - mCheckpoint;
}

int UtlRscTrace::delta(int taskId)
{
   return netAllocCnt(taskId) - mCheckpoint;
}

// Return the number of memory allocations (monotonically increasing)
int UtlRscTrace::allocCnt(int taskId)
{
        unsigned int size;
        char** activeRscs;
        int i;

    mpResourceStoreLock->acquire();
        int traceFlag = sTraceFlag;
        sTraceFlag = 0;
        size = mUtlRscStore.numEntries();

        activeRscs = (char**) new long[size];
        for (i = 0; i < size; i++)
        {
                activeRscs[i] = new char[256];
        }
        mUtlRscStore.getActiveRscs(activeRscs, size);
        sTraceFlag = traceFlag;
        mpResourceStoreLock->release();

        int id;
        unsigned long cnt = 0;
        printf("        total current alloc's: %d\n", size);
        switch (traceFlag)
        {
        case MEMORY:
                printf("  taskId     address     size    time   taskName\n");
                break;

        case OSSOCKET:
                printf("  taskId     address     time     taskName\n");
                break;
       
        case OSMSGQ:
                printf("  taskId     address     time     taskName           qName\n");
                break;
  
        case OSBSEM:
                printf("  taskId     address  options  time     taskName     state\n");
                break;

        case OSCSEM:
                printf("  taskId     address    queueOptions  time   taskName    maxCount   initCount  \n");
                break;

        case OSMUTEX:
        case OSRWMUTEX:
                printf("  taskId     address     options   time   taskName\n");
                break;

        case OSTIMER:
                printf("  taskId     address     state   time   taskName    timerId   type  \n");
                break;

        case OSTASK:
                printf("  taskId     address     size    time  taskName   taskName  pArg priority options\n");
                break;

        case NONE:
        default:
                break;
        }
        printf("============================================================================\n");
        if (taskId == 0)
        {
                for (i = 0; i < size; i++)
                {
                        cnt++;
                        printf("%s\n", activeRscs[i]);
                }
                printf("============================================================================\n");
                printf("        total current alloc's: %d\n", size);
        }
        else
        {
                for (i = 0; i < size; i++)
                {
                        sscanf(activeRscs[i], "0x%08x", &id);
                        if (id == taskId)
                        {
                                cnt++;
                                printf("%s\n", activeRscs[i]);
                        }
                }
                printf("============================================================================\n");
#ifdef _VXWORKS
                printf("current alloc's for %s 0x%08x: %d\n", taskName(taskId), taskId, cnt);
#else
                printf("current alloc's for 0x%08x: %d\n", taskId, cnt);
#endif
        }

        return cnt;
}

// Return the number of memory frees (monotonically increasing)
int UtlRscTrace::rscStatus()
{
        unsigned int nInserts;
        unsigned int nRemoves;

        mUtlRscStore.getStoreStats(nInserts, nRemoves);
        printf("inserts: %d removes: %d\n", nInserts, nRemoves);

        return (nInserts - nRemoves);
}

int UtlRscTrace::freeCnt(int taskId)
{
        unsigned int nInserts;
        unsigned int nRemoves;

        mUtlRscStore.getStoreStats(nInserts, nRemoves);
        printf("%d\n", nRemoves);

        return nRemoves;
}

// Return the net number of allocations (allocCnt - freeCnt)
int UtlRscTrace::netAllocCnt()
{
        int cnt;

        cnt = mUtlRscStore.numEntries();
        printf("%d\n", cnt);

        return cnt;
}

int UtlRscTrace::netAllocCnt(int taskId)
{
        int cnt;

        cnt = mUtlRscStore.numEntries();
        printf("%d\n", cnt);

        return cnt;
}

int UtlRscTrace::enableMemTracking(int taskId) 
{ 
#ifdef _VXWORKS
    sStartTime = *osTimerCtr;
#endif 
        mTaskId = taskId;
        sTraceFlag = MEMORY; 
        return mTaskId;
}

int UtlRscTrace::enableMsgQTracking(int taskId)
{
#ifdef _VXWORKS
    sStartTime = *osTimerCtr;
#endif 
        mTaskId = taskId;
        sTraceFlag = OSMSGQ; 
        return mTaskId;
}

int UtlRscTrace::enableBSemTracking(int taskId)
{
#ifdef _VXWORKS
    sStartTime = *osTimerCtr;
#endif 
        mTaskId = taskId;
        sTraceFlag = OSBSEM; 
        return mTaskId;
}

int UtlRscTrace::enableCSemTracking(int taskId)
{
#ifdef _VXWORKS
    sStartTime = *osTimerCtr;
#endif 
        mTaskId = taskId;
        sTraceFlag = OSCSEM; 
        return mTaskId;
}

int UtlRscTrace::enableMutexTracking(int taskId)
{
#ifdef _VXWORKS
    sStartTime = *osTimerCtr;
#endif 
        mTaskId = taskId;
        sTraceFlag = OSMUTEX; 
        return mTaskId;
}

int UtlRscTrace::enableRWMutexTracking(int taskId)
{
#ifdef _VXWORKS
    sStartTime = *osTimerCtr;
#endif 
        mTaskId = taskId;
        sTraceFlag = OSRWMUTEX; 
        return mTaskId;
}

int UtlRscTrace::enableTimerTracking(int taskId)
{
#ifdef _VXWORKS
    sStartTime = *osTimerCtr;
#endif 
        mTaskId = taskId;
        sTraceFlag = OSTIMER; 
        return mTaskId;
}

int UtlRscTrace::enableTaskTracking(int taskId)
{
#ifdef _VXWORKS
    sStartTime = *osTimerCtr;
#endif 
        mTaskId = taskId;
        sTraceFlag = OSTASK; 
        return mTaskId;
}

int UtlRscTrace::enableSocketTracking(int taskId)
{
#ifdef _VXWORKS
    sStartTime = *osTimerCtr;
#endif 
        mTaskId = taskId;
        sTraceFlag = OSSOCKET; 
        return mTaskId;
}

int UtlRscTrace::disableTracking() 
{ 
        allocCnt(0);
        sTraceFlag = NONE; 
        mTaskId = 0;
        mUtlRscStore.cleanUp();
        return sTraceFlag;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

#endif // RSC_TEST

