//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include "mp/MpDecoderBase.h"

// Constructor
// Returns a new decoder object.
// param: payloadType - (in) RTP payload type associated with this decoder
MpDecoderBase::MpDecoderBase(int payloadType)
: mPayloadType(payloadType)
{
 // initializers do it all!
}

//Destructor
MpDecoderBase::~MpDecoderBase()
{
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean MpDecoderBase::setDtmfNotify(OsNotification* pNotify)
{
   assert(FALSE);
   return TRUE;
}

/* ============================ ACCESSORS ================================= */

// Returns the RTP payload type associated with this decoder.
int MpDecoderBase::getPayloadType(void)
{
   return(mPayloadType);
}

OsStatus MpDecoderBase::getSignalingData(uint8_t &key,
                                         UtlBoolean &isStarted,
                                         UtlBoolean &isStopped,
                                         uint16_t &duration)
{
   return OS_NOT_SUPPORTED;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
