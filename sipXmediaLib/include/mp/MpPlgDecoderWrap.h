//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: 

#ifndef _PlgDecoderWrapper_h_
#define _PlgDecoderWrapper_h_

#include "mp/MpDecoderBase.h"
#include "mp/MpCodecFactory.h"

class MpPlgDecoderWrapper : public MpDecoderBase//, protected MpCodecInfo
{
   friend class MpCodecFactory;

protected:
   MpCodecInfo mpTmpInfo;
   const MpCodecCallInfoV1& mplgci;
   UtlBoolean mInitialized;
   UtlBoolean mSDPNumAssigned;
   void* plgHandle;
   const char* mDefParamString;

   UtlBoolean codecSupportPLC;

public:
   MpPlgDecoderWrapper(int payloadType, const MpCodecCallInfoV1& plgci, const char* permanentDefaultMode);
   virtual ~MpPlgDecoderWrapper();

   virtual OsStatus initDecode(const char* codecFmtString);
   virtual OsStatus initDecode();
   virtual OsStatus freeDecode();

   virtual int decode(const MpRtpBufPtr &pPacket,
      unsigned decodedBufferLength,
      MpAudioSample *samplesBuffer);

   virtual const MpCodecInfo* getInfo(void) const;
protected:
   OsStatus setAssignedSDPNum(SdpCodec::SdpCodecTypes sdpNum); //< SHOULD BE Removed after new API has been merged with sipX
private:
   UtlBoolean initializeWrapper(const char *fmt);

};

#endif
