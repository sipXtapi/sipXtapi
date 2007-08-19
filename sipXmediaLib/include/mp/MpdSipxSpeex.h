//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2006-2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 Hector Izquierdo Seliva. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _MpdSipxSpeex_h_  /* [ */
#define _MpdSipxSpeex_h_

#ifdef HAVE_SPEEX /* [ */

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpDecoderBase.h"
#include <speex/speex.h>
#include <speex/speex_header.h>
#include <speex/speex_stereo.h>
#include <speex/speex_preprocess.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/// Derived class for Speex  decoder.
class MpdSipxSpeex: public MpDecoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpdSipxSpeex(int payloadType);
     /**<
     *  @param payloadType - (in) RTP payload type associated with this decoder
     */

     /// Destructor
   virtual
   ~MpdSipxSpeex();

     /// Initializes a codec data structure for use as a decoder
   virtual OsStatus initDecode();
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_NO_MEMORY</b> - Memory allocation failure
     */

     /// Frees all memory allocated to the decoder by <i>initDecode</i>
   virtual OsStatus freeDecode(void);
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_DELETED</b> - Object has already been deleted
     */

//@}

/* ============================ MANIPULATORS ============================== */

     /// Decode incoming RTP packet
   virtual int decode(const MpRtpBufPtr &pPacket, ///< (in) Pointer to a media buffer
                      unsigned decodedBufferLength, ///< (in) Length of the samplesBuffer (in samples)
                      MpAudioSample *samplesBuffer ///< (out) Buffer for decoded samples
                     );
     /**<
     *  @return Number of decoded samples.
     */

///@name Manipulators
//@{

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

private:
   static const MpCodecInfo smCodecInfo;  ///< static information about the codec
   void        *mpDecoderState;
   SpeexBits    mDecbits;
   unsigned     mNumSamplesPerFrame;
};

#endif /* HAVE_SPEEX ] */

#endif  // _MpdSipxSpeex_h_ ]
