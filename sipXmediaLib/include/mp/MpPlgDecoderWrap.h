//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifndef _PlgDecoderWrapper_h_
#define _PlgDecoderWrapper_h_

#include "mp/MpCodecFactory.h"

class MpPlgDecoderWrapper : public MpDecoderBase
{
public:

   MpPlgDecoderWrapper(int payloadType,
                       const MpCodecCallInfoV1& callInfo,
                       const MppCodecInfoV1_1& codecInfo,
                       const char* defaultFmtp);

   virtual ~MpPlgDecoderWrapper();

     /// Create decoder with parsing custom fmtp string
   virtual OsStatus initDecode(const char* codecFmtString);
     /// @copydoc MpDecoderBase::initDecode()
   virtual OsStatus initDecode();
     /// @copydoc MpDecoderBase::freeDecode()
   virtual OsStatus freeDecode();

     /// @copydoc MpDecoderBase::decode()
   virtual int decode(const MpRtpBufPtr &pPacket,
                      unsigned decodedBufferLength,
                      MpAudioSample *samplesBuffer);

     /// @copydoc MpDecoderBase::getSignalingData()
   virtual OsStatus getSignalingData(uint8_t &event,
                                     UtlBoolean &isStarted,
                                     UtlBoolean &isStopped,
                                     uint16_t &duration);

     /// @copydoc MpDecoderBase::getInfo()
   virtual const MpCodecInfo* getInfo(void) const;

protected:
   MpCodecInfo mCodecInfo;
   const MpCodecCallInfoV1& mCallInfo;
   UtlBoolean mInitialized;
   void* plgHandle;
   const char* mDefaultFmtp;

   UtlBoolean codecSupportPLC;

private:
   UtlBoolean initializeWrapper(const char *fmtp);

};

#endif
