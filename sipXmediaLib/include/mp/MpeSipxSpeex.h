//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 Hector Izquierdo Seliva. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _MpeSipxSpeex_h_
#define _MpeSipxSpeex_h_

#ifdef HAVE_SPEEX /* [ */

// APPLICATION INCLUDES
#include "mp/MpEncoderBase.h"
#include <speex/speex.h>
#include <speex/speex_header.h>
#include <speex/speex_stereo.h>
#include <speex/speex_preprocess.h>

/// Derived class for Speex encoder.
class MpeSipxSpeex: public MpEncoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpeSipxSpeex(int payloadType, int mode=3);
     /**<
     *  @param payloadType - (in) RTP payload type associated with this encoder
     *  @param mode - (in) Speex encoder mode: 
     *                     <pre>
     *                     mMode = 2 - 5,950 bps (Will use preprocess to make voice clearer)
     *                     mMode = 3 - 8,000 bps (default)
     *                     mMode = 4 - 11,000 bps
     *                     mMode = 5 - 15,000 bps
     *                     mMode = 6 - 18,200 bps
     *                     mMode = 7 - 24,600 bps
     *                     </pre>
     *                     If not supported mode will be passed, default mode
     *                     will be used.
     */

     /// Destructor
   virtual ~MpeSipxSpeex(void);

     /// Initializes a codec data structure for use as an encoder
   virtual OsStatus initEncode(void);
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_NO_MEMORY</b> - Memory allocation failure
     */

     /// Frees all memory allocated to the encoder by <i>initEncode</i>
   virtual OsStatus freeEncode(void);
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_DELETED</b> - Object has already been deleted
     */

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Encode audio samples
   virtual OsStatus encode(const MpAudioSample* pAudioSamples,
                           const int numSamples,
                           int& rSamplesConsumed,
                           unsigned char* pCodeBuf,
                           const int bytesLeft,
                           int& rSizeInBytes,
                           UtlBoolean& sendNow,
                           MpAudioBuf::SpeechType& rAudioCategory);
     /**<
     *  Processes the array of audio samples.  If sufficient samples to encode
     *  a frame are now available, the encoded data will be written to the
     *  <i>pCodeBuf</i> array.  The number of bytes written to the
     *  <i>pCodeBuf</i> array is returned in <i>rSizeInBytes</i>.
     *
     *  @param pAudioSamples - (in) Pointer to array of PCM samples
     *  @param numSamples - (in) number of samples at pAudioSamples
     *  @param rSamplesConsumed - (out) Number of samples encoded
     *  @param pCodeBuf - (out) Pointer to array for encoded data
     *  @param bytesLeft - (in) number of bytes available at pCodeBuf
     *  @param rSizeInBytes - (out) Number of bytes written to the <i>pCodeBuf</i> array
     *  @param sendNow - (out) if true, the packet is complete, send it.
     *  @param rAudioCategory - (out) Audio type (e.g., unknown, silence, comfort noise)
     *
     *  @returns <b>OS_SUCCESS</b> - Success
     */


//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static const MpCodecInfo smCodecInfo; ///< Static information about the codec

   SpeexBits mBits;         ///< Bits used by speex to store information
   void *mpEncoderState;    ///< State of the encoder   
   int mSampleRate;         ///< Sample rate
   int mMode;
     /**< Mode used.
      * From the Speex documentation:
      *
      * Mode  Bitrate  MFlops Quality
      *  0    250      N/A    No transmission (DTX)
      *  1    2,150    6      Vocoder (mostly for comfort noise)
      *  2    5,950    9      Very noticeable artifacts/noise, good intelligibility
      *  3    8,000    10     Artifacts/noise sometimes noticeable
      *  4    11,000   14     Artifacts usually noticeable only with headphones
      *  5    15,000   11     Need good headphones to tell the difference
      *  6    18,200   17.5   Hard to tell the difference even with good headphones
      *  7    24,600   14.5   Completely transparent for voice, good quality music
      *  8    3,950    10.5   Very noticeable artifacts/noise, good intelligibility
      */
   int mDoVad;              ///< Set to 1 to enable voice activity detection
   int mDoDtx;              ///< Set to 1 to enable discontinuous transmission
   int mDoVbr;              ///< Set to 1 to enable variable bitrate mode
   spx_int16_t mpBuffer[160]; ///< Buffer used to store input samples
   int mBufferLoad;          ///< How much data there is in the byffer
   bool mDoPreprocess;         ///< Should we do preprocess or not
   SpeexPreprocessState *mpPreprocessState; ///< Preprocessor state
   int mDoDenoise;             ///< Denoises the input
   int mDoAgc;                 ///< Automatic Gain Control

};

#endif /* HAVE_SPEEX ] */

#endif  // _MpeSipxSpeex_h_
