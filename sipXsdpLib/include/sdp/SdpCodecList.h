//
// Copyright (C) 2007-2013 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _SdpCodecList_h_
#define _SdpCodecList_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <sdp/SdpCodec.h>
#include <utl/UtlDList.h>
#include <os/OsRWMutex.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// List of codecs, used for storing supported codecs and for codec negotiation.
class SdpCodecList
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Constructor.
   SdpCodecList(int numCodecs = 0,
                SdpCodec* codecArray[] = NULL);

     /// Copy constructor.
   SdpCodecList(const SdpCodecList& rSdpCodecFactory);

     /// Assignment operator.
   SdpCodecList& operator=(const SdpCodecList& rhs);

     /// Destructor.
   virtual
   ~SdpCodecList();

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

     /// Add a new codec type to the list of known codecs.
   void addCodec(const SdpCodec& newCodec);

     /// Add copies of the array of codecs.
   void addCodecs(int numCodecs, SdpCodec* newCodecs[]);

     /// Add specified codecs to the list.
   int addCodecs(const UtlString &codecList);
     /**<
     *  @returns Number of unknown codecs in the list. Zero for full success.
     */

     /// Add specified codecs to the list.
   int addCodecs(UtlSList& codecNameList);
     /**<
     *  @returns Number of unknown codecs in the list. Zero for full success.
     */
     /// Add codecs specified by array of codec token names and assign given payload ID
   int addCodecs(int numTokens, const char* codecTokens[], int payloadIds[]);

     /// Add specified codecs to the list.
   int addCodecs(int codecCount, SdpCodec::SdpCodecTypes codecTypes[]);
     /**<
     *  @returns Number of unknown codecs in the list. Zero for full success.
     */

     /// Assign any unset payload type ids.
   void bindPayloadTypes();

   /// @brief unbind any codec with the given dynamic payload type
   int unbindPayloadType(int payloadId);
   /*
    * @param payloadId - no op if not a dynamic payload ID (> SDP_CODEC_MAXIMUM_STATIC_CODEC), 
    *        codec payload ID to unbind (set to SDP_CODEC_UNKNOWN)
    *
    * @returns number of codecs unbound
    */

     /// @brief If there is a matching codec in this factory, set its
     /// payload type to that of the given codec.
   void copyPayloadType(const SdpCodec& codec);

     /// @brief For all matching codecs, copy the payload type from
     /// the codecArray to the matching codec in this factory.
   void copyPayloadTypes(int numCodecs, const SdpCodec* codecArray[]);

     /// @brief For all matching codecs, copy the payload type from
     /// the given codecList to the matching codec in this factory.
   void copyPayloadTypes(const SdpCodecList& codecList);

   /// @brief Remove any codecs in this list that are not equivalent to those in the given list
   void limitCodecs(const SdpCodecList& includeOnlyCodecList);

     /// Clear the list.
   void clearCodecs(void);

     /// Limits the advertised codec by CPU limit level.
   void setCodecCPULimit(int iLimit);
     /**<
     *  @param[in] iLimit - The limit level for codecs.  A value of
     *             SDP_CODEC_CPU_LOW indicates only low cpu intensity codecs and
     *             a value of SDP_CODEC_CPU_HIGH indicates either low or high
     *             cpu intensity.
     */

//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{

     /// Get a codec given an internal codec id.
   const SdpCodec* getCodec(SdpCodec::SdpCodecTypes internalCodecId) const;

     /// Get a codec given the payload type id.
   const SdpCodec* getCodecByType(int payloadTypeId, UtlBoolean shouldLock = TRUE) const;

     /// Get a codec given the MIME type, subtype and other parameters.
   const SdpCodec* getCodec(const char* MIMEType, 
                            const char* MIMESubType,
                            int sampleRate,
                            int numChannels,
                            const UtlString &fmtp) const;

     /// Get the number of codecs.
   int getCodecCount() const;

     /// Get the number of codecs by MIME type.
   int getCodecCount(const char* MIMEType) const;

     /// Get codecs from this list, taking into account maximum CPU cost.
   void getCodecs(int& numCodecs,
                  SdpCodec**& codecArray) const;

     /// @brief Get codecs from this list, taking into account maximum CPU cost
     /// and given MIME type.
   void getCodecs(int& numCodecs,
                  SdpCodec**& codecArray,
                  const char* MIMEType) const;

     /// Returns a copy of all the codecs.
   void getCodecs(int& numCodecs, 
                  SdpCodec**& codecArray,
                  const char* MIMEType,
                  const char* subMimeType) const;

    /// Frees up codecs and codec pointer array
    static void freeArray(int arraySize, SdpCodec**& codecArray);
    /* 
     *  getCodecs allocates a codec pointer array and codecs.  This frees them.
     */

     /// String representation of factory and codecs.
   void toString(UtlString& serializedFactory) const;

     /// Gets the codec CPU limit level.
   int getCodecCPULimit() const;

//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{

    /// Query if equivalent codec is in the list
    UtlBoolean containsCodec(const SdpCodec& codec, UtlBoolean exeact = FALSE) const;

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Add a new codec type to the list of known codecs (without locking).
   void addCodecNoLock(const SdpCodec& newCodec);

   UtlDList mCodecs;
   OsRWMutex mReadWriteMutex;
   int mCodecCPULimit;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SdpCodecList_h_
