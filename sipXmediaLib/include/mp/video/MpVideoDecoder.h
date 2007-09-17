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

#ifndef _MpVideoDecoder_h_
#define _MpVideoDecoder_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpVideoBuf.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
enum OsStatus;
class MpRtpBufPtr;
class MpBufPool;

/**
*  @brief Base class for family of video-decoder implementations.
*
*  Reassembles RTP payload and decodes video frames.
*/
class MpVideoDecoder
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{
   /// Constructor
   MpVideoDecoder(int payloadType ///< (in) RTP payload type associated with
                                  ///<  this decoder
         , MpBufPool *pVideoBufPool
          );

   /// Destructor
   virtual ~MpVideoDecoder();

     /// Initializes a codec data structure for use as a decoder
   virtual OsStatus initDecode() = 0;
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_NO_MEMORY</b> - Memory allocation failure
     */

     /// Frees all memory allocated to the decoder by <i>initDecode</i>
   virtual OsStatus freeDecode() = 0;
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_DELETED</b> - Object has already been deleted
     */
//@}


/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{
   /// Initialize stream
   virtual OsStatus initStream(const MpRtpBufPtr &pPacket) = 0;
    /**<
    *  When arrive first RTP packet in the stream it will be passed to this
    *  callback function. Then this packet will be then passed to decode()
    *  function as usual.
    *  
    *  @note initStream() implementation should not modify or release passed
    *        packet.
    * 
    *  @return OS_SUCCESS for now.
    */

   /// Decode incoming RTP packet
   virtual MpVideoBufPtr decode(const MpRtpBufPtr &pPacket ///< (in) Pointer to a media buffer
                               , bool& packetConsumed ///< (out) Is packet consumed by decoder
                                                      ///< or should be passed to next call to decoder.
                               , bool forceFlag = false ///< (in) Force frame decoding even
                                                        ///< if frame is still incomplete.
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
   MpBufPool* getVideoBufferPool() const;

/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:
   int         mPayloadType;     ///< RTP payload type for this codec.
   MpBufPool*  mpVideoBufPool;   ///< Buffer pool for decoded frames.
};

/* ============================ INLINE METHODS ============================ */
inline int MpVideoDecoder::getPayloadType() const
{
   return mPayloadType;
}

inline MpBufPool* MpVideoDecoder::getVideoBufferPool() const
{
   return mpVideoBufPool;
}

#endif  // _MpVideoDecoder_h_
