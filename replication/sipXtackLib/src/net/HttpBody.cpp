// 
// Copyright (C) 2005 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004 SIPfoundry Inc.
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

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

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

        if (contentType)
        {
                append(contentType);
        NameValueTokenizer::frontBackTrim(this, " \t");
        //osPrintf("Content type: \"%s\"\n", mBodyContentType.data());
        int boundaryIndex = index(MULTIPART_BOUNDARY_PARAMETER,
            0, UtlString::ignoreCase);

        if(boundaryIndex >=0 &&
            index(CONTENT_TYPE_MULTIPART,
            0, UtlString::ignoreCase) == 0)
        {
            boundaryIndex += strlen(MULTIPART_BOUNDARY_PARAMETER);
            //osPrintf("Boundary start:=>%s\n",
            //    (mBodyContentType.data())[boundaryIndex]);

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
                //osPrintf("HttpBody: boundary=\"%s\"\n", mMultipartBoundary.data());
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
                    //UtlString contentType;
                    //UtlString name;
                    //UtlString value;
                    const char* partBytes;
                    const char* parentBodyBytes;
                    int partLength;
                    int parentBodyLength;
                    getBytes(&parentBodyBytes, &parentBodyLength);
                    getMultipartBytes(partIndex, &partBytes, &partLength);
                    //osPrintf("Body part 1 length: %d\n", firstPart.length());
                    //osPrintf("++++ Multipart Body #1 ++++\n%s\n++++ End Multipart #1 ++++\n",
                    //    firstPart.data());
                    if(partLength <= 0) break;

                    // Parse throught the header to the MIME part
                    // The first blank line is the begining of the part body
                    /*NameValueTokenizer parser(partBytes, partLength);
                    do
                    {
                        parser.getNextPair(HTTP_NAME_VALUE_DELIMITER,
                                            &name, & value);
                        if(name.compareTo(HTTP_CONTENT_TYPE_FIELD) == 0)
                        {
                            contentType = name;
                        }
                    }
                    while(!name.isNull());*/

                    // This is a bit of a temporary kludge
                    //Prepend a HTTP header to make it look like a HTTP message
                    //partBytes.insert(0, "GET / HTTP/1.0\n");
                    //HttpMessage firstPartMessage(partBytes.data(), partBytes.length());
                    //const HttpBody* partFileBody = firstPartMessage.getBody();
                    //int bytesLeft = parser.getProcessedIndex() - partLength;

                                if (partLength > 0)
                                        mpBodyParts[partIndex] = new MimeBodyPart(this, partBytes - parentBodyBytes,
                                                                partLength);
                                else
                                        mpBodyParts[partIndex] = NULL;
                }
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

// Copy constructor
HttpBody::HttpBody(const HttpBody& rHttpBody)
{
    mClassType = HTTP_BODY_CLASS;

    mBody = rHttpBody.mBody;

    bodyLength = rHttpBody.bodyLength;

    // Set the content type
    append(rHttpBody);

    mMultipartBoundary = rHttpBody.mMultipartBoundary;

    for(int partIndex = 0; partIndex < MAX_HTTP_BODY_PARTS; partIndex++)
    {
        if (rHttpBody.mpBodyParts[partIndex])
        {
            mpBodyParts[partIndex] = new MimeBodyPart(*(rHttpBody.mpBodyParts[partIndex]));
        }
        else
        {
            mpBodyParts[partIndex] = NULL;
        }
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

    for(int partIndex = 0; partIndex < MAX_HTTP_BODY_PARTS; partIndex++)
    {
       if(mpBodyParts[partIndex]) delete mpBodyParts[partIndex];
           if (rhs.mpBodyParts[partIndex])
                        mpBodyParts[partIndex] = new MimeBodyPart(*(rhs.mpBodyParts[partIndex]));
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

/* ============================ ACCESSORS ================================= */
int HttpBody::getLength() const
{
        return(bodyLength);
}

void HttpBody::getBytes(const char** bytes, int* length) const
{
        *bytes = mBody.data();
        *length = bodyLength;
}

void HttpBody::getBytes(UtlString* bytes, int* length) const
{
    bytes->remove(0);
    //bytes->append(mBody);
    //*length = bodyLength;
    const char* bytePtr;
    getBytes(&bytePtr, length);
    if(*length > 0)
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

HttpBody::BodyClassTypes HttpBody::getClassType() const
{
    return(mClassType);
}

const char* HttpBody::getContentType() const
{
        return(data());
}

UtlBoolean HttpBody::getMultipartBytes(int partIndex, const char** bytes, int* length) const
{


    UtlBoolean partFound = FALSE;
    if(!mMultipartBoundary.isNull())
    {
        int byteIndex = -1;
        int partNum = -1;
        int partStartIndex = -1;
        int partEndIndex = -1;
        do
        {
            byteIndex = mBody.index(mMultipartBoundary.data(), byteIndex + 1);
            if(byteIndex >= 0)
            {
                partNum++;
                if(partNum == partIndex)
                {
                    partStartIndex = byteIndex + mMultipartBoundary.length();
                    if((mBody.data())[partStartIndex] == '\r') partStartIndex++;
                    if((mBody.data())[partStartIndex] == '\n') partStartIndex++;
                }
                else if(partNum == partIndex + 1)
                {
                    partEndIndex = byteIndex - 3;
                    //osPrintf("Part End Index: %d\n", partEndIndex);
                    //osPrintf("End of file: %c %d\n", mBody.data()[partEndIndex],
                    //    (int) ((mBody.data())[partEndIndex]));
                    if(((mBody.data())[partEndIndex]) == '\n') partEndIndex--;
                    //osPrintf("End of file: %c %d\n", mBody.data()[partEndIndex],
                    //    (int) ((mBody.data())[partEndIndex]));
                    if(((mBody.data())[partEndIndex]) == '\r') partEndIndex--;
                    //osPrintf("End of file: %c %d\n", mBody.data()[partEndIndex],
                    //    (int) ((mBody.data())[partEndIndex]));
                }
            }
        }
        while(partNum <= partIndex && byteIndex >= 0);
        if(partStartIndex >= 0 && partEndIndex > 0)
        {
            *bytes = &(mBody.data()[partStartIndex]);
            *length = partEndIndex - partStartIndex + 1;
            partFound = TRUE;
        }
        else
        {
            *bytes = NULL;
            *length = 0;
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

/* ============================ INQUIRY =================================== */
UtlBoolean HttpBody::isMultipart() const
{
    return(!mMultipartBoundary.isNull());
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
