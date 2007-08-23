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
MpDecoderBase::MpDecoderBase(int payloadType, const MpCodecInfo* pInfo) :
   mpCodecInfo(pInfo),
   mPayloadType(payloadType)
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

// Get static information about the decoder
// Returns a pointer to an <i>MpCodecInfo</i> object that provides
// static information about the decoder.
const MpCodecInfo* MpDecoderBase::getInfo(void) const
{
   return(mpCodecInfo);
}


// Returns the RTP payload type associated with this decoder.
int MpDecoderBase::getPayloadType(void)
{
   return(mPayloadType);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
