//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _SdpCodecFactory_h_
#define _SdpCodecFactory_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "utl/UtlDList.h"

#include <os/OsBSem.h>
#include <os/OsRWMutex.h>

#include <sdp/SdpCodec.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Factory and container for all supported codec types

// Class detailed description which may extend to multiple lines
class SdpCodecList
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SdpCodecList(int numCodecs = 0,
               SdpCodec* codecArray[] = NULL);
     //:Default constructor

   SdpCodecList(const SdpCodecList& rSdpCodecFactory);
     //:Copy constructor

   SdpCodecList& operator=(const SdpCodecList& rhs);
     //:Assignment operator

   virtual
   ~SdpCodecList();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   void addCodec(SdpCodec& newCodec);
   //: Add a new codec type to the list of known codecs

   void addCodecs(int numCodecs, SdpCodec* newCodecs[]);
   //: Add copies of the array of codecs

   void bindPayloadTypes();
   // Assign any unset payload type ids

   void copyPayloadType(SdpCodec& codec);
   //: If there is a matching codec in this factory, set its payload type to that of the given codec

   void copyPayloadTypes(int numCodecs, SdpCodec* codecArray[]);
   //: For all matching codecs, copy the payload type from the codecArray to the matching codec in this factory

   void clearCodecs(void);
   //: Discard all codecs

   int buildSdpCodecFactory(const UtlString &codecList);
   //: Function just called other buildSdpCodecFactory. Here for compatibility

   int buildSdpCodecFactory(int codecCount, SdpCodec::SdpCodecTypes codecTypes[]);
   //: Add the default set of codecs specified in list; returns 0 if OK.

   void setCodecCPULimit(int iLimit);
     //:Limits the advertised codec by CPU limit level.
     //!param (in) iLimit - The limit level for codecs.  A value of 
     //       SDP_CODEC_CPU_LOW indicates only low cpu intensity codecs and
     //       a value of SDP_CODEC_CPU_HIGH indicates either low or high
     //       cpu intensity.


/* ============================ ACCESSORS ================================= */

   const SdpCodec* getCodec(SdpCodec::SdpCodecTypes internalCodecId);
   //: Get a codec given an internal codec id

   const SdpCodec* getCodecByType(int payloadTypeId);
   //: Get a codec given the payload type id

   const SdpCodec* getCodec(const char* mimeType, 
                            const char* mimeSubType);
   //: Get a codec given the mime type and subtype

   int getCodecCount();
   //: Get the number of codecs

   int getCodecCount(const char* mimeType);
   //: Get the number of codecs by mime type

   void getCodecs(int& numCodecs,
                  SdpCodec**& codecArray);

   void getCodecs(int& numCodecs,
                  SdpCodec**& codecArray,
                  const char* mimeType);

   void getCodecs(int& numCodecs, 
                  SdpCodec**& codecArray,
                  const char* mimeType,
                  const char* subMimeType);
   //: Returns a copy of all the codecs

   void toString(UtlString& serializedFactory);
   //: String representation of factory and codecs

   int getCodecCPULimit();
     //:Gets the codec CPU limit level

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   void addCodecNoLock(const SdpCodec& newCodec);
   //: Add a new codec type to the list of known codecs

   UtlDList mCodecs;
   OsRWMutex mReadWriteMutex;
   int mCodecCPULimit;

};

class SdpDefaultCodecFactory
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   static
   SdpCodec getCodec(SdpCodec::SdpCodecTypes internalCodecId);
   //: Get a codec given an internal codec id

   static SdpCodec::SdpCodecTypes getCodecType(const char* pCodecName);
   //: Converts the readable text codec name into an enum defined in Sdpcodec.h

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SdpCodecFactory_h_
