//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpDecoderPayloadMap.h"
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpDecoderPayloadMap::MpDecoderPayloadMap()
{
   for (int i=0; i<NUM_PAYLOAD_TYPES; i++) {
      mpPayloadMap[i] = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

void MpDecoderPayloadMap::addPayloadType(int payloadType, MpDecoderBase* pDecoder)
{
   // Check that payloadType is valid.
   if ((payloadType < 0) || (payloadType >= NUM_PAYLOAD_TYPES))
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpRtpInputAudioConnection::addPayloadType"
         " Attempting to add an invalid payload type %d",
         payloadType);
   }
   // Check to see if we already have a decoder for this payload type.
   else if (!(NULL == mpPayloadMap[payloadType]))
   {
      // This condition probably indicates that the sender of SDP specified
      // two decoders for the same payload type number.
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpRtpInputAudioConnection::addPayloadType"
         " Attempting to add a second decoder for payload type %d",
         payloadType);
   }
   else
   {
      mpPayloadMap[payloadType] = pDecoder;
   }

}

void MpDecoderPayloadMap::deletePayloadType(int payloadType)
{
   // Check that payloadType is valid.
   if ((payloadType < 0) || (payloadType >= NUM_PAYLOAD_TYPES))
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpRtpInputAudioConnection::deletePayloadType"
         " Attempting to delete an invalid payload type %d",
         payloadType);
   }
   // Check to see if this entry has already been deleted.
   else if (NULL == mpPayloadMap[payloadType])
   {
      // Seems this payload type is already deleted.
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpRtpInputAudioConnection::deletePayloadType"
         " Attempting to delete again payload type %d",
         payloadType);
   }
   else
   {
      mpPayloadMap[payloadType] = NULL;
   }

}

/* ============================ ACCESSORS ================================= */

MpDecoderBase* MpDecoderPayloadMap::mapPayloadType(int payloadType) const
{
   if ((payloadType < 0) || (payloadType >= NUM_PAYLOAD_TYPES))
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
         "MpRtpInputAudioConnection::mapPayloadType"
         " Attempting to map an invalid payload type %d",
         payloadType);
      return NULL;
   }
   else
   {
      return mpPayloadMap[payloadType];
   }

}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */