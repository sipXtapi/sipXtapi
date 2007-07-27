// 
// Copyright (C) 2005-2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie (dpetrie AT SIPez DOT com)


// SYSTEM INCLUDES

#include <string.h>
#include <assert.h>

// APPLICATION INCLUDES
#include <net/HttpBody.h>
#include <net/SdpBody.h>
#include <net/PidfBody.h>
#include <net/SmimeBody.h>
#include <net/MimeBodyPart.h>
#include <net/SipDialogEvent.h>
#include <net/NameValueTokenizer.h>
#include <net/HttpMessage.h>
#include <os/OsSysLog.h>
#include <utl/UtlDList.h>
#include <utl/UtlDListIterator.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// The number of hex chars to use for boundary strings.
#define BOUNDARY_STRING_LENGTH 8
// Mask to extract the low (BOUNDARY_STRING_LENGTH*4) bits of an unsigned int.
#define BOUNDARY_COUNTER_MASK 0xFFFFFFFF

// STATIC VARIABLE INITIALIZATIONS

unsigned HttpBody::boundaryCounter = 0;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
HttpBody::HttpBody(const char* bytes, int length, const char* contentType) :
   bodyLength(0)
{
   mClassType = HTTP_BODY_CLASS;
   for(int partIndex = 0; partIndex < MAX_HTTP_BODY_PARTS; partIndex++)
   {
      mpBodyParts[partIndex] = NULL;
   }
   mBodyPartCount = 0;

   if (contentType)
   {
      append(contentType);
      NameValueTokenizer::frontBackTrim(this, " \t");
      #ifdef TEST_PRINT
         osPrintf("Content type: \"%s\"\n", contentType);
      #endif

      int boundaryIndex = index(MULTIPART_BOUNDARY_PARAMETER,
                                0, UtlString::ignoreCase);

      if(boundaryIndex >=0 &&
         index(CONTENT_TYPE_MULTIPART,
               0, UtlString::ignoreCase) == 0)
      {
         boundaryIndex += strlen(MULTIPART_BOUNDARY_PARAMETER);

         // Allow white space before =
         int fieldLength = this->length();
         while(boundaryIndex < fieldLength &&
               (data()[boundaryIndex] == ' ' ||
                data()[boundaryIndex] == '\t'))
            boundaryIndex++;

         if(data()[boundaryIndex] == '=')
         {
            mMultipartBoundary.append(&data()[boundaryIndex + 1]);
            NameValueTokenizer::frontTrim(&mMultipartBoundary, " \t");
            int whiteSpaceIndex = mMultipartBoundary.first(' ');
            if(whiteSpaceIndex > 0) mMultipartBoundary.remove(whiteSpaceIndex);
            whiteSpaceIndex = mMultipartBoundary.first('\t');
            if(whiteSpaceIndex > 0) mMultipartBoundary.remove(whiteSpaceIndex);
            whiteSpaceIndex = mMultipartBoundary.first('\"');
            if(whiteSpaceIndex == 0) mMultipartBoundary.remove(whiteSpaceIndex,1);
            whiteSpaceIndex = mMultipartBoundary.last('\"');
            if(whiteSpaceIndex > 0) mMultipartBoundary.remove(whiteSpaceIndex);
            #ifdef TEST_PRINT
               osPrintf("HttpBody: boundary=%s\n", mMultipartBoundary.data());
            #endif
         }
      }
   }

   if(bytes && length < 0) length = strlen(bytes);
   if(bytes && length > 0)
   {
      if (mBody.append(bytes, length).length() > 0) //append was successful
      {
         bodyLength = length;

         if(isMultipart())
         {
            for(int partIndex = 0; partIndex < MAX_HTTP_BODY_PARTS; partIndex++)
            {
               const char* partBytes;
               const char* parentBodyBytes;
               int partLength;
               int partStart;
               int parentBodyLength;
               getBytes(&parentBodyBytes, &parentBodyLength);
               getMultipartBytes(partIndex, &partBytes, &partLength, &partStart);
               if(partLength <= 0) break;

               if (partLength > 0)
               {
                  #ifdef TEST_PRINT
                     osPrintf("HttpBody constructor - MimeBodyPart %d added - partStart=%d - partLength=%d\n",
                              partIndex, partStart, partLength );
                  #endif
                  mpBodyParts[partIndex] = new MimeBodyPart(this, partStart, partLength);

                  // Save the number of body parts.
                  mBodyPartCount = partIndex + 1;
               }
               else
                  mpBodyParts[partIndex] = NULL;
            }
         }
         // Append failed
         else
         {
            bodyLength = mBody.length();
         }
      }
      // No content
      else
      {
         bodyLength = 0;
      }
   }
}

