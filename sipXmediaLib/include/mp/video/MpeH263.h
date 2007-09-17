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

#ifndef _MpeH263_h_
#define _MpeH263_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/video/MpVideoEncoder.h"
#include "mp/MpVideoBuf.h"
#include "mp/MpRtpBuf.h"
#include "mp/video/MpVideoStreamParams.h"
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

/**
*  @brief AVCodec-based H.263 encoder implementation.
*/
class MpeH263: public MpVideoEncoder
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{
   /// Constructor
   MpeH263(int payloadType ///< (in) RTP payload type associated with
                                  ///<  this decoder
                  , MprToNet* pRtpWriter ///< (in) pointer to RTP writer utility
                 );

   /// Initializes a codec data structure for use as a decoder
   OsStatus initEncode(const MpVideoStreamParams* params);
   /**<
   *  @param params Settings for encoded video stream.
   *  @returns <b>OS_SUCCESS</b> - Success
   *  @returns <b>OS_NO_MEMORY</b> - Memory allocation failure
   */

   /// Frees all memory allocated to the decoder by <i>initDecode</i>
   OsStatus freeEncode();
   /**<
   *  @returns <b>OS_SUCCESS</b> - Success
   *  @returns <b>OS_DELETED</b> - Object has already been deleted
   */

   /// Destructor
   ~MpeH263();
//@}


/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{
   /// Encode incoming RTP packet
   OsStatus encode(const MpVideoBufPtr &pFrame ///< (in) Pointer to a media buffer
                          );

   void setAdvancedPrediction(UtlBoolean ap);
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
   MpVideoStreamParams  mStreamParams;
   AVCodec*             mpCodec;         ///< FFMpeg codec.
   AVCodecContext*      mpCodecContext;  ///< FFMpeg codec instance.
   AVFrame*             mpPicture;       ///< FFMpeg structure for raw video frame.
   UCHAR*               mpEncodedBuffer; ///< Buffer for raw encoded data.
   UCHAR*               mpPacket;        ///< Packetization buffer with Mode A header
   size_t               mEncodedBufferSize; ///< Size of mpEncodedBuffer.
   //UtlBoolean          mFirstFrame;
   UtlBoolean           mPacketReady;
   RtpTimestamp         mTimestamp;
   size_t               mPacketSize;
   h263_header_a*       mpHeader;
   UtlBoolean           mAp;

   static void rtpCallback(AVCodecContext* avctx, void *data, int size, int mb_nb);
   void rtpCallback(void *data, int data_size, int mb_nb);
   void flushReadyPacket(UtlBoolean lastFramePacket);
};

/* ============================ INLINE METHODS ============================ */

inline void MpeH263::setAdvancedPrediction(UtlBoolean ap)
{
   mAp = ap;
}

#endif  // _MpeH263_h_
