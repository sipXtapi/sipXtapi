//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MpSipxDecoders_h_
#define _MpSipxDecoders_h_

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

/// Class for managing dejitter/decode of incoming RTP.
class MpSipxDecoder
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpSipxDecoder(void);

     /// Destructor
   virtual
   ~MpSipxDecoder(void);

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

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

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor
   MpSipxDecoder(const MpSipxDecoder& rMpSipxDecoder);

     /// Assignment operator
   MpSipxDecoder& operator=(const MpSipxDecoder& rhs);

};

#ifdef __cplusplus
extern "C" {
#endif

extern int G711A_Decoder(int N, const JB_uchar* S, MpAudioSample* D);
extern int G711U_Decoder(int N, const JB_uchar* S, MpAudioSample* D);

#ifdef __cplusplus
}
#endif

#endif  // _MpSipxDecoders_h_
