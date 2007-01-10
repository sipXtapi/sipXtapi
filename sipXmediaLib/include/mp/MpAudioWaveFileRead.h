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


#ifndef MP_AUDIO_WAVE_FILE_READ_H
#define MP_AUDIO_WAVE_FILE_READ_H

#include "mp/MpAudioAbstract.h"
#include "mp/MpAudioFileDecompress.h"

bool isWaveFile(istream &file);

class MpAudioWaveFileRead: public MpAudioAbstract {
public:
   typedef enum {
      DePcm8Unsigned = 1,
      DePcm16LsbSigned,
      DeG711ALaw = 6,
      DeG711MuLaw = 7,
      DeImaAdpcm = 17
   } WaveDecompressionType;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

   /// Constructor form a stream
   MpAudioWaveFileRead(istream & s);

   /// Destructor
   ~MpAudioWaveFileRead();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

   size_t readBytes(AudioByte *buffer, size_t numSamples);

   size_t getBytesSize();

   int getDecompressionType();

//@}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   void minMaxSamplingRate(long *min, long *max, long *preferred);
   void minMaxChannels(int *min, int *max, int *preferred);
   size_t getSamples(AudioSample *buffer, size_t numSamples);
   void initializeDecompression();

   /// Get next chunk for chunk management
   void nextChunk(void);

private:
    //: private stream, ... stuff
   istream & mStream;
   AbstractDecompressor *_decoder;  ///< Current decompressor
   unsigned char *mpformatData;     ///< Contents of fmt chunk
   unsigned long mformatDataLength; ///< length of fmt chunk

private:
    //: Chunk stuff
   // WAVE chunk stack
   struct {
      unsigned long type;          ///< Type of chunk
      unsigned long size;          ///< Size of chunk
      unsigned long remaining;     ///< Bytes left to read
      bool isContainer;            ///< true if this is a container
      unsigned long containerType; ///< type of container
   } _chunk[5];
   int _currentChunk;              ///< top of stack
   unsigned long mFileSize;
};

#endif
