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
   MpCodecInfo mCodecInfo;
   const MpCodecCallInfoV1& mplgci;
   UtlBoolean mInitialized;
   void* plgHandle;
   const char* mDefParamString;

   UtlBoolean codecSupportPLC;

public:
   MpPlgDecoderWrapper(int payloadType, const MpCodecCallInfoV1& plgci, const char* permanentDefaultMode);
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

private:
   UtlBoolean initializeWrapper(const char *fmt);

};

#endif
