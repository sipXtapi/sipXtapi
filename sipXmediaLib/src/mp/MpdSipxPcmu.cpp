//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#if 1 /* [ */
#ifndef HAVE_GIPS /* [ */

// APPLICATION INCLUDES
#include "mp/MpConnection.h"
#include "mp/MpdSipxPcmu.h"
#include "mp/JB/JB_API.h"
#include "mp/MprDejitter.h"

const MpCodecInfo MpdSipxPcmu::smCodecInfo(
         SdpCodec::SDP_CODEC_PCMU, JB_API_VERSION, true,
         8000, 8, 1, 160, 64000, 1280, 1280, 1280, 160);

MpdSipxPcmu::MpdSipxPcmu(int payloadType)
   : MpDecoderBase(payloadType, &smCodecInfo)
{
}

MpdSipxPcmu::~MpdSipxPcmu()
{
   freeDecode();
}

OsStatus MpdSipxPcmu::initDecode(MpConnection* pConnection)
{
   //Get JB pointer
   pJBState = pConnection->getJBinst();

   // Set the payload number for JB
   JB_initCodepoint(pJBState, "PCMU", 8000, getPayloadType());

   return OS_SUCCESS;
}

OsStatus MpdSipxPcmu::freeDecode(void)
{
   return OS_SUCCESS;
}

#endif /* HAVE_GIPS ] */
#endif /* ] */
