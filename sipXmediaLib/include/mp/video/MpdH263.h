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

#ifndef _MpdH263_h_
#define _MpdH263_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/video/MpVideoDecoder.h"
#include "mp/MpRtpBuf.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
struct AVCodecContext;
struct AVCodec;
struct AVFrame;
struct h263_header_a;
struct h263_header_b;

/**
*  @brief AVCodec-based H.263 encoder implementation.
*/
class MpdH263: public MpVideoDecoder
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{
   /// Constructor
   MpdH263(int payloadType ///< (in) RTP payload type associated with
                                  ///<  this decoder
         , MpBufPool *pVideoBufPool
          );

   /// Destructor
   ~MpdH263();

     /// Initializes a codec data structure for use as a decoder
   OsStatus initDecode();
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_NO_MEMORY</b> - Memory allocation failure
     */

     /// Frees all memory allocated to the decoder by <i>initDecode</i>
   OsStatus freeDecode();
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_DELETED</b> - Object has already been deleted
     */
//@}


/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{
   /// Initialize stream
   OsStatus initStream(const MpRtpBufPtr &pPacket);
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
   MpVideoBufPtr decode(const MpRtpBufPtr &pPacket ///< (in) Pointer to a media buffer
                        , bool& packetConsumed ///< (out) Is packet consumed by decoder
                                               ///< or should be passed to next call to decoder.
                        , bool forceFlag = false ///< (in) Force frame decoding even
                                                        ///< if frame is still incomplete.
                       );
//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{
//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{
//@}

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:

   /* /////////////////////////////// PRIVATE //////////////////////////////// */
private:

   AVCodec*             mpCodec;
   AVCodecContext*      mpCodecContext;
   AVFrame*             mpFrame;
   h263_header_a*       mpHeaderA;
   h263_header_b*       mpHeaderB;

   RtpSeq               mPreviousSeqNum;        ///< Sequence number of last received packet
   RtpTimestamp         mPreviousTimeStamp;     ///< Timestamp of last received packet

   UCHAR*               mpDecodeBuffer;
   size_t               mDecodeBufferSize;
   size_t               mAccuPayloadBits;
   UCHAR*               mpDecodePos;
   size_t               mDecodeLeft;

   UtlBoolean parseHeader(const MpRtpBufPtr &pPacket, const void*& payload, size_t& payloadSize, size_t& sbit, size_t& ebit);
   UtlBoolean accumulatePayload(const void* payload, size_t payloadSize, size_t sbit, size_t ebit);
   void flushDecoder();
   MpVideoBufPtr renderVideoBuffer();
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpdH263_h_
