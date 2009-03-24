//  
// Copyright (C) 2009 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <mp/MpBuf.h>
#include <mp/MprHook.h>
#include <mp/MpPackedResourceMsg.h>
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprHook::MprHook(const UtlString& rName)
: MpAudioResource(rName, 0, 1, 0, 1)
, mpHook(NULL)
, mFrameNum(0)
{
}

// Destructor
MprHook::~MprHook()
{
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprHook::setHook(const UtlString& namedResource,
                          OsMsgQ& fgQ,
                          HookFunction func,
                          void *pUserData)
{
   OsStatus stat;
   MpPackedResourceMsg msg((MpResourceMsg::MpResourceMsgType)MPRM_SET_HOOK,
      namedResource);
   UtlSerialized &msgData = msg.getData();

   stat = msgData.serialize((void*)func);
   assert(stat == OS_SUCCESS);
   stat = msgData.serialize((void*)pUserData);
   assert(stat == OS_SUCCESS);
   msgData.finishSerialize();

   return fgQ.send(msg, sOperationQueueTimeout);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprHook::doProcessFrame(MpBufPtr inBufs[],
                                   MpBufPtr outBufs[],
                                   int inBufsSize,
                                   int outBufsSize,
                                   UtlBoolean isEnabled,
                                   int samplesPerFrame,
                                   int samplesPerSecond)
{
   // Increment frame number even if disabled.
   mFrameNum++;

   if (!isEnabled)
   {
      return TRUE;
   }

   assert(inBufsSize>0);

   // Call hook-function.
   if (mpHook != NULL)
   {
      mpHook(inBufs[0], mFrameNum, mpUserData);
   }

   // Pass processed buffer further
   if (outBufsSize > 0)
   {
      outBufs[0].swap(inBufs[0]);
   }

   return TRUE;
}

UtlBoolean MprHook::handleMessage(MpResourceMsg& rMsg)
{
   OsSysLog::add(FAC_MP, PRI_DEBUG,
                 "MprHook::handleMessage(%d)", rMsg.getMsg());
   switch (rMsg.getMsg())
   {
   case MPRM_SET_HOOK:
      {
         OsStatus stat;
         HookFunction pHook;
         void *pUserData;

         UtlSerialized &msgData = ((MpPackedResourceMsg*)(&rMsg))->getData();
         stat = msgData.deserialize((void*&)pHook);
         assert(stat == OS_SUCCESS);
         stat = msgData.deserialize((void*&)pUserData);
         assert(stat == OS_SUCCESS);

         mpHook = pHook;
         mpUserData = pUserData;
         return TRUE;
      }
      break;
   }
   return MpAudioResource::handleMessage(rMsg);
}

/* ============================ FUNCTIONS ================================= */

