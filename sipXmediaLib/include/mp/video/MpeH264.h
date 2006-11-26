//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 


#ifndef _MpeH264_h_
#define _MpeH264_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpVideoBuf.h"
#include "mp/MpRtpBuf.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MprToNet;
class MpCodecInfo;
struct AVCodecContext;
struct AVCodec;
struct AVFrame;

/// H.264 video stream encoder (use x264).
class MpeH264
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpeH264( int payloadType ///< (in) RTP payload type associated with
                            ///<  this decoder
          , MprToNet* pRtpWriter ///< (in) pointer to RTP writer utility
          );

     /// Destructor
   virtual
   ~MpeH264();

     /// Initializes a codec data structure for use as a decoder
   virtual OsStatus initEncode();
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_NO_MEMORY</b> - Memory allocation failure
     */

     /// Frees all memory allocated to the decoder by <i>initDecode</i>
   virtual OsStatus freeEncode();
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_DELETED</b> - Object has already been deleted
     */

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Decode incoming RTP packet
   virtual OsStatus encode(const MpVideoBufPtr &pFrame ///< (in) Pointer to a media buffer
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

   int             mPayloadType;    ///< RTP payload type for this codec.
   MprToNet       *mpRtpWriter;     ///< We will pass encoded data to this RTP writer.
   AVCodec        *mpCodec;         ///< FFMpeg codec (x264).
   AVCodecContext *mpCodecContext;  ///< FFMpeg codec instance.
   AVFrame        *mpPicture;       ///< FFMpeg structure for raw video frame.
   UCHAR          *mpEncodedBuffer; ///< Buffer for raw encoded data.
   int             mEncodedBufferSize; ///< Size of mpEncodedBuffer.

   enum {
      ENCODED_BUFFER_SIZE=(320*240*3) ///< Initial size of mpEncodedBuffer.
   };

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpeH264_h_
