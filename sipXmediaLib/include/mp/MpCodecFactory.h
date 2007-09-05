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


#ifndef _MpCodecFactory_h_
#define _MpCodecFactory_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "os/OsBSem.h"
#include "sdp/SdpCodec.h"
#include "mp/MpEncoderBase.h"
#include "mp/MpDecoderBase.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpFlowGraphBase;


/**<
*  Singleton class used to generate encoder and decoder objects of an indicated type.
*/
class MpCodecFactory
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{
     /// Get/create singleton factory.
   static MpCodecFactory* getMpCodecFactory(void);
     /**<
     *  Return a pointer to the MpCodecFactory singleton object, creating 
     *  it if necessary
     */

     /// Destructor
   virtual
   ~MpCodecFactory();
 
//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
     /// Returns a new instance of a decoder of the indicated type
   OsStatus createDecoder(SdpCodec::SdpCodecTypes internalCodecId,
                          int payloadType,
                          MpDecoderBase*& rpDecoder,
                          MpConnectionID connId,
                          MpFlowGraphBase* pFlowGraph = NULL);
   /**<
   *  @param[in]  internalCodecId - codec type identifier
   *  @param[in]  payloadType - RTP payload type associated with this decoder
   *  @param[out] rpDecoder - Reference to a pointer to the new decoder object
   *  @param[in]  connId - The ID of the connection this decoder is associated with.
   *  @param[in]  pFlowGraph - Pointer to the flowgraph that the decoder is part of. (used for notifications)
   */

     /// Returns a new instance of an encoder of the indicated type
   OsStatus createEncoder(SdpCodec::SdpCodecTypes internalCodecId,
                          int payloadType,
                          MpEncoderBase*& rpEncoder);
     /**<
     *  @param[in]  internalCodecId - codec type identifier
     *  @param[in]  payloadType - RTP payload type associated with this encoder
     *  @param[out] rpEncoder - Reference to a pointer to the new encoder object
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

     /// Constructor (called only indirectly via getMpCodecFactory())
   MpCodecFactory();
     /**<
     *  We identify this as a protected (rather than a private) method so
     *  that gcc doesn't complain that the class only defines a private
     *  constructor and has no friends.
     */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   // Static data members used to enforce Singleton behavior
   static MpCodecFactory* spInstance; //< pointer to the single instance of
                                      //<  the MpCodecFactory class.
   static OsBSem sLock; //< semaphore used to ensure that there is only one 
                        //< instance of this class.

     /// Copy constructor (not supported)
   MpCodecFactory(const MpCodecFactory& rMpCodecFactory);

     /// Assignment operator (not supported)
   MpCodecFactory& operator=(const MpCodecFactory& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpCodecFactory_h_
