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


#ifndef MP_AUDIO_FILE_DECOMPRESS_H
#define MP_AUDIO_FILE_DECOMPRESS_H

// SYSTEM INCLUDES
#include <os/iostream>
// APPLICATION INCLUDES
#include "mp/MpAudioAbstract.h"

/* =========================CLASS AbstractDecompressor=========================== */

class AbstractDecompressor {
protected:
   MpAudioAbstract &_dataSource;  // The object to get raw bytes from
   size_t readBytes(AudioByte *buff, size_t length);
public:
   AbstractDecompressor(MpAudioAbstract &a);
   virtual ~AbstractDecompressor();
   virtual size_t getSamples(AudioSample *, size_t) = 0;
   virtual void minMaxSamplingRate(long *, long *, long *);
   virtual void minMaxChannels(int *, int *, int *);
};

/* ==========================CLASS DecompressPcm8Signed========================= */

class DecompressPcm8Signed: public AbstractDecompressor {
public:
   DecompressPcm8Signed(MpAudioAbstract &a);
   //:Constructor
   size_t getSamples(AudioSample * buffer, size_t length);
   //:getSamples
};

/* ==========================CLASS DecompressPcm8Unsigned======================= */

class DecompressPcm8Unsigned: public AbstractDecompressor 
{
public:
   DecompressPcm8Unsigned(MpAudioAbstract &a);
   //:constructor
   size_t getSamples(AudioSample * buffer, size_t length);
   //: getSamples
};

/* ==================== CLASS DecompressPcm16MsbSigned=========================== */

class DecompressPcm16MsbSigned: public AbstractDecompressor {
public:
   DecompressPcm16MsbSigned(MpAudioAbstract &a);
   // constructor
   size_t getSamples(AudioSample *buffer, size_t length);
   // getSamples
};

/* ==========================CLASS DecompressPcm16LsbSigned======================= */

class DecompressPcm16LsbSigned: public AbstractDecompressor {
public:

   DecompressPcm16LsbSigned(MpAudioAbstract &a);
   //: Constructor 

   size_t getSamples(AudioSample *buffer, size_t length);
};

/* ==========================CLASS DecompressG711MuLaw======================= */

class DecompressG711MuLaw: public AbstractDecompressor {
public:
   DecompressG711MuLaw(MpAudioAbstract &a);
   size_t getSamples(AudioSample *buffer, size_t length);
};

AudioSample MuLawDecode(AudioByte);
AudioByte MuLawEncode(AudioSample);

/* ==========================CLASS DecompressG711ALaw======================= */

class DecompressG711ALaw: public AbstractDecompressor {
private:
   static AudioSample *_decodeTable;
public:
   DecompressG711ALaw(MpAudioAbstract &a);
   size_t getSamples(AudioSample *buffer, size_t length);
};

AudioSample ALawDecode(AudioByte);
AudioByte ALawEncode(AudioSample);

#endif
