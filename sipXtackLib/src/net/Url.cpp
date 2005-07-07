// 
//
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

// APPLICATION INCLUDES
#include <utl/UtlDListIterator.h>

#include <utl/UtlRegex.h>
#include <net/Url.h>
#include <net/NameValueTokenizer.h>
#include <net/SipMessage.h>
#include <net/HttpRequestContext.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

#ifndef min
#define min(x,y) (((x) < (y)) ? (x) : (y))
#endif

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
Url::Url(const char* urlString, UtlBoolean isAddrSpec)
{
    mpUrlParameters = NULL;
    mpHeaderParameters = NULL;
    mpFieldParameters = NULL;
    mAngleBracketsIncluded = FALSE;
    mPasswordSet = FALSE;
    mHostPort = -1;


    // If this is a URI as opposed to a field,
    // there are implied angle brackets
    if(isAddrSpec)
    {
        UtlString tempUrlString(urlString ?  urlString : "");

        int index = tempUrlString.index("<");
        if(index < 0)
            tempUrlString.insert(0, '<');

        index = tempUrlString.index(">");
        if(index < 0)
            tempUrlString.append(">", 1);

        parseString(tempUrlString.data());

        mAngleBracketsIncluded = FALSE;
        mDisplayName.remove(0);
        removeFieldParameters();
    }
    else
    {
        parseString(urlString ?  urlString : "");
    }
}

// Copy constructor
Url::Url(const Url& rUrl)
{
    mpUrlParameters = NULL;
    mpHeaderParameters = NULL;
    mpFieldParameters = NULL;

    *this = rUrl;
    //UtlString thatUrl;
    //((Url&)rUrl).toString(thatUrl);
    //reset();
    // The lazy inefficient way to copy
    //parseString(thatUrl.data());

}

// Destructor
Url::~Url()
{
    removeParameters();
    if(mpUrlParameters)
    {
        delete mpUrlParameters;
        mpUrlParameters = NULL;
    }
    if(mpHeaderParameters)
    {
        delete mpHeaderParameters;
        mpHeaderParameters = NULL;
    }
    if(mpFieldParameters)
    {
        delete mpFieldParameters;
        mpFieldParameters = NULL;
    }
}

void Url::removeParameters()
{
    NameValuePair* pHash = NULL ;

    // Clear URL Params
    if(mpUrlParameters)
    {
        while ((pHash = (NameValuePair*) mpUrlParameters->get()))
        {
            delete pHash;
            pHash = NULL;
        }
    }

    // Clear Header Params
    if(mpHeaderParameters)
    {
        while ((pHash = (NameValuePair*) mpHeaderParameters->get()))
        {
            delete pHash;
            pHash = NULL;
        }
    }

    // Clear Field Params
    if(mpFieldParameters)
    {
        while ((pHash = (NameValuePair*) mpFieldParameters->get()))
        {
            delete pHash;
            pHash = NULL;
        }
    }
}