// Construct a multipart HttpBody with zero parts.
HttpBodyMultipart::HttpBodyMultipart(const char* contentType) :
   HttpBody(NULL, -1, contentType)
{
   for (int partIndex = 0; partIndex < MAX_HTTP_BODY_PARTS; partIndex++)
   {
      mpBodyParts[partIndex] = NULL;
   }

   // Create the boundary.
   nextBoundary(mMultipartBoundary);
   // Write it into the body.
   mBody = "--";
   mBody.append(mMultipartBoundary);
   mBody.append("--\r\n");
   // Add the boundary parameter to the type.
   append(";" MULTIPART_BOUNDARY_PARAMETER "=\"");
   append(mMultipartBoundary);
   append("\"");
   // No need to check validity of the boundary string, as there is no
   // body content for it to appear in.

   // Update bodyLength.
   bodyLength = mBody.length();
}

// Copy constructor
HttpBody::HttpBody(const HttpBody& rHttpBody) :
   UtlString(rHttpBody),
   bodyLength(rHttpBody.bodyLength),
   mBody(rHttpBody.mBody),
   mMultipartBoundary(rHttpBody.mMultipartBoundary),
   mBodyPartCount(rHttpBody.mBodyPartCount),
   mClassType(rHttpBody.mClassType)
{
   for (int partIndex = 0; partIndex < MAX_HTTP_BODY_PARTS; partIndex++)
   {
      mpBodyParts[partIndex] =
         rHttpBody.mpBodyParts[partIndex] ?
         new MimeBodyPart(this, rHttpBody.mpBodyParts[partIndex]->getRawStart(), rHttpBody.mpBodyParts[partIndex]->getRawLength() ) :
         NULL;
   }
}

// Destructor
HttpBody::~HttpBody()
{
   for(int partIndex = 0; partIndex < MAX_HTTP_BODY_PARTS; partIndex++)
   {
       if(mpBodyParts[partIndex])
       {
           delete mpBodyParts[partIndex];
           mpBodyParts[partIndex] = NULL;
       }

   }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
HttpBody&
HttpBody::operator=(const HttpBody& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

    mBody = rhs.mBody;

    bodyLength = rhs.bodyLength;

    // Set the content type
    remove(0);
    append(rhs);

    mMultipartBoundary = rhs.mMultipartBoundary;

    mBodyPartCount = rhs.mBodyPartCount;
    for(int partIndex = 0; partIndex < MAX_HTTP_BODY_PARTS; partIndex++)
    {
       if(mpBodyParts[partIndex]) delete mpBodyParts[partIndex];
           if (rhs.mpBodyParts[partIndex])
                   mpBodyParts[partIndex] = new MimeBodyPart(this, rhs.mpBodyParts[partIndex]->getRawStart(), 
                                                             rhs.mpBodyParts[partIndex]->getRawLength() );

           else
                   mpBodyParts[partIndex] = NULL;
   }
   return *this;
}

// Pseudo factory body copier
HttpBody* HttpBody::copyBody(const HttpBody& sourceBody)
{
    // TODO:  There should be a type member.  This is dangerous
    // as the class type may not line up with the content type
    HttpBody* body = NULL;
    BodyClassTypes classType = sourceBody.getClassType();

    switch(classType)
    {
    case SDP_BODY_CLASS:
        body = new SdpBody(((const SdpBody&)sourceBody));
        break;

    case SMIME_BODY_CLASS:
        body = new SmimeBody(((const SmimeBody&)(sourceBody)));
        break;

    case PIDF_BODY_CLASS:
        body = new PidfBody(((const PidfBody&)(sourceBody)));
        break;

    case DIALOG_EVENT_BODY_CLASS:
        body = new SipDialogEvent(sourceBody);
        break;

    case HTTP_BODY_CLASS:
        body = new HttpBody(sourceBody);
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "HttpMessage::HttpMessage HttpBody copy content-type: %s\n",
            bodyType ? bodyType : "<null>");
#endif
        break;

    default:
        OsSysLog::add(FAC_SIP, PRI_ERR,
            "HttpBody::copyBody unhandled body type: %d", classType);
        body = new HttpBody(sourceBody);
        break;
    }

    return(body);
}

