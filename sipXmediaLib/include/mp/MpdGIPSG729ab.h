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
#ifdef HAVE_G729

#ifndef _MpdGIPSG729ab_h_
#define _MpdGIPSG729ab_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpDecoderBase.h"
#include "mp/GIPS/gips_typedefs.h"

#include "mp/iG729/_ippdefs.h"
#include "mp/iG729/ippdefs.h"
#include "mp/iG729/_p729.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/// Derived class for G.729 decoder.
class MpdGIPSG729ab: public MpDecoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpdGIPSG729ab( int payloadType ///< (in) RTP payload type associated with
                                  ///<  this decoder
                );

     /// Destructor
   virtual
   ~MpdGIPSG729ab(void);

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

     /// Receive a packet of RTP data
   virtual int decodeIn(const MpRtpBufPtr &pPacket ///< (in) Pointer to a media buffer
                       );
     /**<
     *  @note This method can be called more than one time per frame interval.
     *
     *  @returns >0 - length of packet to hand to jitter buffer.
     *  @returns 0  - decoder don't want more packets.
     *  @returns -1 - discard packet (e.g. out of order packet).
     */

   virtual OsStatus createDecoder(void);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   virtual IppsG729DecoderStruct* getDecoder(void);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static const MpCodecInfo smCodecInfo;  ///< static information about the codec
   MpJitterBuffer* mpJBState;
   IppsG729DecoderStruct* mpDecoderState;
};

#endif  // _MpdGIPSG729ab_h_
#endif /* HAVE_G729 */
#endif /* HAVE_GIPS ] */
