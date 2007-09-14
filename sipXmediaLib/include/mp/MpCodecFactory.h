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
   OsStatus createDecoder(const UtlString &mime,
                          const UtlString &fmtp,
                          int payloadType,
                          MpDecoderBase*& rpDecoder);
     /**<
     *  @param[in]  mime - codec MIME-subtype
     *  @param[in]  fmtp - codec-specific string in format of SDP "fmtp" parameter
     *  @param[in]  payloadType - RTP payload type to be associated with this decoder
     *  @param[out] rpDecoder - Reference to a pointer to the new decoder object
     */

     /// Returns a new instance of an encoder of the indicated type
   OsStatus createEncoder(const UtlString &mime,
                          const UtlString &fmtp,
                          int payloadType,
                          MpEncoderBase*& rpEncoder);
     /**<
     *  @param[in]  mime - codec MIME-subtype
     *  @param[in]  fmtp - codec-specific string in format of SDP "fmtp" parameter
     *  @param[in]  payloadType - RTP payload type to be associated with this encoder
     *  @param[out] rpEncoder - Reference to a pointer to the new encoder object
     */

     /// Load specified codec plugin.
   OsStatus loadDynCodec(const char* name);

     /// Load all codec plugins within specified path and filter.
   OsStatus loadAllDynCodecs(const char* path, const char* regexFilter);
     /**<
     *  Useful to load all libs in plugins directory.
     */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   SdpCodec::SdpCodecTypes* getAllCodecTypes(unsigned& count);
   const char** getAllCodecModes(SdpCodec::SdpCodecTypes codecId, unsigned& count);

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

     /// Search codec by given MIME-subtype
   MpCodecSubInfo* searchByMIME(const UtlString& mime) const;

public:
   static MpCodecCallInfoV1* addStaticCodec(MpCodecCallInfoV1* sStaticCode);   

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

   UtlBoolean fCacheListMustUpdate; ///< Flag points that cached array must be rebuilt in the next call 
   UtlSList mCodecsInfo; ///< list of all known and workable codecs

   // Static data members used to enforce Singleton behavior
   static MpCodecFactory* spInstance; ///< Pointer to the singleton instance.
   static OsBSem sLock;               ///< Semaphore used to synchronize singleton construction
                                      ///< and destruction.
   static MpCodecCallInfoV1* sStaticCodecsV1; ///< List of all static codecs.
                                      ///< Filled by global magic .ctor.

     /// Add new codec wrapper to codec list.
   OsStatus addCodecWrapperV1(MpCodecCallInfoV1* wrapper);

     /// Copy constructor (not supported)
   MpCodecFactory(const MpCodecFactory& rMpCodecFactory);

     /// Assignment operator (not supported)
   MpCodecFactory& operator=(const MpCodecFactory& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpCodecFactory_h_
