//  
// Copyright (C) 2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2007 Wirtualna Polska S.A. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Andrzej Ciarkowski <andrzejc AT wp-sa DOT pl>

#ifndef _MpVideoEncoder_h_
#define _MpVideoEncoder_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
enum OsStatus;
class MprToNet;
class MpVideoStreamParams;
class MpVideoBufPtr;

/**
*  @brief Base class for family of video-encoder implementations.
*
*  Encodes, performs RTP packetization and writes down the wire video 
*  frames using provided MprToNet instance as a sink.
*/
class MpVideoEncoder
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{
   /// Constructor
   MpVideoEncoder(int payloadType ///< (in) RTP payload type associated with
                                  ///<  this decoder
                  , MprToNet* pRtpWriter ///< (in) pointer to RTP writer utility
                 );

   /// Initializes a codec data structure for use as a decoder
   virtual OsStatus initEncode(const MpVideoStreamParams* params) = 0;
   /**<
   *  @param params Settings for encoded video stream.
   *  @returns <b>OS_SUCCESS</b> - Success
   *  @returns <b>OS_NO_MEMORY</b> - Memory allocation failure
   */

   /// Frees all memory allocated to the decoder by <i>initDecode</i>
   virtual OsStatus freeEncode() = 0;
   /**<
   *  @returns <b>OS_SUCCESS</b> - Success
   *  @returns <b>OS_DELETED</b> - Object has already been deleted
   */

   /// Destructor
   virtual ~MpVideoEncoder();
//@}


/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{
   /// Encode incoming RTP packet
   virtual OsStatus encode(const MpVideoBufPtr &pFrame ///< (in) Pointer to a media buffer
                          ) = 0;
//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{
   /// Query configured payload type.
   int getPayloadType() const;
//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{


//@}

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:
   MprToNet* getRtpSink() const;

/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:
   int         mPayloadType;     ///< RTP payload type for this codec.
   MprToNet*   mpRtpSink;        ///< We will pass encoded data to this RTP writer.
};

/* ============================ INLINE METHODS ============================ */
inline 
int MpVideoEncoder::getPayloadType() const
{
   return mPayloadType;
}

inline 
MprToNet* MpVideoEncoder::getRtpSink() const
{
   return mpRtpSink;
}

#endif  // _MpVideoEncoder_h_
