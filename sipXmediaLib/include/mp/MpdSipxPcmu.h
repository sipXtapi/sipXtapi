//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MpdSipxPcmu_h_
#define _MpdSipxPcmu_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpDecoderBase.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/// Derived class for G.711 u-Law decoder.
class MpdSipxPcmu: public MpDecoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

     /// Constructor
   MpdSipxPcmu(int payloadType);
     /**<
     *  @param payloadType - (in) RTP payload type associated with this decoder
     */

     /// Destructor
   virtual ~MpdSipxPcmu();

     /// Initializes a codec data structure for use as a decoder
   virtual OsStatus initDecode();
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_NO_MEMORY</b> - Memory allocation failure
     */

     /// Frees all memory allocated to the decoder by <i>initDecode</i>
   virtual OsStatus freeDecode();
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_DELETED</b> - Object has already been deleted
     */

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Decode incoming RTP packet
   virtual int decode(const MpRtpBufPtr &pPacket, ///< (in) Pointer to a media buffer
                      unsigned decodedBufferLength, ///< (in) Length of the samplesBuffer (in samples)
                      MpAudioSample *samplesBuffer ///< (out) Buffer for decoded samples
                     );
     /**<
     *  @return Number of decoded samples.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static const MpCodecInfo smCodecInfo;  ///< Static information about the codec
};

#endif  // _MpdSipxPcmu_h_
