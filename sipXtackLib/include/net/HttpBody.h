//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
// Author: Dan Petrie (dpetrie AT SIPez DOT com)


#ifndef _HttpBody_h_
#define _HttpBody_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <utl/UtlString.h>


// DEFINES
#define CONTENT_TYPE_TEXT_PLAIN "text/plain"
#define CONTENT_TYPE_TEXT_HTML "text/html"
#define CONTENT_SMIME_PKCS7 "application/pkcs7-mime"
#define CONTENT_TYPE_MULTIPART "multipart/"

#define MULTIPART_BOUNDARY_PARAMETER "boundary"

#define MAX_HTTP_BODY_PARTS 20

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MimeBodyPart;

//! class to contain an HTTP body
/*! This is the base class and container for all HTTP (SIP, etc.)
 * message bodies.  This includes multipart MIME bodies, single
 * part MIME and specific MIME types.  The HttpBody is essentially
 * a container for a blob.
 */
class HttpBody : public UtlString
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   HttpBody(const char* bytes = NULL, 
            int length = -1, 
            const char* contentType = NULL);
   //: Construct an HttpBody from a bunch of bytes

   HttpBody(const HttpBody& rHttpBody);
     //:Copy constructor

   virtual
   ~HttpBody();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   HttpBody& operator=(const HttpBody& rhs);
   //:Assignment operator

   //! Copy the source HttpBody using the correct derived class constructor
   static HttpBody* copyBody(const HttpBody& sourceBody);

   //! Pseudo body factory
   static HttpBody* createBody(const char* bodyBytes,
                               int bodyBytesLength,
                               const char* contentType,
                               const char* contentEncoding);

/* ============================ ACCESSORS ================================= */

   virtual int getLength() const;

   virtual void getBytes(const char** bytes, int* length) const;
   virtual void getBytes(UtlString* bytes, int* length) const;
   // Note: for conveniece bytes is null terminated
   // However depending upon the content type, the body may
   // contain more than one null character.

   UtlBoolean getMultipartBytes(int partIndex, 
       const char** bytes, int* length) const;

   const MimeBodyPart* getMultipart(int partIndex) const;

   const char*  getContentType() const;

/* ============================ INQUIRY =================================== */

   UtlBoolean isMultipart() const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   int bodyLength;
   UtlString mBody;
   UtlString  mMultipartBoundary;
   MimeBodyPart* mpBodyParts[MAX_HTTP_BODY_PARTS];

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _HttpBody_h_