void Url::reset()
{
    removeParameters();
    mUrlType.remove(0);
    mDisplayName.remove(0);
    mUserId.remove(0);
    mPassword.remove(0);
    mPasswordSet = FALSE;
    mHostAddress.remove(0);
    mHostPort = -1;
    mPath.remove(0);
    mAngleBracketsIncluded = FALSE;
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
Url&
Url::operator=(const Url& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   //UtlString thatUrl;
   //((Url&)rhs).toString(thatUrl);
   reset();
   // The lazy inefficient way to copy
   //parseString(thatUrl.data());

   // Copy the members
    mUrlType = rhs.mUrlType;
    mDisplayName = rhs.mDisplayName;
    mUserId = rhs.mUserId;
    mPassword = rhs.mPassword;
    mPasswordSet = rhs.mPasswordSet;
    mHostAddress = rhs.mHostAddress;
    mHostPort = rhs.mHostPort;
    mPath = rhs.mPath;
    mAngleBracketsIncluded = rhs.mAngleBracketsIncluded;

    // Copy the URL parameters
    if(rhs.mpUrlParameters && rhs.mpUrlParameters->entries())
    {
        if(mpUrlParameters == NULL) mpUrlParameters =
            new UtlDList();

        UtlDListIterator urlParamIterator(*(rhs.mpUrlParameters));
        NameValuePair* urlParam = NULL;
        while ((urlParam = (NameValuePair*) urlParamIterator()))
        {
            mpUrlParameters->append(new NameValuePair(*urlParam));
        }
    }

    // Copy the header parameters
    if(rhs.mpHeaderParameters && rhs.mpHeaderParameters->entries())
    {
        if(mpHeaderParameters == NULL) mpHeaderParameters =
            new UtlDList();

        UtlDListIterator headerParamIterator(*(rhs.mpHeaderParameters));
        NameValuePair* headerParam = NULL;
        while ((headerParam = (NameValuePair*) headerParamIterator()))
        {
            mpHeaderParameters->append(new NameValuePair(*headerParam));
        }
    }

    // Copy the field parameters
    if(rhs.mpFieldParameters && rhs.mpFieldParameters->entries())
    {
        if(mpFieldParameters == NULL) mpFieldParameters =
            new UtlDList();

        UtlDListIterator fieldParamIterator(*(rhs.mpFieldParameters));
        NameValuePair* fieldParam = NULL;
        while ((fieldParam = (NameValuePair*) fieldParamIterator()))
        {
            mpFieldParameters->append(new NameValuePair(*fieldParam));
        }
    }
   return *this;
}

Url& Url::operator=(const char* urlString)
{
   reset();
   parseString(urlString);

   return *this;
}
/* ============================ ACCESSORS ================================= */


void Url::getUrlType(UtlString& urlProtocol) const
{
    urlProtocol = mUrlType;
}

void Url::setUrlType(const char* urlProtocol)
{
    if (urlProtocol)
        mUrlType = urlProtocol;
    else
        mUrlType.remove(0);
}

void Url::getDisplayName(UtlString& displayName) const
{
    displayName = mDisplayName;
    if (isDigitString(mDisplayName.data()))
        NameValueTokenizer::frontBackTrim(&displayName, "\"");
}

void Url::setDisplayName(const char* displayName)
{
    if (displayName)
        mDisplayName = displayName;
    else
        mDisplayName.remove(0);
}

void Url::getUserId(UtlString& userId) const
{
    userId = mUserId;
}

void Url::setUserId(const char* userId)
{
    if (userId)
        mUserId = userId;
    else
        mUserId.remove(0);
}

UtlBoolean Url::getPassword(UtlString& password) const
{
    password = mPassword;
    return(mPasswordSet);
}

void Url::setPassword(const char* password)
{
    if (password)
    {
        mPassword = password;
        mPasswordSet = TRUE;
    }
    else
    {
        mPassword.remove(0);
        mPasswordSet = FALSE;
    }
}

void Url::getHostAddress(UtlString& address) const
{
    address = mHostAddress;
}

void Url::setPath(const char* path)
{
    if (path)
        mPath = path;
    else
        mPath.remove(0);
}

UtlBoolean Url::getPath(UtlString& path, UtlBoolean getStyle)
{
    path = mPath;

    // If the desire is to have an HTTP GET style path with CGI variables
    if(getStyle)
    {
        if(mpHeaderParameters)
        {
            UtlDListIterator headerParamIterator(*mpHeaderParameters);
            NameValuePair* headerParam = NULL;
            UtlString headerParamValue ;
            UtlBoolean firstHeader = TRUE;

            while ((headerParam = (NameValuePair*) headerParamIterator()))
            {
                // Add separator for first header parameter
                if(firstHeader)
                {
                    // @JC Changed to not mutate *this
                    path.append("?", 1);
                    firstHeader = FALSE;
                }
                else
                {
                    // @JC Changed to not mutate *this
                    path.append("&", 1);
                }

                // @JC Changed to not mutate *this
                path.append(*headerParam);
                headerParamValue = headerParam->getValue();
                if(!headerParamValue.isNull())
                {
                    // @JC Changed to not mutate *this
                    path.append("=", 1);
                    HttpMessage::escape(headerParamValue);
                    // @JC Changed to not mutate *this
                    path.append(headerParamValue);
                }
            }
        } // endif mpHeaderParameters
    }

    return(!mPath.isNull());
}

void Url::setHostAddress(const char* address)
{
    if (address)
        mHostAddress = address;
    else
        mHostAddress.remove(0);
}

int Url::getHostPort() const
{
    return(mHostPort);
}

void Url::setHostPort(int port)
{
    mHostPort = port;
}

UtlBoolean Url::getUrlParameter(const char* name, UtlString& value, int index)
{
    int foundIndex = 0;
    UtlBoolean found = FALSE;
    value = "";
    if(mpUrlParameters)
    {
        UtlDListIterator urlParamIterator(*mpUrlParameters);
        NameValuePair* urlParam = NULL;

        UtlString paramName;

        while ((urlParam = (NameValuePair*) urlParamIterator()))
        {
            paramName = *urlParam;
            if(paramName.compareTo(name, UtlString::ignoreCase) == 0)
            {
                if(index == foundIndex)
                {
                    found = TRUE;
                    value = urlParam->getValue();
                    break;
                }
                foundIndex++;
            }
        }
    }
    return(found);
}

UtlBoolean Url::getUrlParameter(int urlIndex, UtlString& name, UtlString& value)
{
    NameValuePair* url = NULL;


    if (mpUrlParameters &&
        (((int)(mpUrlParameters->entries())) > urlIndex) &&
            (urlIndex >= 0))
        url = (NameValuePair*) mpUrlParameters->at(urlIndex);

    if(url)
    {
        name = *url;
        value = url->getValue();
    }

    return(NULL != url);
}

UtlBoolean Url::getUrlParameters(int iMaxReturn, UtlString* pNames, UtlString *pValues, int& iActualReturn)
{
    if(mpUrlParameters == NULL)
    {
        iActualReturn = 0;
    }

    // If the pNames or pValue is null, return false and set the actual return
    // to the actual number of items.
    else if ((pNames == NULL) || (pValues == NULL))
    {
        iActualReturn = mpUrlParameters->entries() ;
        return FALSE ;
    }
    else
    {
        iActualReturn = min(iMaxReturn, ((int)(mpUrlParameters->entries()))) ;

        for (int i=0; i<iActualReturn; i++)
        {
            NameValuePair* pair = (NameValuePair*) mpUrlParameters->at(i) ;
            pNames[i] = *pair;
            pValues[i] = pair->getValue() ;
        }
    }
    return (iActualReturn > 0) ;
}

void Url::setUrlParameter(const char* name, const char* value)
{
    NameValuePair* nv = new NameValuePair(name ? name : "",
        value ? value : "");

    if(mpUrlParameters == NULL) mpUrlParameters =
        new UtlDList();
    mpUrlParameters->append(nv);
}

UtlBoolean Url::getHeaderParameter(const char* name, UtlString& value, int index)
{
    int foundIndex = 0;
    UtlBoolean found = FALSE;
    value = "";
    if(mpHeaderParameters)
    {
        UtlDListIterator headerParamIterator(*mpHeaderParameters);
        NameValuePair* headerParam = NULL;

        UtlString paramName;

        while ((headerParam = (NameValuePair*) headerParamIterator()))
        {
            paramName = *headerParam;
            if(paramName.compareTo(name, UtlString::ignoreCase) == 0)
            {
                if(index == foundIndex)
                {
                    found = TRUE;
                    value = headerParam->getValue();
                    break;
                }
                foundIndex++;
            }
        }
    }
    return(found);
}

UtlBoolean Url::getHeaderParameters(int iMaxReturn, UtlString* pNames, UtlString *pValues, int& iActualReturn)
{
    if(mpHeaderParameters == NULL)
    {
        iActualReturn = 0;
    }

    // If the pValue is null, return false and set the actual return to the actual
    // number of items.
    else if (pValues == NULL || pNames == NULL)
    {
        iActualReturn = mpHeaderParameters->entries() ;
        return FALSE ;
    }
    else
    {
        iActualReturn = min(iMaxReturn, ((int)(mpHeaderParameters->entries()))) ;

        for (int i=0; i<iActualReturn; i++)
        {
            NameValuePair *pair = (NameValuePair*) mpHeaderParameters->at(i) ;
            pNames[i] = *pair;
            pValues[i] = pair->getValue() ;
        }
    }
    return (iActualReturn > 0) ;
}


void Url::removeUrlParameters()
{
    if(mpUrlParameters) mpUrlParameters->destroyAll();
}

void Url::removeUrlParameter(const char* name)
{
    if(mpUrlParameters)
    {
        NameValuePair nv(name ? name : "");

        UtlDListIterator iterator(*mpUrlParameters);

        UtlContainable* matchingParam;
        while((matchingParam = iterator.findNext(&nv)))
        {
           mpUrlParameters->destroy(matchingParam);
        }
    }

}



void Url::getUri(UtlString& urlString)
{

    if(mUrlType.isNull() ||
       mUrlType.compareTo("sip") == 0)
    {
        urlString = "sip:";
    }
    else if(mUrlType.compareTo("http") == 0)
    {
        urlString = "http://";
    }
    else if(mUrlType.compareTo("https") == 0)
    {
        urlString = "https://";
    }
    else if(mUrlType.compareTo("file") == 0)
    {
        urlString = "file://";
    }
    else
    {
        urlString = mUrlType;
        urlString.append(":", 1);
    }

    if(!mUserId.isNull())
    {
        urlString.append(mUserId);
        if(!mPassword.isNull() || mPasswordSet)
        {
            urlString.append(":", 1);
            urlString.append(mPassword);
        }
        urlString.append("@", 1);
    }

    urlString.append(mHostAddress);
    if(mHostPort > 0)
    {
        char portBuffer[20];
        sprintf(portBuffer, ":%d", mHostPort);
        urlString.append(portBuffer);
    }

    // Add the path
    if(!mPath.isNull())
    {
        urlString.append(mPath);
    }

    // Add the URL parameters
    if(mpUrlParameters)
    {
        UtlDListIterator urlParamIterator(*mpUrlParameters);
        NameValuePair* urlParam = NULL;
        UtlString urlParamValue;

        while ((urlParam = (NameValuePair*) urlParamIterator()))
        {
            urlString.append(";", 1);
            urlString.append(*urlParam);
            urlParamValue = urlParam->getValue();
            if(!urlParamValue.isNull())
            {
                urlString.append("=", 1);
                HttpMessage::escape(urlParamValue);
                urlString.append(urlParamValue);
            }
        }
    }

    // Add the header parameters
    if(mpHeaderParameters)
    {
        UtlDListIterator headerParamIterator(*mpHeaderParameters);
        NameValuePair* headerParam = NULL;
        UtlString headerParamValue;
        UtlBoolean firstHeader = TRUE;

        while ((headerParam = (NameValuePair*) headerParamIterator()))
        {
            // Add separator for first header parameter
            if(firstHeader)
            {
                urlString.append("?", 1);
                firstHeader = FALSE;
            }

            else
            {
                urlString.append("&", 1);
            }

            urlString.append(*headerParam);
            headerParamValue = headerParam->getValue();
            if(!headerParamValue.isNull())
            {
                urlString.append("=", 1);
                HttpMessage::escape(headerParamValue);
                urlString.append(headerParamValue);
            }
        }

    }

}

void Url::setHeaderParameter(const char* name, const char* value)
{
//    UtlString escapedValue(value ? value : "");
//    HttpMessage::escape(escapedValue);
    NameValuePair* nv = new NameValuePair(name ? name : "",
        value ? value : "");

    if(mpHeaderParameters == NULL) mpHeaderParameters =
        new UtlDList();

    mpHeaderParameters->append(nv);
}

UtlBoolean Url::getHeaderParameter(int headerIndex, UtlString& name, UtlString& value)
{
    NameValuePair* header = NULL;


    if (mpHeaderParameters &&
        (((int)(mpHeaderParameters->entries())) > headerIndex) &&
            (headerIndex >= 0))
        header = (NameValuePair*) mpHeaderParameters->at(headerIndex);

    if(header)
    {
        name = *header;
        value = header->getValue();
    }

    return(NULL != header);
}

void Url::removeHeaderParameters()
{
    if(mpHeaderParameters) mpHeaderParameters->destroyAll();
}

void Url::removeHeaderParameter(const char* name)
{
    if(mpHeaderParameters)
    {
        NameValuePair nv(name ? name : "");

        UtlDListIterator iterator(*mpHeaderParameters);

        UtlContainable* matchingParam;
        while((matchingParam=iterator.findNext(&nv)))
        {
           mpHeaderParameters->destroy(matchingParam);
        }
    }
}

UtlBoolean Url::getFieldParameter(const char* name, UtlString& value, int index) const
{
    int foundIndex = 0;
    UtlBoolean found = FALSE;
    value = "";
    if(mpFieldParameters)
    {
        UtlDListIterator fieldParamIterator(*mpFieldParameters);
        NameValuePair* fieldParam = NULL;

        UtlString paramName;

        while ((fieldParam = (NameValuePair*) fieldParamIterator()))
        {
            paramName = *fieldParam;
            if(paramName.compareTo(name, UtlString::ignoreCase) == 0)
            {
                if(index == foundIndex)
                {
                    found = TRUE;
                    value = fieldParam->getValue();
                    break;
                }
                foundIndex++;
            }
        }
    }
    return(found);
}

UtlBoolean Url::getFieldParameter(int fieldIndex, UtlString& name, UtlString& value)
{
    NameValuePair* field = NULL;


    if (mpFieldParameters &&
        ((int)(mpFieldParameters->entries())) > fieldIndex &&
            fieldIndex >= 0)
        field = (NameValuePair*) mpFieldParameters->at(fieldIndex);

    if(field)
    {
        name = *field;
        value = field->getValue();
    }

    return(NULL != field);
}

UtlBoolean Url::getFieldParameters(int iMaxReturn, UtlString* pNames, UtlString *pValues, int& iActualReturn)
{
    if(mpFieldParameters == NULL)
    {
        iActualReturn = 0;
    }

    // If the pValue is null, return false and set the actual return to the actual
    // number of items.
    else if (pNames == NULL || pValues == NULL)
    {
        iActualReturn = mpFieldParameters->entries() ;
        return FALSE ;
    }
    else
    {
        iActualReturn = min(iMaxReturn, ((int)(mpFieldParameters->entries()))) ;

        for (int i=0; i<iActualReturn; i++)
        {
            NameValuePair *pair = (NameValuePair*) mpFieldParameters->at(i) ;
            pNames[i] = *pair;
            pValues[i] = pair->getValue() ;

        }
    }
    return (iActualReturn > 0) ;
}


void Url::setFieldParameter(const char* name, const char* value)
{
    NameValuePair* nv = new NameValuePair(name ? name : "",
        value ? value : "");

    if(mpFieldParameters == NULL) mpFieldParameters = new UtlDList();
    mpFieldParameters->append(nv);
}

void Url::removeFieldParameters()
{
    if(mpFieldParameters) mpFieldParameters->destroyAll();
}

void Url::removeFieldParameter(const char* name)
{
    if(mpFieldParameters)
    {
        NameValuePair nv(name ? name : "");

        UtlDListIterator iterator(*mpFieldParameters);

        UtlContainable* matchingParam;
        while((matchingParam=iterator.findNext(&nv)))
        {
           mpFieldParameters->destroy(matchingParam);
        }
    }
}

void Url::includeAngleBrackets()
{
    mAngleBracketsIncluded = TRUE;
}

void Url::removeAngleBrackets()
{
    mAngleBracketsIncluded = FALSE;
}

UtlString Url::toString() const
{
    UtlString str;
    toString(str);
    return str;
}

void Url::toString(UtlString& urlString) const
{
   UtlBoolean isNameAddr = FALSE;

   urlString = mDisplayName;

   // If the display name is not a sequence of tokens
#  define SIP_TOKEN "[a-zA-Z0-9!%*_+`'~-]+"
   RegEx SipTokenSequence("^" SIP_TOKEN "([ \t]*" SIP_TOKEN ")*$");
   if (   ( 0 < urlString.length() )
       && ( ! SipTokenSequence.Search( urlString.data() ))
       )
   {
      // then quote it.
      if (urlString.index("\"") != 0)
      {
         urlString.insert(0, "\"") ;
         urlString.append("\"") ;
      }
   }

   // If this should be nameAddr as opposed to addrSpec
   // (i.e. do we need anglebrackets)
   if ( mAngleBracketsIncluded || !mDisplayName.isNull() ||
       (mpUrlParameters && mpUrlParameters->entries()) )
   {
       urlString.append("<", 1);
       isNameAddr = TRUE;
   }

   // Test for the SIP protocol
   if(mUrlType.isNull() ||
      mUrlType.compareTo("sip") == 0 )
   {
       urlString.append("sip:", 4);
   }
   else if( mUrlType.compareTo("http") == 0 )
   {
       urlString.append("http://", 7);
   }
   else if( mUrlType.compareTo("https") == 0 )
   {
       urlString.append("https://", 8);
   }

   else if(mUrlType.compareTo("file") == 0)
   {
       urlString.append("file://", 7);
   }

   else
   {
       urlString.append(mUrlType, mUrlType.length());
       urlString.append(":", 1);
   }

   if(!mUserId.isNull())
   {
       urlString.append(mUserId);
       if(!mPassword.isNull() || mPasswordSet)
       {
           urlString.append(":", 1);
           urlString.append(mPassword);
       }
       urlString.append("@", 1);
   }

   urlString.append(mHostAddress);
   if(mHostPort > 0)
   {
       char portBuffer[20];
       sprintf(portBuffer, ":%d", mHostPort );
       urlString.append(portBuffer);
   }

   // Add the path
   if(!mPath.isNull())
   {
       urlString.append(mPath);
   }

   // Add the URL parameters
   if( mpUrlParameters )
   {
       UtlDListIterator urlParamIterator(*mpUrlParameters);
       NameValuePair* urlParam = NULL;
       UtlString urlParamValue;

       while ((urlParam = (NameValuePair*) urlParamIterator()))
       {
           urlString.append(";", 1);
           urlString.append(*urlParam);
           urlParamValue = urlParam->getValue();
           if(!urlParamValue.isNull())
           {
               urlString.append("=", 1);
               HttpMessage::escape(urlParamValue);
               urlString.append(urlParamValue);
           }
       }
   }

   // Add the header parameters
   if(mpHeaderParameters)
   {
      UtlDListIterator headerParamIterator(*mpHeaderParameters);
      NameValuePair* headerParam = NULL;
      UtlString headerParamValue ;
      UtlBoolean firstHeader = TRUE;

      while ((headerParam = (NameValuePair*) headerParamIterator()))
      {
         // Add separator for first header parameter
         if(firstHeader)
         {
            urlString.append("?", 1);
            firstHeader = FALSE;
         }
         else
         {
            urlString.append("&", 1);
         }

         urlString.append(*headerParam);
         headerParamValue = headerParam->getValue();
         if(!headerParamValue.isNull())
         {
            urlString.append("=", 1);
            HttpMessage::escape(headerParamValue);
            urlString.append(headerParamValue);
         }
      }

   }

   // Add the terminating angle bracket
   if(isNameAddr)
   {
      urlString.append(">", 1);
   }

   // Add the field parameters
   if(mpFieldParameters)
   {
      UtlDListIterator fieldParamIterator(*mpFieldParameters);
      NameValuePair* fieldParam = NULL;
      UtlString fieldParamValue;

      while ((fieldParam = (NameValuePair*) fieldParamIterator()))
      {
         urlString.append(";", 1);
         urlString.append(*fieldParam);
         fieldParamValue = fieldParam->getValue();
         if(!fieldParamValue.isNull())
         {
            urlString.append("=", 1);
            HttpMessage::escape(fieldParamValue);
            urlString.append(fieldParamValue);
         }
      }

   }

}

void Url::dump()
{
    UtlString proto;
    getUrlType(proto);
    printf("Url type: \"%s\"\n", proto.data());

    UtlString disp;
    getDisplayName(disp);
    printf("DisplayName: \"%s\"\n", disp.data());

    UtlString user;
    getUserId(user);
    printf("UserId: \"%s\"\n", user.data());

    UtlString pwd;
    getPassword(pwd);
    printf("Password: \"%s\"\n", pwd.data());

    UtlString server;
    getHostAddress(server);
    printf("Address: \"%s\"\n", server.data());

    int port = getHostPort();
    printf("Port: %d\n", port);

    UtlString callId;
    getHeaderParameter("call-id", callId);
    printf("Call-Id: \"%s\"\n", callId.data());

    UtlString name;
    UtlString value;
    int index = 0;
    printf("\nHeader Parameters:\n");
    while(getHeaderParameter(index, name, value))
    {
        printf("\"%s\"=\"%s\"\n", name.data(), value.data());
        index++;
    }

    index = 0;
    printf("\nField Parameters:\n");
    while(getFieldParameter(index, name, value))
    {
        printf("\"%s\"=\"%s\"\n", name.data(), value.data());
        index++;
    }

    index = 0;
    printf("\nURL Parameters:\n");
    while(getUrlParameter(index, name, value))
    {
        printf("\"%s\"=\"%s\"\n", name.data(), value.data());
        index++;
    }
}

/* ============================ INQUIRY =================================== */

UtlBoolean Url::isDigitString(const char* dialedCharacters)
{
    // Must be digits or *
    RegEx allDigits("^[0-9*]+$");
    return allDigits.Search(dialedCharacters);
}


UtlBoolean Url::isIncludeAngleBracketsSet() const
{
    return mAngleBracketsIncluded ;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void Url::parseString(const char* urlString)
{
    // Display Name<addrSpec>urlParameters
    if(urlString && *urlString)
    {
        //UtlString tempUrl(urlString);

        //UtlString addrSpec;
        const char* fieldParameters = NULL;
        mPasswordSet = FALSE;
        int addrSpecBegin;
        int addrSpecEnd;
        const char* hostStart;
        const char* headerStart;

        // Pull out field parameters.  If we have angle brackets, we are
        // parsing the name_addr form of the URL.  We split the URL into
        // the display name, addr_spec, and field parameters.  If there
        // are no angle brackets, we treat the parameters as field
        // parameters UNLESS there is a header in which case parameters
        // before the header are URL parameters and parameters after the
        // header are field parameters.
        //
        // If the URL has no angle brackets and no header, it is impossible
        // to tell if parameters are field or Url parameters.  In this case,
        // they will be parsed as field parameters.

        const char* foundAngleBracket = strchr(urlString, '<');
        if(foundAngleBracket)
        {
            mAngleBracketsIncluded = TRUE;
            addrSpecBegin = foundAngleBracket - urlString + 1;

            // Back up over the white space
            while(foundAngleBracket > urlString &&
                  (*foundAngleBracket == '\t'  ||
                   *foundAngleBracket == '<'  ||
                   *foundAngleBracket == ' '))
            {
                foundAngleBracket--;
            }

            // If there is a display name
            if(foundAngleBracket > urlString)
            {
                // Find the first non-white space
                int beginDisplayName = strspn(urlString, SIP_SUBFIELD_SEPARATORS);

                mDisplayName.append(&(urlString[beginDisplayName]),
                    foundAngleBracket - &(urlString[beginDisplayName]) + 1);
            }

            // Find the end of the addrSpec which is either delimited by the
            // ">" charactore or the end  of string if not found
            addrSpecEnd = strcspn(&(urlString[addrSpecBegin]), ">");

            // Create the addrSpect string
            //addrSpec.append(&(urlString[addrSpecBegin]), addrSpecEnd);

            hostStart = strchr(&(urlString[addrSpecBegin]), '@');

            headerStart = strchr(hostStart ? hostStart : urlString,
                                       '?');

            fieldParameters = &(urlString[addrSpecBegin + addrSpecEnd]);

            // The field parameters start right after the ">"
            if(*fieldParameters == '>')
            {
                fieldParameters++;
            }

            // There was no ">", tolerate this.
            // There are no field parameters
            else if(*fieldParameters == '\0')
            {
                fieldParameters = NULL;
            }

            // Should not get here
            else
            {
                osPrintf("ERROR: Url::parseString unexpected begin of field parameters: %s\n",
                    fieldParameters);
                fieldParameters = NULL;
            }
        }

        // No angle brackets (therefore no display name)
        // addr_spec case
        else
        {
            mAngleBracketsIncluded = FALSE;
            addrSpecBegin = 0;
            addrSpecEnd = strlen(urlString);

            // It is now legal to have semicolons in the user name
            // So first we try to find the end of the user id
            // (begining of the host) if one exists

            hostStart = strchr(urlString, '@');

            // Then we look for the begining of the field parameters from there.

            headerStart = strchr(hostStart ? hostStart : urlString,
                                       '?');

            if(headerStart || hostStart)
            {
                fieldParameters =
                    strchr((headerStart ? headerStart : hostStart),
                           ';');


            }
            else
            {
                fieldParameters = strchr(urlString, ';');
            }

        }

        // If there is a an addrSpec pull out the url
        // parameters and the header values,
        // then parse them up
        UtlString coreUrl(&(urlString[addrSpecBegin]));

        // It is now legal to have semicolons in the user name
        // So first we try to find the end of the user id
        // (begining of the host) if one exists

        // Then we look for the begining of the url parameters
        // after the host start.
        const char* paramStart = strchr(hostStart ? hostStart : urlString, ';');

        // Get all the url parameters
        if(paramStart &&
            (fieldParameters == NULL || // make sure these are not field
            paramStart + 1 < fieldParameters)) // parameters
        {
            UtlString urlParameters(paramStart + 1);

            if(headerStart) urlParameters.remove(headerStart - paramStart - 1);
            else urlParameters.remove(&(urlString[addrSpecBegin + addrSpecEnd]) -
                                    (paramStart + 1));

            // Parse the url parameters
            if(!urlParameters.isNull())
            {
                if(mpUrlParameters == NULL) mpUrlParameters = new UtlDList();

                HttpRequestContext::parseCgiVariables(urlParameters.data(),
                                                      *mpUrlParameters, ";", "=",
                                                      FALSE);
            }
            //osPrintf("url param count: %d\n", mUrlParameters.entries());


        }
        else
        {
            paramStart = NULL;
        }

        // Get all of the headers values
        if(headerStart)
        {
            UtlString headerParameters(headerStart + 1);

            // Remove the ">" and anything after it (i.e. fieldParameters)
            if(fieldParameters == NULL ||
               &(urlString[addrSpecBegin + addrSpecEnd]) < fieldParameters)
            {
                headerParameters.remove(&(urlString[addrSpecBegin + addrSpecEnd]) -
                                        (headerStart + 1));
            }
            // No angle bracket, but there are field parameters
            else
            {
                headerParameters.remove(fieldParameters -
                                        (headerStart + 1));
            }

            // Parse the header parameters
            if(!headerParameters.isNull())
            {
                if(mpHeaderParameters == NULL) mpHeaderParameters = new UtlDList();
                HttpRequestContext::parseCgiVariables(headerParameters.data(),
                                                      *mpHeaderParameters, "&", "=",
                                                      FALSE);
            }
            //osPrintf("header param count: %d\n", mHeaderParameters.entries());

        }

        // Parse the field parameters
        if(fieldParameters && *fieldParameters)
        {
            if(mpFieldParameters == NULL) mpFieldParameters = new UtlDList();

            HttpRequestContext::parseCgiVariables(fieldParameters,
                                                  *mpFieldParameters, ";", "=",
                                                  FALSE);
        }
        //osPrintf("field param count: %d\n", mFieldParameters.entries());

        // Strip the parameters/headers from the core url.
        int idx = -1;
        if(paramStart)
        {
            idx = (int)(paramStart - &(urlString[addrSpecBegin]));
        }
        else if(headerStart)
        {
            idx = (int)(headerStart - &(urlString[addrSpecBegin]));
        }
        else if(fieldParameters)
        {
            idx = (int)(fieldParameters - &(urlString[addrSpecBegin]));
        }
        if (idx >= 0) coreUrl.remove(idx);

        // Parse the core URL (url protocol, userid, password, address, port)
        // Probably need a factory here
        // For now just assume SIP if not HTTP
        // int foundSip = coreUrl.index("sip:");
        int foundHttp = coreUrl.index("http://");
        int foundHttps = coreUrl.index("https://");
        int foundFile = coreUrl.index("file://");
        int hostBegin = 0;
        int hostEnd = 0;
        if (foundFile >= 0)
        {
            // Format: file://host/path
            mUrlType = "file";

            // Pull of the hostname
            int filePrefixLength = strlen("file://") ;
            int hostEnd = coreUrl.index('/', filePrefixLength) ;
            if (hostEnd > 0)
            {
                mHostAddress = &(coreUrl.data())[filePrefixLength];
                mHostAddress.remove(hostEnd - filePrefixLength);
                hostEnd++ ;

               int portBegin = mHostAddress.index(':');
               if(portBegin >= 0)
               {
                   mHostPort = atoi(&(mHostAddress)[portBegin + 1]);
                   mHostAddress.remove(portBegin);
               }
               else
               {
                   mHostPort = 0;
               }
            }
            else
            {
                hostEnd = filePrefixLength ;
                mHostPort = 0;
            }

            // Pull out the path
            mPath = &(coreUrl.data())[hostEnd];

    #ifdef _WIN32
            // Massage Data under Windows:  C|/foo.txt --> C:\foo.txt
            mPath.replace('|', ':') ;
            mPath.replace('/', '\\') ;
    #endif
        }
        else if (foundHttp >= 0 || foundHttps >= 0)
        {
            if (foundHttps >=0)
            {
                // https://[userid[:password]@]host[:port][/path...]
                mUrlType = "https";
                hostBegin = coreUrl.first('@');
                mHostAddress = &(coreUrl.data())[hostBegin >= 0 ? hostBegin + 1 : foundHttps + 8];
                hostEnd = mHostAddress.index('/');
            }
            else
            {
               // http://[userid[:password]@]host[:port][/path...]
                mUrlType = "http";
                hostBegin = coreUrl.first('@');
                mHostAddress = &(coreUrl.data())[hostBegin >= 0 ? hostBegin + 1 : foundHttp + 7];
                hostEnd = mHostAddress.index('/');
            }



            if(hostEnd >= 0)
            {
                mPath = &(mHostAddress)[hostEnd];
                mHostAddress.remove(hostEnd);
            }
            int portBegin = mHostAddress.index(':');
            if(portBegin >= 0)
            {
                mHostPort = atoi(&(mHostAddress)[portBegin + 1]);
                mHostAddress.remove(portBegin);
            }
            else
            {
                mHostPort = 0;
            }

            if(hostBegin >= 0)
            {
                if (mUrlType == "https")
                    mUserId = &(coreUrl.data())[foundHttps + 8];
                else
                    mUserId = &(coreUrl.data())[foundHttp + 7];

                mUserId.remove(hostBegin);
                int passwordStart = mUserId.index(':');
                if(passwordStart >= 0)
                {
                    mPassword = &((mUserId)[passwordStart + 1]);
                    int passwordEnd = mPassword.index('@');
                    if(passwordEnd >= 0) mPassword.remove(passwordEnd);
                   mUserId.remove(passwordStart);
                  mPasswordSet = TRUE;
                }
            }
        }

        // For now always assume SIP
        //if(foundSip >= 0)
        else
        {
            UtlString userId;
            UtlString hostAddress;
            SipMessage::parseAddressFromUri(coreUrl.data(),
                &hostAddress,
                &mHostPort,
                NULL,
                &userId);
            mHostAddress = hostAddress;
            mUserId = userId;
            mUrlType = "sip";

            // Check for a password in the URL (it ends up in with
            // the userID
            int passwordStart = mUserId.index(':');
            if(passwordStart >= 0)
            {
                mPassword = &((mUserId)[passwordStart + 1]);
                mUserId.remove(passwordStart);
                mPasswordSet = TRUE;
            }
        }
     }
}

UtlBoolean Url::isUserHostPortEqual(const Url &url) const
{
   int port = url.mHostPort ;
   if(port <= 0)
      port = SIP_PORT;

   int checkPort = mHostPort ;
   if(checkPort <= 0)
      checkPort = SIP_PORT;

   if( mHostAddress.compareTo(url.mHostAddress.data(), UtlString::ignoreCase) == 0
      && mUserId.compareTo(url.mUserId.data()) == 0
      && ( checkPort == port ))
   {
      return TRUE;
   }
   return FALSE;
}


void Url::getIdentity(UtlString &identity) const
{
   identity.remove(0);
   identity.append(mUserId);
   identity.append("@");
   UtlString lowerHostAddress(mHostAddress);
   lowerHostAddress.toLower();
   identity.append(lowerHostAddress);
   if(mHostPort > 0 && mHostPort != 5060)
   {
      char portBuffer[20];
      sprintf(portBuffer, ":%d", mHostPort);
      identity.append(portBuffer);
   }

}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */



