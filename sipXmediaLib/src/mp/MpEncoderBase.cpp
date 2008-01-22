//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#include "mp/MpEncoderBase.h"

/* ============================ CREATORS ================================== */

// Constructor
// Returns a new encoder object.
// param: payloadType - (in) RTP payload type associated with this encoder
MpEncoderBase::MpEncoderBase(int payloadType)
: mPayloadType(payloadType)
{
}
    
// Destructor
MpEncoderBase::~MpEncoderBase()
{
}

/* ============================ ACCESSORS ================================= */

//:Returns the RTP payload type associated with this encoder.
int MpEncoderBase::getPayloadType(void)
{
   return(mPayloadType);
}
