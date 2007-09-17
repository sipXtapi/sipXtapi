//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 


#ifndef _MpdH264_h_
#define _MpdH264_h_

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

/// H.264 video stream decoder and RTP depacketizer.
/**
*  This class is aimed to restore H.264 frames from RTP packets (RTP
*  depacketization) and then decode them using FFMpeg library. So H.264 decoder
*  features are the same as FFMpeg H.264 decoder have. RTP depacketization
*  is compliant with RTP 3984 (RTP Payload Format for H.264 Video) and
*  support Single NAL Unit Mode and Non-Interleaved Mode. Interleaved Mode
*  is not supported - seems like it does not make big sense for video telephony.
*/
class MpdH264: public MpVideoDecoder
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpdH264( int payloadType ///< (in) RTP payload type associated with
                            ///<  this decoder
          , MpBufPool *pVideoBufPool
          );

     /// Destructor
   ~MpdH264();

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

/* ============================ MANIPULATORS ============================== */
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
                               , bool &packetConsumed ///< (out) Is packet consumed by decoder
                                                      ///< or should be passed to next call to decoder.
                               , bool forceFlag=false ///< (in) Force frame decoding even
                                                      ///< if frame is still incomplete.
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


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   AVCodec        *mpCodec;
   AVCodecContext *mpCodecContext;
   AVFrame        *mpPicture;

   enum {
      ENCODED_BUFFER_SIZE=(352*288*3) ///< Initial size of mpFrameBuf.
   };

   enum {
      PACKET_DROP,
      PACKET_COLLECT,
      PACKET_REPEAT
   };

   // Depacketizer variables.
   UCHAR   *mpFrameBuf;
   UCHAR   *mpFrameBufEnd;
   UCHAR   *mpDecFrameBuf;
   int      mDecFrameBufSize;
   int      mNALuOctet;
   int      mFrameFBit;
   RtpSeq   mPreviousSeqNum;        ///< Sequence number of last received packet
   RtpTimestamp mPreviousTimeStamp; ///< Timestamp of last received packet

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpdH264_h_
