//
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//  
// Copyright (C) 2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _MpdSipxILBC_h_  // [
#define _MpdSipxILBC_h_

#ifdef HAVE_ILBC // [

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

/// Internal iLBC codec structure (from iLBC_define.h)
struct iLBC_Dec_Inst_t_;

/// Derived class for iLBC decoder.
class MpdSipxILBC : public MpDecoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /// iLBC decoder mode (frame length).
   enum CodecMode
   {
      MODE_20MS=20, ///< 20ms frame length mode (15.200 kbps).
      MODE_30MS=30  ///< 30ms frame length mode (13.333 kbps).
   };

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpdSipxILBC(int payloadType, CodecMode mode);
     /**<
     *  @param[in] payloadType - RTP payload type associated with this decoder
     *  @param[in] mode - iLBC decoder mode (frame length).
     */

     /// Destructor
   virtual ~MpdSipxILBC();

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

     /// @copydoc MpDecoderBase::getInfo()
   virtual const MpCodecInfo* getInfo() const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

private:
   static const MpCodecInfo smCodecInfo20ms;  ///< Information about the codec in 20ms mode
   static const MpCodecInfo smCodecInfo30ms;  ///< Information about the codec in 30ms mode
   CodecMode         mMode;                   ///< iLBC decoder mode (frame length).
   iLBC_Dec_Inst_t_ *mpState;                 ///< Internal iLBC decoder state.
};

#endif // HAVE_ILBC ]

#endif  // _MpdSipxILBC_h_ ]
