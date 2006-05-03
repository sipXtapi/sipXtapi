//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <utl/UtlDListIterator.h>
#include <net/MimeBodyPart.h>
#include <net/HttpMessage.h>
#include <net/NameValuePair.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MimeBodyPart::MimeBodyPart(const HttpBody* parent, int parentBodyStartIndex, int rawBodyLength)
{
   mpParentBody = parent;
   mParentBodyRawStartIndex = parentBodyStartIndex;
   mRawBodyLength = rawBodyLength;
   mParentBodyStartIndex = parentBodyStartIndex;
   mBodyLength = 0;

   if(rawBodyLength > 0 && parent)
   {
       const char* parentBodyBytes;
       const char* bodyBytes;
       int parentBodyLength;
       parent->getBytes(&parentBodyBytes, &parentBodyLength);
       bodyBytes = parentBodyBytes + parentBodyStartIndex;
       if(parentBodyLength >= parentBodyStartIndex + rawBodyLength)
       {
            
           int parsedBytes = HttpMessage::parseHeaders(bodyBytes, rawBodyLength,
                    mNameValues);

           // search the part headers for a Content-Type
           NameValuePair* partType;
           UtlDListIterator partHeaders(mNameValues);
           for( partType = static_cast<NameValuePair*>(partHeaders());
                partType && partType->compareTo(HTTP_CONTENT_TYPE_FIELD, UtlString::ignoreCase);
                partType = static_cast<NameValuePair*>(partHeaders())
               )
           {
           }
           if (partType)
           {
              // the content-type of the part is stored in the parent UtlString
              append(partType->getValue());
           }
            mParentBodyStartIndex = parentBodyStartIndex + parsedBytes;
            mBodyLength = mRawBodyLength - parsedBytes;
       }
   }

}

// Copy constructor
MimeBodyPart::MimeBodyPart(const MimeBodyPart& rMimeBodyPart) :
HttpBody(rMimeBodyPart)
{
    UtlDListIterator iterator((UtlDList&)rMimeBodyPart.mNameValues);
    NameValuePair* nvp;
    while((nvp = (NameValuePair*)iterator()))
    {
        mNameValues.append(new NameValuePair(nvp->data(), nvp->getValue()));
    }
    mpParentBody = rMimeBodyPart.mpParentBody;
    mParentBodyRawStartIndex = rMimeBodyPart.mParentBodyRawStartIndex;
    mRawBodyLength = rMimeBodyPart.mRawBodyLength;
    mParentBodyStartIndex = rMimeBodyPart.mParentBodyStartIndex;
    mBodyLength = rMimeBodyPart.mBodyLength;
}

// Destructor
MimeBodyPart::~MimeBodyPart()
{
    mNameValues.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
MimeBodyPart& 
MimeBodyPart::operator=(const MimeBodyPart& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

    mNameValues.destroyAll();
    UtlDListIterator iterator((UtlDList&)rhs.mNameValues);
    NameValuePair* nvp;
    while((nvp = (NameValuePair*)iterator()))
    {
        mNameValues.append(new NameValuePair(nvp->data(), nvp->getValue()));
    }
    mpParentBody = rhs.mpParentBody;
    mParentBodyRawStartIndex = rhs.mParentBodyRawStartIndex;
    mRawBodyLength = rhs.mRawBodyLength;
    mParentBodyStartIndex = rhs.mParentBodyStartIndex;
    mBodyLength = rhs.mBodyLength;

   return *this;
}

/* ============================ ACCESSORS ================================= */
int MimeBodyPart::getLength() const
{
   return(mBodyLength);
}

void MimeBodyPart::getBytes(const char** bytes, int* length) const
{
    *bytes = NULL;
    if(mpParentBody)
    {
       const char* parentBodyBytes;
       int parentBodyLength;
       mpParentBody->getBytes(&parentBodyBytes, &parentBodyLength);
       if(mParentBodyStartIndex + mBodyLength <= parentBodyLength)
       {
           *bytes = parentBodyBytes + mParentBodyStartIndex;
       }
    }
    *length = mBodyLength;
}

UtlBoolean MimeBodyPart::getPartHeaderValue(const char* headerName, UtlString& headerValue) const
{
    headerValue.remove(0);
    NameValuePair matchName(headerName);
    NameValuePair* nvp = (NameValuePair*) mNameValues.find(&matchName);
    if(nvp)
    {
        headerValue.append(nvp->getValue());
    }
    return(nvp != NULL);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

