//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpDecoderPayloadMap_h_
#define _MpDecoderPayloadMap_h_

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
class MpDecoderBase;

/// Helper class for MprDecode - it maps payload types to decoder instances.
class MpDecoderPayloadMap
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum {
      NUM_PAYLOAD_TYPES = (1<<8)
   };

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpDecoderPayloadMap();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Add an RTP payload type to decoder instance mapping table
   void addPayloadType(int payloadType, MpDecoderBase* pDecoder);

     /// Remove an RTP payload type from decoder instance map
   void deletePayloadType(int payloadType);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get decoder for this payload type
   MpDecoderBase* mapPayloadType(int payloadType) const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   MpDecoderBase *mpPayloadMap[NUM_PAYLOAD_TYPES];
                                     ///< Map RTP payload types to our decoders.

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpDecoderPayloadMap_h_
