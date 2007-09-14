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

#ifndef _PlgEncoderWrapper_h_
#define _PlgEncoderWrapper_h_

#include "mp/MpEncoderBase.h"
#include "mp/MpCodecFactory.h"

class MpPlgEncoderWrapper : public MpEncoderBase
{
   friend class MpCodecFactory;

protected:
   MpCodecInfo mCodecInfo;
   const MpCodecCallInfoV1& mplgci;
   UtlBoolean mInitialized;
   void* plgHandle;
   const char* mDefParamString;

public:
   MpPlgEncoderWrapper(int payloadType, const MpCodecCallInfoV1& plgci, const char* permanentDefaultMode);
   virtual ~MpPlgEncoderWrapper();

   virtual OsStatus initEncode(const char* codecFmtString);
   virtual OsStatus initEncode();
   virtual OsStatus freeEncode();

   virtual OsStatus encode(const MpAudioSample* pAudioSamples,
      const int numSamples,
      int& rSamplesConsumed,
      unsigned char* pCodeBuf,
      const int bytesLeft,
      int& rSizeInBytes,
      UtlBoolean& sendNow,
      MpAudioBuf::SpeechType& rAudioCategory); 

   virtual const MpCodecInfo* getInfo(void) const;

private:
   UtlBoolean initializeWrapper(const char *fmt);
};


#endif
