//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MimeBodyPart_h_
#define _MimeBodyPart_h_

// SYSTEM INCLUDES
//#include <...>
#include "utl/UtlDList.h"

// APPLICATION INCLUDES
#include <net/HttpBody.h>


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:One part of a multipart Mime body
// This is a child part of a multipart MIME body
class MimeBodyPart : public HttpBody
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   MimeBodyPart(const HttpBody* parent = NULL, int parentBodyStartIndex = 0, int rawBodyLength = 0);
     //:Default constructor

   MimeBodyPart(const MimeBodyPart& rMimeBodyPart);
     //:Copy constructor

   virtual
   ~MimeBodyPart();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   MimeBodyPart& operator=(const MimeBodyPart& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */
   virtual int getLength() const;

   virtual void getBytes(const char** bytes, int* length) const;

   UtlBoolean getPartHeaderValue(const char* headerName, UtlString& headerValue) const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UtlDList mNameValues;
    const HttpBody* mpParentBody;
    int mParentBodyRawStartIndex;
    int mRawBodyLength;
    int mParentBodyStartIndex;
    int mBodyLength;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MimeBodyPart_h_
