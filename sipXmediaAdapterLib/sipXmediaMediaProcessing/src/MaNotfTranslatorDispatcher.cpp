//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

// SYSTEM INCLUDES
#include <mp/MpResNotificationMsg.h>
#include <mp/MprnDTMFMsg.h>
#include <mp/MprnProgressMsg.h>

// APPLICATION INCLUDES
#include "MaNotfTranslatorDispatcher.h"
#include "mi/MiNotification.h"
#include "mi/MiDtmfNotf.h"
#include "mi/MiProgressNotf.h"
#include <mp/MpResNotificationMsg.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// FORWARD DECLARATIONS
/// Look up the media interface notification type associated with the mediaLib
/// notification type.
static
MiNotification::NotfType lookupNotfType(MpResNotificationMsg::RNMsgType rnMsgType);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MaNotfTranslatorDispatcher::MaNotfTranslatorDispatcher(OsMsgDispatcher* pAbstractedMsgDispatcher)
: mpAbstractedMsgDispatcher(pAbstractedMsgDispatcher)
{
}

MaNotfTranslatorDispatcher::~MaNotfTranslatorDispatcher()
{
}

/* ============================ MANIPULATORS ============================== */

OsMsgDispatcher* MaNotfTranslatorDispatcher::setDispatcher( OsMsgDispatcher* pMIDispatcher )
{
   OsMsgDispatcher* oldDispatcher = mpAbstractedMsgDispatcher;
   mpAbstractedMsgDispatcher = pMIDispatcher;
   return oldDispatcher;
}

OsStatus MaNotfTranslatorDispatcher::post(const OsMsg& msg)
{
   OsStatus stat = OS_FAILED;
   if(mpAbstractedMsgDispatcher)
   {
      // we should have a resource notification message - if not, something is wrong.
      assert(msg.getMsgType() == OsMsg::MP_RES_NOTF_MSG);
      if(msg.getMsgType() != OsMsg::MP_RES_NOTF_MSG)
      {
         // TODO: add a syslog msg.
         return OS_FAILED;
      }

      MpResNotificationMsg& resNotf = (MpResNotificationMsg&)msg;
      MpResNotificationMsg::RNMsgType notfType = 
         (MpResNotificationMsg::RNMsgType)(msg.getMsgSubType());
      switch(notfType)
      {
      case MpResNotificationMsg::MPRNM_FROMFILE_STARTED:
      case MpResNotificationMsg::MPRNM_FROMFILE_PAUSED:
      case MpResNotificationMsg::MPRNM_FROMFILE_RESUMED:
      case MpResNotificationMsg::MPRNM_FROMFILE_STOPPED:
      case MpResNotificationMsg::MPRNM_FROMFILE_FINISHED:
      case MpResNotificationMsg::MPRNM_BUFRECORDER_STOPPED:
      case MpResNotificationMsg::MPRNM_BUFRECORDER_FINISHED:
      case MpResNotificationMsg::MPRNM_BUFRECORDER_NOINPUTDATA:
         {
            MiNotification miNotf(lookupNotfType(notfType), 
                                  resNotf.getOriginatingResourceName());
            stat = mpAbstractedMsgDispatcher->post(miNotf);
         }
         break;
      case MpResNotificationMsg::MPRNM_DTMF_RECEIVED:
         {
            // In this case we know the message received is a DTMF notification..
            MprnDTMFMsg& dtmfMLibNotf = (MprnDTMFMsg&)resNotf;
            MiDtmfNotf dtmfMINotf(dtmfMLibNotf.getOriginatingResourceName(),
                                  (int)(dtmfMLibNotf.getConnectionId()),
                                  (MiDtmfNotf::KeyCode)dtmfMLibNotf.getKeyCode(), 
                                  (MiDtmfNotf::KeyPressState)dtmfMLibNotf.getKeyPressState(), 
                                  dtmfMLibNotf.getDuration());
            stat = mpAbstractedMsgDispatcher->post(dtmfMINotf);
         }
         break;
      case MpResNotificationMsg::MPRNM_FROMFILE_PROGRESS:
         {
            // In this case we know the message received is a progress notification.
            MprnProgressMsg& prgMLibNotf = (MprnProgressMsg&)resNotf;
            MiProgressNotf prgMINotf(prgMLibNotf.getOriginatingResourceName(),
                                     prgMLibNotf.getPositionMS(), 
                                     prgMLibNotf.getTotalMS());
            stat = mpAbstractedMsgDispatcher->post(prgMINotf);
         }
         break;
      default:
         // If we don't recognize the message, just pass it through
         // without any conversion.
         stat = mpAbstractedMsgDispatcher->post(msg);
      }
   }
   return stat;
}

OsStatus MaNotfTranslatorDispatcher::receive(OsMsg*& rpMsg, const OsTime& rTimeout)
{
   return OS_NOT_SUPPORTED;
}

/* ============================ ACCESSORS ================================= */

OsMsgDispatcher* MaNotfTranslatorDispatcher::getDispatcher() const
{
   return mpAbstractedMsgDispatcher;
}

/* ============================ INQUIRY =================================== */

UtlBoolean MaNotfTranslatorDispatcher::hasDispatcher() const
{
   return mpAbstractedMsgDispatcher ? TRUE : FALSE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

MiNotification::NotfType lookupNotfType( MpResNotificationMsg::RNMsgType rnMsgType )
{
   MiNotification::NotfType miNotfType;
   switch(rnMsgType)
   {
   case MpResNotificationMsg::MPRNM_FROMFILE_STARTED:
      miNotfType = MiNotification::MI_NOTF_PLAY_STARTED;
      break;
   case MpResNotificationMsg::MPRNM_FROMFILE_PAUSED:
      miNotfType = MiNotification::MI_NOTF_PLAY_PAUSED;
      break;
   case MpResNotificationMsg::MPRNM_FROMFILE_RESUMED:
      miNotfType = MiNotification::MI_NOTF_PLAY_RESUMED;
      break;
   case MpResNotificationMsg::MPRNM_FROMFILE_STOPPED:
      miNotfType = MiNotification::MI_NOTF_PLAY_STOPPED;
      break;
   case MpResNotificationMsg::MPRNM_FROMFILE_FINISHED:
      miNotfType = MiNotification::MI_NOTF_PLAY_FINISHED;
      break;
   case MpResNotificationMsg::MPRNM_BUFRECORDER_STOPPED:
      miNotfType = MiNotification::MI_NOTF_RECORD_STOPPED;
      break;
   case MpResNotificationMsg::MPRNM_BUFRECORDER_FINISHED:
      miNotfType = MiNotification::MI_NOTF_RECORD_FINISHED;
      break;
   case MpResNotificationMsg::MPRNM_BUFRECORDER_NOINPUTDATA:
      miNotfType = MiNotification::MI_NOTF_RECORD_NOINPUTDATA;
      break;
   case MpResNotificationMsg::MPRNM_DTMF_RECEIVED:
      miNotfType = MiNotification::MI_NOTF_DTMF_RECEIVED;
      break;
   case MpResNotificationMsg::MPRNM_FROMFILE_PROGRESS:
      miNotfType = MiNotification::MI_NOTF_PROGRESS;
      break;
   default:
      miNotfType = MiNotification::MI_NOTF_MESSAGE_INVALID;
   }
   return miNotfType;
}
