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

#include "utl/UtlLink.h"
#include "utl/UtlVoidPtr.h"
#include "utl/UtlList.h"
#include "os/OsSharedLibMgr.h"
#include "mp/codecs/PlgDefsV1.h"
#include "mp/MpPlgStaffV1.h"
#include "mp/MpMisc.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpFlowGraphBase;
class MpCodecSubInfo;


/**
*  Singleton class used to generate encoder and decoder objects of an indicated type.
*/
class MpCodecFactory
{
   friend OsStatus mpShutdown();
   friend class MpCodecCallInfoV1;

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
                          MpDecoderBase*& rpDecoder);
     /**<
     *  @param[in]  internalCodecId - codec type identifier
     *  @param[in]  payloadType - RTP payload type associated with this decoder
     *  @param[out] rpDecoder - Reference to a pointer to the new decoder object
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
   SdpCodec::SdpCodecTypes* getAllCodecTypes(unsigned& count);
   const char** getAllCodecModes(SdpCodec::SdpCodecTypes codecId, unsigned& count);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

     /// Constructor (called only indirectly via getMpCodecFactory())
   MpCodecFactory();
     /**<
     *  We identify this as a protected (rather than a private) method so
     *  that gcc doesn't complain that the class only defines a private
     *  constructor and has no friends.
     */

     /// Search codec by given MIME-subtype
   MpCodecSubInfo* searchByMIME(UtlString& mime) const;

public:
   static MpCodecCallInfoV1* addStaticCodec(MpCodecCallInfoV1* sStaticCode);   

     /// Load specified codec and add it to codec list
   OsStatus loadDynCodec(const char* name);
     /// Load all codec with specified path and filter. Useful to load all libs in pugins directory
   OsStatus loadAllDynCodecs(const char* path, const char* regexFilter);

     /// Initialize all static codecs. Should be called only from mpStartup() 
   void initializeStaticCodecs();

protected:
     /// Deinitialize all dynamic codecs.  Should be called only from mpShutdown() 
   void freeAllLoadedLibsAndCodec();
     /// Deinitialize all static codecs and freeing handle. Should be called only mpShutdown()
   static void freeSingletonHandle();
     /// Freeing internal data of static codecs.  Should be called only from global .dtor
   static void globalCleanUp();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int maxDynamicCodecTypeAssigned; ///< Maximum number of dynamically assigned SdpCodecType
   UtlBoolean fCacheListMustUpdate; ///< Flag points that cached array must be rebuilt in the next call 
   SdpCodec::SdpCodecTypes* pCodecs; ///< Cached array of known codecs
   
   UtlSList mCodecsInfo; ///< list of all known and workable codecs

   // Static data members used to enforce Singleton behavior
   static MpCodecFactory* spInstance; ///< pointer to the single instance of
                                      ///<  the MpCodecFactory class.
   static OsBSem sLock; ///< semaphore used to ensure that there is only one 
                        ///< instance of this class.
   static MpCodecCallInfoV1* sStaticCodecsV1; ///< List of all static codecs. Filled by global magic .ctor

   void updateCodecArray(void); ///< not implemented yet
   OsStatus addCodecWrapperV1(MpCodecCallInfoV1* wrapper); ///< Build 

   SdpCodec::SdpCodecTypes assignAudioSDPnumber(const UtlString& mimeSubtypeInLowerCase); ///< mimeSubtype SHOULD BE in lower case   

     /// Copy constructor (not supported)
   MpCodecFactory(const MpCodecFactory& rMpCodecFactory);

     /// Assignment operator (not supported)
   MpCodecFactory& operator=(const MpCodecFactory& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpCodecFactory_h_
