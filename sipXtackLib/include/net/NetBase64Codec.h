//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _NetBase64Codec_h_
#define _NetBase64Codec_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <utl/UtlString.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class NetBase64Codec
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    static const char* base64Codes;

/* ============================ CREATORS ================================== */

   virtual
   ~NetBase64Codec();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   static void encode(int dataSize, const char data[],
                int& encodedDataSize, char encodedData[]);

   static void encode(int dataSize, const char data[], UtlString& encodedData);

        static int encodedSize(int dataSize);

        inline static char decodeChar(const char encoded);

        static void decode(int encodedDataSize, const char encodedData[],
                int& dataSize, char data[]);

        static int decodedSize(int encodedDataSize, const char encodedData[]);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   NetBase64Codec();
     //:Default constructor (disabled)

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   NetBase64Codec(const NetBase64Codec& rNetBase64Codec);
     //:Copy constructor (disabled)

   NetBase64Codec& operator=(const NetBase64Codec& rhs);
     //:Assignment operator (disabled)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _NetBase64Codec_h_
