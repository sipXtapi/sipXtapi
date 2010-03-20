//  
// Copyright (C) 2006-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
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
#include "mp/MpEncoderBase.h"
#include "mp/MpDecoderBase.h"
#include "mp/codecs/PlgDefsV1.h"
#include "mp/MpPlgStaffV1.h"
#include "mp/MpMisc.h"
#include "sdp/SdpCodecList.h"
#include "utl/UtlHashBag.h"
#include "utl/UtlHashBagIterator.h"
#include "os/OsSharedLibMgr.h"
#include "os/OsStatus.h"
#include "os/OsBSem.h"

// DEFINES
/// PLUGIN_FILTER is a standard file filter for codec plugins.
#ifdef __pingtel_on_posix__ // [
#  define CODEC_PLUGINS_FILTER "^codec_.*\\.so$"
#elif defined(WIN32) // __pingtel_on_posix__ ] [
#  define CODEC_PLUGINS_FILTER "^codec_.*\\.dll$"
#else // WIN32 ] [
#  error Unknown platform! Please specify correct codec plugins file filter.
#endif // ]


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
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{
 
     /// Get/create singleton factory.
   static
   MpCodecFactory* getMpCodecFactory(void);
     /**<
     *  Return a pointer to the MpCodecFactory singleton object, creating 
     *  it if necessary
     */

     /// Deinitialize all static codecs and freeing handle. Should be called only mpShutdown()
   static
   void freeSingletonHandle();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Load specified codec plugin.
   OsStatus loadDynCodec(const char* name);

     /// Load all codec plugins within specified path and filter.
   OsStatus loadAllDynCodecs(const char* path, const char* regexFilter);
     /**<
     *  Load all libs in given plugins directory.
     *
     *  @retval OS_SUCCESS - if at least one codec plugin was found. Note, that
     *          OS_SUCCESS is returned even if loading of found plugins failed.
     *  @retval OS_FAILED - if no codec plugins were found.
     */

     /// Add static codec to factory.
   static
   MpCodecCallInfoV1* addStaticCodec(MpCodecCallInfoV1* sStaticCode);   

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Returns a new instance of a decoder of the indicated type
   OsStatus createDecoder(const UtlString &mime,
                          const UtlString &fmtp,
                          int sampleRate,
                          int numChannels,
                          int payloadType,
                          MpDecoderBase*& rpDecoder) const;
     /**<
     *  @param[in]  mime - codec MIME-subtype
     *  @param[in]  fmtp - codec-specific string in format of SDP "fmtp" parameter
     *  @param[in]  sampleRate - requested codec's sample rate
     *  @param[in]  numChannels - requested codec's number of channels
     *  @param[in]  payloadType - RTP payload type to be associated with this decoder
     *  @param[out] rpDecoder - Reference to a pointer to the new decoder object
     */

     /// Returns a new instance of an encoder of the indicated type
   OsStatus createEncoder(const UtlString &mime,
                          const UtlString &fmtp,
                          int sampleRate,
                          int numChannels,
                          int payloadType,
                          MpEncoderBase*& rpEncoder) const;
     /**<
     *  @param[in]  mime - codec MIME-subtype
     *  @param[in]  fmtp - codec-specific string in format of SDP "fmtp" parameter
     *  @param[in]  sampleRate - requested codec's sample rate
     *  @param[in]  numChannels - requested codec's number of channels
     *  @param[in]  payloadType - RTP payload type to be associated with this encoder
     *  @param[out] rpEncoder - Reference to a pointer to the new encoder object
     */

     /// Get list of all codecs' info.
   void getCodecInfoArray(unsigned &count,
                          const MppCodecInfoV1_1 **&codecInfoArray) const;
     /**<
     *  @param[out] count - number of elements returned in \p codecInfoArray array.
     *  @param[out] codecInfoArray - array of pointers to codecs' info.
     */

     /// Add all supported codecs to SDP descriptions list.
   void addCodecsToList(SdpCodecList &codecList) const;
     /**<
     *  @note Not really all codecs are added to list. Only codecs, known
     *        by sipXsdpLib are added.
     */

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

     /// Destructor
   ~MpCodecFactory();

     /// Search codec by given MIME-subtype, sample rate and channels number.
   MpCodecSubInfo* searchByMIME(const UtlString& mime,
                                int sampleRate,
                                int numChannels) const;

     /// Deinitialize all dynamic codecs.  Should be called only from mpShutdown() 
   void freeAllLoadedLibsAndCodec();

     /// Initialize all static codecs.
   void initializeStaticCodecs();
     /**<
     *  @note This should called exactly once at startup. At current code it is
     *        called from mpStartUp().
     */

     /// Freeing internal data of static codecs.  Should be called only from global .dtor
   static
   void freeStaticCodecs();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   UtlHashBag mCodecsInfo;                     ///< List of all known and workable codecs.
   mutable UtlBoolean mCodecInfoCacheValid;    ///< Should we rebuild MIME-subtypes cache?
   mutable unsigned   mCachedCodecInfoNum;     ///< Number of elements in mpMimeTypesCache.
   mutable const MppCodecInfoV1_1** mpCodecInfoCache; ///< Cached array of MIME-subtypes of loaded codecs.
                                               ///< This is used as return value of getMimeTypes().

   // Static data members used to enforce Singleton behavior
   static MpCodecFactory* spInstance; ///< Pointer to the singleton instance.
   static OsBSem sLock;               ///< Semaphore used to synchronize singleton construction
                                      ///< and destruction.
   static MpCodecCallInfoV1* sStaticCodecsV1; ///< List of all static codecs.
                                      ///< Filled by global magic .ctor.

     /// Add new codec wrapper to codec list.
   OsStatus addCodecWrapperV1(MpCodecCallInfoV1* wrapper);

     /// Update cached array of MIME-types of loaded codecs.
   void updateCodecInfoCache() const;

     /// Copy constructor (not supported)
   MpCodecFactory(const MpCodecFactory& rMpCodecFactory);

     /// Assignment operator (not supported)
   MpCodecFactory& operator=(const MpCodecFactory& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpCodecFactory_h_
