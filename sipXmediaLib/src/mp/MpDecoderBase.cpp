//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#include <assert.h>
#include "mp/MpDecoderBase.h"
#include "mp/MpConnection.h"
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

int MpDecoderBase::decodeIn(MpBufPtr pPacket)
{
   return MpBuf_getContentLen(pPacket);
}

int MpDecoderBase::decode(JB_uchar *encoded, int inSamples, Sample *decoded)
{
	return 0;  // Base really can't do any decoding
}

int MpDecoderBase::reportBufferLength(int iAvePackets)
{
	// When implemented, this method allows a codec to take action based on the length of
	// the jitter buffer since last asked
	return 0;
}

void MpDecoderBase::FrameIncrement(void){
	return;
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

UtlBoolean MpDecoderBase::handleSetDtmfNotify(OsNotification* pNotify)
{
   assert(FALSE);
   return TRUE;
}

UtlBoolean MpDecoderBase::setDtmfTerm(MprRecorder *pRecorder)
{
   assert(FALSE);
   return TRUE;
}