// Pseudo factory
HttpBody* HttpBody::createBody(const char* bodyBytes,
                               int bodyLength,
                               const char* contentType,
                               const char* contentEncoding)
{
    HttpBody* body = NULL;

    UtlString contentTypeString;
    if(contentType)
    {
        contentTypeString.append(contentType);
        contentTypeString.toLower();
    }
    if(contentType && 
       strcmp(contentTypeString.data(), SDP_CONTENT_TYPE) == 0)
    {
        body = new SdpBody(bodyBytes, bodyLength);
    }
    else if(contentType && 
            strcmp(contentTypeString.data(), CONTENT_SMIME_PKCS7) == 0)
    {
        body = new SmimeBody(bodyBytes, bodyLength, contentEncoding);
    }
    else if(contentType &&
            strcmp(contentTypeString.data(), CONTENT_TYPE_PIDF) == 0)
    {
        body = new PidfBody(bodyBytes, bodyLength, contentEncoding);
    }
    //else if(contentTYpe &&
    //        strcmp(contentTypeString.data(), DIALOG_EVENT_CONTENT_TYPE) == 0)
    //{
    //    body = new SipDialogEvent(bodyBytes, bodyLength, contentEncoding);
    //}
    else if ((bodyLength  > 1) || 
             (bodyBytes[0] != '\n'))
    {
        body = new HttpBody(bodyBytes, bodyLength,
                            contentType);
    }

    return(body);
}

// Append a multipart body part to an existing multiparty body.
void HttpBody::appendBodyPart(const HttpBody& body,
                              const UtlDList& parameters)
{
   assert(isMultipart());
   
   // Construct a new MimeBodyPart for the new body part.
   MimeBodyPart* part = new MimeBodyPart(body, parameters);

   // Insert it as the last body part.
   int index;
   for (index = 0; index < MAX_HTTP_BODY_PARTS; index++)
   {
      if (!mpBodyParts[index])
      {
         mpBodyParts[index] = part;
         break;
      }
   }
   assert(index < MAX_HTTP_BODY_PARTS);

   // Turn the final boundary into an intermediate boundary.
   mBody.remove(mBody.length() - 4);
   mBody.append("\r\n");

   // Insert the headers.
   int rawPartStart = mBody.length();
   UtlDListIterator iterator(*part->getParameters());
   NameValuePair* nvp;
   while ((nvp = (NameValuePair*) iterator()))
   {
      mBody.append(nvp->data());
      mBody.append(": ");
      mBody.append(nvp->getValue());
      mBody.append("\r\n");
   }
   mBody.append("\r\n");

   // Insert the body.
   int partStart = mBody.length();
   const char* bytes;
   int length;
   body.getBytes(&bytes, &length);
   mBody.append(bytes, length);
   int partEnd = mBody.length();

   // Update bodyLength.
   bodyLength = mBody.length();

   // Determine if we have to change the boundary string.
   bool change_boundary_string =
      mBody.index(mMultipartBoundary, partStart) != UTL_NOT_FOUND;

   // Add the final boundary.
   mBody.append("\r\n--");
   mBody.append(mMultipartBoundary);
   mBody.append("--\r\n");

   // Update the MimeBodyPart to know where it is contained in the HttpBody.
   part->attach(this,
                rawPartStart, partEnd - rawPartStart,
                partStart, partEnd - partStart);

   // If we have to change the boundary string.
   if (change_boundary_string)
   {
      // Find a new boundary string that isn't in the body.
      do {
         nextBoundary(mMultipartBoundary);
      } while (mBody.index(mMultipartBoundary) != UTL_NOT_FOUND);

      // Replace the old boundary string.
      for (int partIndex = 0; partIndex < MAX_HTTP_BODY_PARTS; partIndex++)
      {
         MimeBodyPart* part = mpBodyParts[partIndex];
         if (part)
         {
            // Replace the boundary string just before this part.
            mBody.replace(part->getRawStart() - (2 + BOUNDARY_STRING_LENGTH),
                          BOUNDARY_STRING_LENGTH,
                          mMultipartBoundary.data(),
                          BOUNDARY_STRING_LENGTH);
         }
      }

      // Replace the boundary string in the final boundary.
      mBody.replace(mBody.length() - (4 + BOUNDARY_STRING_LENGTH),
                    BOUNDARY_STRING_LENGTH,
                    mMultipartBoundary.data(),
                    BOUNDARY_STRING_LENGTH);

      // Replace the boundary string in the Content-Type.
      size_t loc = this->index(";" MULTIPART_BOUNDARY_PARAMETER "=\"");
      this->replace(loc + sizeof (";" MULTIPART_BOUNDARY_PARAMETER "=\"") - 1,
                    BOUNDARY_STRING_LENGTH,
                    mMultipartBoundary.data(),
                    BOUNDARY_STRING_LENGTH);
   }
}

