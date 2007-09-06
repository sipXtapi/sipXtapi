//
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
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
   void addCodec(SdpCodec& newCodec);

     /// Add copies of the array of codecs.
   void addCodecs(int numCodecs, SdpCodec* newCodecs[]);

     /// Assign any unset payload type ids.
   void bindPayloadTypes();

     /// @brief If there is a matching codec in this factory, set its
     /// payload type to that of the given codec.
   void copyPayloadType(SdpCodec& codec);

     /// @brief For all matching codecs, copy the payload type from
     /// the codecArray to the matching codec in this factory.
   void copyPayloadTypes(int numCodecs, SdpCodec* codecArray[]);

     /// Clear the list.
   void clearCodecs(void);

     /// Function just called other buildSdpCodecFactory. Here for compatibility.
   int buildSdpCodecFactory(const UtlString &codecList);

     /// Add the default set of codecs specified in list; returns 0 if OK.
   int buildSdpCodecFactory(int codecCount, SdpCodec::SdpCodecTypes codecTypes[]);

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
   const SdpCodec* getCodec(SdpCodec::SdpCodecTypes internalCodecId);

     /// Get a codec given the payload type id.
   const SdpCodec* getCodecByType(int payloadTypeId);

     /// Get a codec given the MIME type and subtype.
   const SdpCodec* getCodec(const char* MIMEType, 
                            const char* MIMESubType);

     /// Get the number of codecs.
   int getCodecCount();

     /// Get the number of codecs by MIME type.
   int getCodecCount(const char* MIMEType);

     /// Get codecs from this list, taking into account maximum CPU cost.
   void getCodecs(int& numCodecs,
                  SdpCodec**& codecArray);

     /// @brief Get codecs from this list, taking into account maximum CPU cost
     /// and given MIME type.
   void getCodecs(int& numCodecs,
                  SdpCodec**& codecArray,
                  const char* MIMEType);

     /// Returns a copy of all the codecs.
   void getCodecs(int& numCodecs, 
                  SdpCodec**& codecArray,
                  const char* MIMEType,
                  const char* subMimeType);

     /// String representation of factory and codecs.
   void toString(UtlString& serializedFactory);

     /// Gets the codec CPU limit level.
   int getCodecCPULimit();

//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{


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
