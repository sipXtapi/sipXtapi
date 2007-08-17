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


#ifdef HAVE_GIPS /* [ */

// If OLD_GIPS is defined, then we check that G.711 packets are an exact
// multiple of 10 milliseconds (80 samples), to avoid a bug in the older
// version of the GIPS libraries.  Currently, the only platform that is
// using the newer version of the libraries is the TCAS8 (XScale) hard phone.
#define OLD_GIPS
#ifdef CPU_XSCALE /* [ */
#undef  OLD_GIPS
#endif /* CPU_XSCALE ] */

#ifndef _MpdGIPSPCMA_h_
#define _MpdGIPSPCMA_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpDecoderBase.h"
#include "mp/GIPS/gips_typedefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/// Derived class for GIPS PCMA decoder.
class MpdGIPSPCMA: public MpDecoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpdGIPSPCMA( int payloadType ///< (in) RTP payload type associated with
                                ///<  this decoder
              );

     /// Destructor
   virtual
   ~MpdGIPSPCMA(void);

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
///@name Manipulators
//@{
#ifdef OLD_GIPS /* [ */

     /// Receive a packet of RTP data
   virtual int decodeIn(const MpRtpBufPtr &pPacket ///< (in) Pointer to a media buffer
                       );
     /**<
     *  @returns length of packet to hand to jitter buffer, 0 means don't.
     */

#endif /* OLD_GIPS ] */

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
   MpJitterBuffer* mpJBState;
};

#endif  // _MpdGIPSPCMA_h_
#endif /* HAVE_GIPS ] */
