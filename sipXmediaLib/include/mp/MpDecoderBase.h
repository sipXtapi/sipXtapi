//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpDecoderBase_h_
#define _MpDecoderBase_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "mp/MpCodecInfo.h"
#include "mp/MpRtpBuf.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class MpConnection;
class OsNotification;
class MprRecorder;

/// Base class for all media processing decoders.
class MpDecoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   friend class MprDecode;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpDecoderBase( int payloadType ///< (in) RTP payload type associated with
                                  ///<  this decoder
                , const MpCodecInfo* pInfo ///< (in) pointer to derived class'
                                           ///<  static const MpCodecInfo
                );

     /// Destructor
   virtual
   ~MpDecoderBase();

     /// Initializes a codec data structure for use as a decoder
   virtual OsStatus initDecode(MpConnection* pConnection)=0;
     /**<
     //!param: pConnection - (in) Pointer to the MpConnection container
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_NO_MEMORY</b> - Memory allocation failure
     */

     /// Frees all memory allocated to the decoder by <i>initDecode</i>
   virtual OsStatus freeDecode(void)=0;
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_DELETED</b> - Object has already been deleted
     */

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get static information about the decoder
   virtual const MpCodecInfo* getInfo(void) const;
     /**<
     *  @returns a pointer to an <i>MpCodecInfo</i> object that provides
     *  static information about the decoder.
     */

     /// Returns the RTP payload type associated with this decoder.
   virtual int getPayloadType(void);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

     /// Receive a packet of RTP data
   virtual int decodeIn(MpRtpBufPtr &pPacket ///< (in) Pointer to a media buffer
                       );
     /**<
     *  @returns length of packet to hand to jitter buffer, 0 means don't.
     */

     /// Handle the FLOWGRAPH_SET_DTMF_NOTIFY message.
   virtual UtlBoolean handleSetDtmfNotify(OsNotification* pNotify);

   virtual UtlBoolean setDtmfTerm(MprRecorder *pRecorder);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor
   MpDecoderBase(const MpDecoderBase& rMpDecoderBase);

     /// Assignment operator
   MpDecoderBase& operator=(const MpDecoderBase& rhs);

   const MpCodecInfo* mpCodecInfo;
   int mPayloadType;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpDecoderBase_h_
