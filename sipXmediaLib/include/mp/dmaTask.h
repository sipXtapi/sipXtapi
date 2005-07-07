// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _INCLUDED_DMATASK_H /* [ */
#define _INCLUDED_DMATASK_H

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsTask.h"
#include "os/OsBSem.h"
#include "mp/MpMisc.h"

// DEFINES
/* the maximum number of buffers in input or output queue */
#define DMA_QLEN 10

// MACROS
// EXTERNAL FUNCTIONS
int showFrameCount(int silent);
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef void (*MpDma_MuteListenerFuncPtr)(bool);

// FORWARD DECLARATIONS
extern OsStatus dmaStartup(int samplesPerFrame);/* initialize the DMA driver */
extern void dmaShutdown(void);                  /* release the DMA driver */

extern int unMuteSpkr(void);
extern int muteSpkr(void);
extern int unMuteMic(void);
extern int muteMic(void);

enum MpDmaMicChoice {
    MP_MIC_SELECT_NEITHER,
    MP_MIC_SELECT_HANDSET,
    MP_MIC_SELECT_BASE,
};

extern OsStatus MpDma_selectMic(MpDmaMicChoice choice);
extern MpDmaMicChoice MpDma_getMicMode(void);

// The MpDma_setRingerEnabled function allows external modules to control
// whether the device's ringer is enabled.
bool MpDma_setRingerEnabled(bool enabled);

// The following functions allow external modules to monitor and manage
// whether the device is "muted".
void MpDma_setMuteListener(MpDma_MuteListenerFuncPtr pFunc);
bool MpDma_setMuteEnabled(bool enabled);
bool MpDma_isMuteEnabled(void);

extern unsigned short       gTableSize;
extern unsigned short       shpAttenTable[];

#ifdef WIN32 /* [ */
extern int DmaTask_setSpkrQPreload(int qlen);
extern int DmaTask_setMicQPreload(int qlen);
#endif /* WIN32 ] */

class DmaTask : public OsTask
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   static DmaTask* getDmaTask(int samplesPerFrame = 80);
     //:Return a pointer to the DMA task, creating it if necessary

   virtual
   ~DmaTask();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
   virtual int run(void* pArg);

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   DmaTask(MSG_Q_ID doneQ = NULL, int samplesPerFrame = 0,
      int prio    = DEF_DMA_TASK_PRIORITY,      // default task priority
      int options = DEF_DMA_TASK_OPTIONS,       // default task options
      int stack   = DEF_DMA_TASK_STACKSIZE);    // default task stacksize
     //:Default constructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   static const int DEF_DMA_TASK_PRIORITY;      // default task priority
   static const int DEF_DMA_TASK_OPTIONS;       // default task options
   static const int DEF_DMA_TASK_STACKSIZE;     // default task stacksize

   int       mFrameSamples; // Number of samples per frame
   MSG_Q_ID  mDoneQ;        // Message queue to wait on

   // Static data members used to enforce Singleton behavior
   static DmaTask* spInstance;  // pointer to the single instance of
                                    //  the MpDmaTask class
   static OsBSem       sLock;       // semaphore used to ensure that there
                                    //  is only one instance of this class

   DmaTask(const DmaTask& rDmaTask);
     //:Copy constructor (not implemented for this task)

   DmaTask& operator=(const DmaTask& rhs);
     //:Assignment operator (not implemented for this task)

};

/*************************************************************************/
#endif /* _INCLUDED_DMATASK_H ] */