/* ============================ ACCESSORS ================================= */

int HttpBody::getLength() const
{
   return bodyLength;
}

void HttpBody::getBytes(const char** bytes, int* length) const
{
   *bytes = mBody.data();
   *length = mBody.length();
}

void HttpBody::getBytes(UtlString* bytes, int* length) const
{
   bytes->remove(0);
   const char* bytePtr;
   getBytes(&bytePtr, length);
   if (*length > 0)
   {
      //hint to the string to change the capacity to the new length.
      //if this fails, we may not have enough ram to complete this operation
      unsigned int newLength = (*length);
      if (bytes->capacity(newLength) >= newLength)
      {
         bytes->append(bytePtr, *length);
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "HttpBody::getBytes allocation failure to reserve %d bytes", newLength);
      }
   }
}

const char* HttpBody::getBytes() const
{
   return mBody.data();
}

HttpBody::BodyClassTypes HttpBody::getClassType() const
{
    return(mClassType);
}

const char* HttpBody::getMultipartBoundary() const
{
   return mMultipartBoundary.data();
}

const char* HttpBody::getContentType() const
{
   return data();
}

UtlBoolean HttpBody::getMultipartBytes(int partIndex,
                                       const char** bytes,
                                       int* length, int* start) const
{
    #ifdef TEST_PRINT
        osPrintf("GetMultipartBytes: PartIndex = %d\n", partIndex);
    #endif

    UtlBoolean partFound = FALSE;
    if(!mMultipartBoundary.isNull())
    {
        #ifdef TEST_PRINT
            osPrintf("Multipart Boundary: %s\n", mMultipartBoundary.data() ); // JMJ
        #endif

        int byteIndex = -1;
        int partNum = -1;
        int partStartIndex = -1;
        int partEndIndex = -1;
        do
        {
            byteIndex = mBody.index(mMultipartBoundary.data(), byteIndex + 1);
            #ifdef TEST_PRINT
                osPrintf("ByteIndex: %d\n", byteIndex);
            #endif

            if(byteIndex >= 0)
            {
                partNum++;
                if(partNum == partIndex)
                {
                    #ifdef TEST_PRINT
                        osPrintf("Part Num: %d\n", partNum);
                    #endif
                    partStartIndex = byteIndex + mMultipartBoundary.length();
                    if((mBody.data())[partStartIndex] == '\r') partStartIndex++;
                    if((mBody.data())[partStartIndex] == '\n') partStartIndex++;
                    #ifdef TEST_PRINT
                        osPrintf("Part Start Index: %d\n", partStartIndex);
                    #endif
                }
                else if(partNum == partIndex + 1)
                {
                    partEndIndex = byteIndex - 3;
                    if(((mBody.data())[partEndIndex]) == '\n') partEndIndex--;
                    if(((mBody.data())[partEndIndex]) == '\r') partEndIndex--;
                    #ifdef TEST_PRINT
                        osPrintf("Part End Index: %d\n", partEndIndex);
                    #endif
                }
            }
        }
        while(partNum <= partIndex && byteIndex >= 0);
        if(partStartIndex >= 0 && partEndIndex > 0)
        {
            *bytes = &(mBody.data()[partStartIndex]);
            *length = partEndIndex - partStartIndex + 1;
            *start = partStartIndex;
            partFound = TRUE;
        }
        else
        {
            *bytes = NULL;
            *length = 0;
            *start = -1;
        }
    }
    return(partFound);
}

const MimeBodyPart* HttpBody::getMultipart(int index) const
{
    const MimeBodyPart* bodyPart = NULL;

    if(index >= 0 && index < MAX_HTTP_BODY_PARTS && isMultipart())
    {
        bodyPart = mpBodyParts[index];
    }
    return(bodyPart);
}

int HttpBody::getMultipartCount() const
{
   return mBodyPartCount;
}

/* ============================ INQUIRY =================================== */
UtlBoolean HttpBody::isMultipart() const
{
    return(!mMultipartBoundary.isNull());
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void HttpBody::nextBoundary(UtlString& boundary)
{
   boundaryCounter += 0x54637281; // This constant is arbitrary, but it must
                                  // be odd.
   char buffer[BOUNDARY_STRING_LENGTH + 1];
   // Need to trim boundary counter to the needed length, as
   // "unsigned" may be longer.
   sprintf(buffer, "%0*x",
           BOUNDARY_STRING_LENGTH, boundaryCounter & BOUNDARY_COUNTER_MASK);
   boundary = buffer;
}
