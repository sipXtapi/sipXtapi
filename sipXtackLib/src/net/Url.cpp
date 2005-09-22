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
#include "os/OsSysLog.h"
#include "utl/UtlDListIterator.h"
#include "utl/UtlRegex.h"
#include "net/Url.h"
#include "net/NameValueTokenizer.h"
#include "net/NameValuePair.h"
#include "net/NameValuePairInsensitive.h"
#include "net/SipMessage.h"
#include "net/HttpRequestContext.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

#define DQUOTE "\""
#define LWS "\\s+"
#define SWS "\\s*"
#define SLASHESC "\\"

#define SIP_TOKEN "[a-zA-Z0-9.!%*_+`'~-]+"
#define TOKENS_OR_DQUOTED "(" SIP_TOKEN "(?:" LWS SIP_TOKEN ")*" \
                           "|" DQUOTE "(?:[^" SLASHESC DQUOTE "]" \
                                       "|" SLASHESC DQUOTE \
                                       "|" SLASHESC SLASHESC \
                                      ")*" DQUOTE \
                          ")"

// SipTokenSequenceOrQuoted - used to validate display name values in setDisplayName
const RegEx SipTokenSequenceOrQuoted("^" TOKENS_OR_DQUOTED "$");

// DisplayName - used to parse display name from a url string
//    $1 matches a quoted or unquoted name (including the quotes)
//       but not any leading or trailing whitespace
const RegEx DisplayName("^\\s*" TOKENS_OR_DQUOTED "(?=" SWS "<)" );

// AngleBrackets
//   allows and matches leading whitespace
//   $0 matches any leading whitespace, the angle brackets, and the contents
//   $1 matches just the contents
const RegEx AngleBrackets( SWS "<([^>]+)>" );

// AnyScheme
//   matches any scheme name as defined by RFC 3986
//   allows but does not match leading and trailing whitespace
//   $1 is the scheme name 
const RegEx AnyScheme( SWS "([a-zA-Z][a-zA-Z0-9+.-]+)" SWS ":" );

/* SupportedSchemes - matches any supported scheme name followed by ':'
 *   allows leading whitespace
 *   $0 matches the scheme name with the colon and any leading whitespace
 *   $1 matches the scheme name without the colon
 *
 * IMPORTANT
 *    The number and order of the strings in the following two constants MUST match the
 *    the number and order of the enum values in the Url::Scheme type.
 *
 *    The value SupportedSchemes.Matches()-1 is used to assign the Scheme in Url::Scheme,
 *    so if the above rule is broken, the scheme recognition will not work.
 *
 *    Similarly, the Scheme value is used as an index into SchemeName, so the translation
 *    to a string will be wrong if that is not kept correct.
 */
const RegEx SupportedSchemes( "^(?i)(?:(sip)|(sips)|(http)|(https)|(ftp)|(file)|(mailto))$" );
const char* SchemeName[ Url::NUM_SUPPORTED_URL_SCHEMES ] =
{
   "UNKNOWN-URL-SCHEME",
   "sip",
   "sips",
   "http",
   "https",
   "ftp",
   "file",
   "mailto",
};

// UsernameAndPassword
//   requires and matches the trailing '@'
//   $0 matches user:password@ 
//   $1 matches user
//   $4 matches password
const RegEx UsernameAndPassword(
   "("
     "("
         "[a-zA-Z0-9_.!~*'()&=+$,;?/-]"
      "|"
         "%[0-9a-fA-F]{2}"
      ")+"
    ")"
   "(:"
      "("
        "("
           "[a-zA-Z0-9_.!~*'()&=+$,-]"
        "|"
           "%[0-9a-fA-F]{2}"
        ")*"
      ")"
    ")?"
    "@"
                                   );

// Host Address and Port
//   does not allow leading whitespace
//   $0 matches host:port
//   $1 matches host
//   $3 matches port
#define DOMAIN_LABEL "(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]*[a-zA-Z0-9])?)"
const RegEx HostAndPort( 
   "("
    "(?:" DOMAIN_LABEL "\\.)*" DOMAIN_LABEL "\\.?" // DNS name 
   "|"
    "(?:[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3})" // IPv4 address
   "|"
    "(?:\\[[0-9a-fA-F:.]+\\])" // IPv6 address
   ")"
  "(:([0-9]+))?" // port number
                        );

// UrlPath
//   does not allow whitespace
//   does not require, but matches a trailing '?'
//   $0 matches path?
//   $1 matches path
const RegEx UrlPath( "([^?\\s]+)\\??" );

// UrlParams
//   allows leading whitespace
//   is terminated by but does not require a trailing '?' or '>'
//   $0 matches ;params
//   $1 matches params
const RegEx UrlParams( SWS ";([^?>]+)" );

// FieldParams
//   allows leading whitespace
//   is terminated by end of string
//   $0 matches ;params
//   $1 matches params
const RegEx FieldParams( SWS ";(.+)$" );

// HeaderOrQueryParams
//   allows leading whitespace
//   is terminated by but does not require a trailing '>'
//   $0 matches ?params
//   $1 matches params
const RegEx HeaderOrQueryParams( SWS "\\?([^>]+)>?" );

// AllDigits
const RegEx AllDigits("^\\+?[0-9*]+$");

// STATIC VARIABLE INITIALIZATIONS

#ifndef min
#define min(x,y) (((x) < (y)) ? (x) : (y))
#endif

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
Url::Url(const char* urlString, UtlBoolean isAddrSpec) :
   mScheme(SipUrlScheme),
   mPasswordSet(FALSE),
   mHostPort(PORT_NONE),
   mpUrlParameters(NULL),
   mpHeaderOrQueryParameters(NULL),
   mpFieldParameters(NULL),
   mAngleBracketsIncluded(FALSE)
{
   if (urlString && *urlString)
   {
      parseString(urlString ,isAddrSpec);
   }
}

// Copy constructor
Url::Url(const Url& rUrl) :
   mPasswordSet(FALSE),
   mHostPort(PORT_NONE),
   mpUrlParameters(NULL),
   mpHeaderOrQueryParameters(NULL),
   mpFieldParameters(NULL),
   mAngleBracketsIncluded(FALSE)
{
    *this = rUrl;
}

// Destructor
Url::~Url()
{
    removeParameters();
}

void Url::removeParameters()
{
   removeUrlParameters();
   removeFieldParameters();
   removeHeaderParameters();
}

void Url::reset()
{
    removeParameters();
    mScheme = SipUrlScheme;
    mDisplayName.remove(0);
    mUserId.remove(0);
    mPassword.remove(0);
    mPasswordSet = FALSE;
    mHostAddress.remove(0);
    mHostPort = PORT_NONE;
    mPath.remove(0);
    mAngleBracketsIncluded = FALSE;
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
Url&
Url::operator=(const Url& rhs)
{
   if (this != &rhs) // handle the assignment to self case
   {
      reset();

      // Copy the members
      mScheme = rhs.mScheme;
      mDisplayName = rhs.mDisplayName;
      mUserId = rhs.mUserId;
      mPassword = rhs.mPassword;
      mPasswordSet = rhs.mPasswordSet;
      mHostAddress = rhs.mHostAddress;
      mHostPort = rhs.mHostPort;
      mPath = rhs.mPath;
      mAngleBracketsIncluded = rhs.mAngleBracketsIncluded;

      if (rhs.mpUrlParameters)
      {
         mpUrlParameters = new UtlDList;
         
         UtlDListIterator paramIterator(*rhs.mpUrlParameters);
         NameValuePair* rhsParam;
         while ((rhsParam = (NameValuePair*) paramIterator()))
         {
            mpUrlParameters->append(new NameValuePair(*rhsParam));
         }
      }
      else
      {
         mRawUrlParameters = rhs.mRawUrlParameters;
      }

      if (rhs.mpHeaderOrQueryParameters)
      {
         mpHeaderOrQueryParameters = new UtlDList;
         
         UtlDListIterator paramIterator(*rhs.mpHeaderOrQueryParameters);
         NameValuePair* rhsParam;
         while ((rhsParam = (NameValuePair*) paramIterator()))
         {
            mpHeaderOrQueryParameters->append(new NameValuePair(*rhsParam));
         }
      }
      else
      {
         mRawHeaderOrQueryParameters = rhs.mRawHeaderOrQueryParameters;
      }

      if (rhs.mpFieldParameters)
      {
         mpFieldParameters = new UtlDList;
         
         UtlDListIterator paramIterator(*rhs.mpFieldParameters);
         NameValuePair* rhsParam;
         while ((rhsParam = (NameValuePair*) paramIterator()))
         {
            mpFieldParameters->append(new NameValuePair(*rhsParam));
         }         
      }
      else
      {
         mRawFieldParameters = rhs.mRawFieldParameters;
      }
   }

   return *this;
}

Url& Url::operator=(const char* urlString)
{
   reset();
   
   if (urlString && *urlString)
   {
      parseString(urlString,FALSE);
   }

   return *this;
}
/* ============================ ACCESSORS ================================= */


Url::Scheme Url::getScheme() const
{
   return mScheme;
}

void Url::getUrlType(UtlString& urlProtocol) const
{
    urlProtocol = SchemeName[mScheme];
}

void Url::setScheme(Url::Scheme scheme)
{
   mScheme = scheme;
}

void Url::setUrlType(const char* urlProtocol)
{
   if (urlProtocol)
   {
      UtlString schemeName(urlProtocol);
      
      mScheme = scheme(schemeName);

      if ( UnknownUrlScheme == mScheme )
      {
         OsSysLog::add(FAC_SIP, PRI_ERR, "Url::setUrlType unsupported Url scheme '%s'",
                       urlProtocol
                       );
      }
   }
   else
   {
      // no urlProtocol value passed
      OsSysLog::add(FAC_SIP, PRI_CRIT, "Url::setUrlType Url scheme NULL");
      assert(urlProtocol);
      mScheme = UnknownUrlScheme;
   }
}

void Url::getDisplayName(UtlString& displayName) const
{
    displayName = mDisplayName;
    if (isDigitString(mDisplayName.data()))
    {
       NameValueTokenizer::frontBackTrim(&displayName, "\"");
    }
}

void Url::setDisplayName(const char* displayName)
{
   mDisplayName.remove(0);

   if (displayName && strlen(displayName))
   {
       RegEx tokenSequenceOrQuoted(SipTokenSequenceOrQuoted);
       if (tokenSequenceOrQuoted.Search(displayName))
       {
          mDisplayName = displayName;
       }
       else
       {
          assert(FALSE); // invalid display name value
       }
   }
}

void Url::getUserId(UtlString& userId) const
{
   userId = mUserId;
}

void Url::setUserId(const char* userId)
{
   if (userId)
   {
      mUserId = userId;
   }
   else
   {
      mUserId.remove(0);
   }
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
   {
      mPath = path;
   }
   else
   {
      mPath.remove(0);
   }
}

UtlBoolean Url::getPath(UtlString& path, UtlBoolean getStyle)
{
    path = mPath;

    // If the desire is to have an HTTP GET style path with CGI variables
    if(getStyle)
    {
       if (   (   mpHeaderOrQueryParameters
               || const_cast<Url*>(this)->parseHeaderOrQueryParameters()
               )
           && mpHeaderOrQueryParameters->entries()
           )
        {
            UtlDListIterator headerParamIterator(*mpHeaderOrQueryParameters);
            NameValuePair* headerParam = NULL;
            UtlString headerParamValue ;
            UtlBoolean firstHeader = TRUE;

            while ((headerParam = (NameValuePair*) headerParamIterator()))
            {
                // Add separator for first header parameter
                if(firstHeader)
                {
                    path.append("?", 1);
                    firstHeader = FALSE;
                }
                else
                {
                    path.append("&", 1);
                }

                path.append(*headerParam);
                headerParamValue = headerParam->getValue();
                if(!headerParamValue.isNull())
                {
                    path.append("=", 1);
                    HttpMessage::escape(headerParamValue);

                    path.append(headerParamValue);
                }
            }
        } // endif mpHeaderOrQueryParameters
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
    if(mpUrlParameters || parseUrlParameters())
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

    if (   (urlIndex >= 0)
        && (mpUrlParameters || parseUrlParameters())
        && (((int)(mpUrlParameters->entries())) > urlIndex) 
        )
    {
       url = (NameValuePair*) mpUrlParameters->at(urlIndex);
    }
    
    if(url)
    {
        name = *url;
        value = url->getValue();
    }

    return(NULL != url);
}

UtlBoolean Url::getUrlParameters(int iMaxReturn, UtlString* pNames, UtlString *pValues, int& iActualReturn)
{
    if(! (mpUrlParameters || parseUrlParameters()))
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

    // ensure that mpUrlParameters is initialized
    if (! (mpUrlParameters || parseUrlParameters()))
    {
       mpUrlParameters = new UtlDList;
    }
    
    mpUrlParameters->append(nv);
}

UtlBoolean Url::getHeaderParameter(const char* name, UtlString& value, int index)
{
    int foundIndex = 0;
    UtlBoolean found = FALSE;
    value = "";
    if(mpHeaderOrQueryParameters || parseHeaderOrQueryParameters())
    {
        UtlDListIterator headerParamIterator(*mpHeaderOrQueryParameters);
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
    if(!(mpHeaderOrQueryParameters || parseHeaderOrQueryParameters()))
    {
        iActualReturn = 0;
    }

    // If the pValue is null, return false and set the actual return to the actual
    // number of items.
    else if (pValues == NULL || pNames == NULL)
    {
        iActualReturn = mpHeaderOrQueryParameters->entries() ;
        return FALSE ;
    }
    else
    {
        iActualReturn = min(iMaxReturn, ((int)(mpHeaderOrQueryParameters->entries()))) ;

        for (int i=0; i<iActualReturn; i++)
        {
            NameValuePair *pair = (NameValuePair*) mpHeaderOrQueryParameters->at(i) ;
            pNames[i] = *pair;
            pValues[i] = pair->getValue() ;
        }
    }
    return (iActualReturn > 0) ;
}


void Url::removeUrlParameters()
{
   if(mpUrlParameters)
   {
      mpUrlParameters->destroyAll();
      delete mpUrlParameters;
      mpUrlParameters = NULL;
   }
   else
   {
      mRawUrlParameters.remove(0);
   }
}

void Url::removeUrlParameter(const char* name)
{
    if(mpUrlParameters || parseUrlParameters())
    {
        NameValuePairInsensitive nv(name ? name : "", NULL);

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
   // Insert the scheme
    urlString = schemeName(mScheme);
    urlString.append(":",1);
    
    switch(mScheme)
    {
    case FileUrlScheme:
    case FtpUrlScheme:
    case HttpUrlScheme:
    case HttpsUrlScheme:
       urlString.append("//",2);
       break;

    case SipUrlScheme:
    case SipsUrlScheme:
    case MailtoUrlScheme:
    default:
       break;
    }

    // Add the user 
    if (FileUrlScheme != mScheme) // no user defined in a file url
    {
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
    }

    // Add the host
    urlString.append(mHostAddress);
    if(portIsValid(mHostPort))
    {
       char portBuffer[20];
       sprintf(portBuffer, ":%d", mHostPort);
       urlString.append(portBuffer);
    }

    // Add the path
    switch(mScheme)
    {
    case FileUrlScheme:
    case FtpUrlScheme:
    case HttpUrlScheme:
    case HttpsUrlScheme:
       if(!mPath.isNull())
       {
          urlString.append(mPath);
       }
       break;

    case SipUrlScheme:
    case SipsUrlScheme:
    case MailtoUrlScheme:
    default:
       break;
    }

    // Add the URL parameters
    if (   (   mpUrlParameters
            || const_cast<Url*>(this)->parseUrlParameters()
            )
        && mpUrlParameters->entries()
        )
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
    if (   (   mpHeaderOrQueryParameters
            || const_cast<Url*>(this)->parseHeaderOrQueryParameters()
            )
        && mpHeaderOrQueryParameters->entries()
        )
    {
        UtlDListIterator headerParamIterator(*mpHeaderOrQueryParameters);
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
    NameValuePair* nv = new NameValuePair(name ? name : "",
        value ? value : "");

    // ensure that mpHeaderOrQueryParameters is initialized
    if (! (mpHeaderOrQueryParameters || parseHeaderOrQueryParameters()))
    {
       mpHeaderOrQueryParameters = new UtlDList;
    }

    mpHeaderOrQueryParameters->append(nv);
}

UtlBoolean Url::getHeaderParameter(int headerIndex, UtlString& name, UtlString& value)
{
    NameValuePair* header = NULL;

    if (   (headerIndex >= 0)
        && (mpHeaderOrQueryParameters || parseHeaderOrQueryParameters())
        && (((int)(mpHeaderOrQueryParameters->entries())) > headerIndex)
        )
    {
       header = (NameValuePair*) mpHeaderOrQueryParameters->at(headerIndex);
    }
    
    if(header)
    {
        name = *header;
        value = header->getValue();
    }

    return(NULL != header);
}

void Url::removeHeaderParameters()
{
   if(mpHeaderOrQueryParameters)
   {
      mpHeaderOrQueryParameters->destroyAll();
      delete mpHeaderOrQueryParameters;
      mpHeaderOrQueryParameters = NULL;
   }
   mRawHeaderOrQueryParameters.remove(0);
}

void Url::removeHeaderParameter(const char* name)
{
    if(mpHeaderOrQueryParameters || parseHeaderOrQueryParameters())
    {
        NameValuePairInsensitive nv(name ? name : "", NULL);

        UtlDListIterator iterator(*mpHeaderOrQueryParameters);

        UtlContainable* matchingParam;
        while((matchingParam=iterator.findNext(&nv)))
        {
           mpHeaderOrQueryParameters->destroy(matchingParam);
        }
    }
}

UtlBoolean Url::getFieldParameter(const char* name, UtlString& value, int index) const
{
    int foundIndex = 0;
    UtlBoolean found = FALSE;
    value = "";
    if(mpFieldParameters || const_cast<Url*>(this)->parseFieldParameters())
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

    if (   fieldIndex >= 0
        && (mpFieldParameters || parseFieldParameters())
        && ((int)(mpFieldParameters->entries())) > fieldIndex 
        )
    {
       field = (NameValuePair*) mpFieldParameters->at(fieldIndex);
    }
    
    if(field)
    {
        name = *field;
        value = field->getValue();
    }

    return(NULL != field);
}

UtlBoolean Url::getFieldParameters(int iMaxReturn, UtlString* pNames, UtlString *pValues, int& iActualReturn)
{
    if(!(mpFieldParameters || parseFieldParameters()))
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

    // ensure that mpFieldParameters is initialized
    if (! (mpFieldParameters || parseFieldParameters()))
    {
       mpFieldParameters = new UtlDList;
    }
    
    mpFieldParameters->append(nv);
}

void Url::removeFieldParameters()
{
   if(mpFieldParameters)
   {
      mpFieldParameters->destroyAll();
      delete mpFieldParameters;
      mpFieldParameters = NULL;
   }
   mRawFieldParameters.remove(0);
}

void Url::removeFieldParameter(const char* name)
{
    if(mpFieldParameters || parseFieldParameters())
    {
        NameValuePairInsensitive nv(name ? name : "", NULL);

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

   // This is a replace operation; clear the storage string
   urlString.remove(0);

   if ( !mDisplayName.isNull() )
   {
      urlString.append(mDisplayName);
      isNameAddr = TRUE;
   }

   bool haveUrlParams = (   (   mpUrlParameters
                             || const_cast<Url*>(this)->parseUrlParameters()
                             )
                         && mpUrlParameters->entries()
                         );
   bool haveHdrParams = (   (   mpHeaderOrQueryParameters
                             || const_cast<Url*>(this)->parseHeaderOrQueryParameters()
                             )
                         && mpHeaderOrQueryParameters->entries()
                         );

   bool haveFldParams = (   (   mpFieldParameters
                             || const_cast<Url*>(this)->parseFieldParameters()
                             )
                         && mpFieldParameters->entries()
                         );
   
   // If this should be nameAddr as opposed to addrSpec
   // (i.e. do we need anglebrackets)
   if (   isNameAddr                             // There was a Display name
       || mAngleBracketsIncluded                 // Explicit setting from the caller
       || haveFldParams
       || (   ( SipUrlScheme == mScheme || SipsUrlScheme == mScheme )
           && ( haveUrlParams || haveHdrParams )
           )
       )
   {
       urlString.append("<", 1);
       isNameAddr = TRUE;
   }

   UtlString theAddrSpec;
   const_cast<Url*>(this)->getUri(theAddrSpec);
   urlString.append(theAddrSpec);
   
   // Add the terminating angle bracket
   if(isNameAddr)
   {
      urlString.append(">", 1);
   }

   // Add the field parameters
   if(haveFldParams)
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
            Url::gen_value_escape(fieldParamValue);
            urlString.append(fieldParamValue);
         }
      }
   }
}

void Url::dump()
{
    UtlString proto;
    getUrlType(proto);
    printf("Url type: '%s'\n", proto.data());

    UtlString disp;
    getDisplayName(disp);
    printf("DisplayName: '%s'\n", disp.data());

    UtlString user;
    getUserId(user);
    printf("UserId: '%s'\n", user.data());

    UtlString pwd;
    getPassword(pwd);
    printf("Password: '%s'\n", pwd.data());

    UtlString server;
    getHostAddress(server);
    printf("Address: '%s'\n", server.data());

    int port = getHostPort();
    printf("Port: %d\n", port);

    UtlString callId;
    getHeaderParameter("call-id", callId);
    printf("Call-Id: '%s'\n", callId.data());

    UtlString name;
    UtlString value;
    int index = 0;
    printf("\nHeader Parameters:\n");
    while(getHeaderParameter(index, name, value))
    {
        printf("'%s'='%s'\n", name.data(), value.data());
        index++;
    }

    index = 0;
    printf("\nField Parameters:\n");
    while(getFieldParameter(index, name, value))
    {
        printf("'%s'='%s'\n", name.data(), value.data());
        index++;
    }

    index = 0;
    printf("\nURL Parameters:\n");
    while(getUrlParameter(index, name, value))
    {
        printf("'%s'='%s'\n", name.data(), value.data());
        index++;
    }
}

/* ============================ INQUIRY =================================== */

UtlBoolean Url::isDigitString(const char* dialedCharacters)
{
    // Must be digits or *
    RegEx allDigits(AllDigits);
    return allDigits.Search(dialedCharacters);
}


UtlBoolean Url::isIncludeAngleBracketsSet() const
{
    return mAngleBracketsIncluded ;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void Url::parseString(const char* urlString, UtlBoolean isAddrSpec)
{
   // If isAddrSpec:
   //                userinfo@hostport;uriParameters?headerParameters
   // If !isAddrSpec:
   //    DisplayName<userinfo@hostport;urlParameters?headerParameters>;fieldParameters

   // Try to catch when a name-addr is passed but we are expecting an
   // addr-spec -- many name-addr's start with '<' or '"'.
   if (isAddrSpec && (urlString[0] == '<' || urlString[0] == '"'))
   {
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "Url::parseString Invalid addr-spec found (probably name-addr format): '%s'",
                    urlString);
   }

   int workingOffset = 0; // begin at the beginning...
   
   size_t afterAngleBrackets = UTL_NOT_FOUND;
   
   if (isAddrSpec)
   {
      mAngleBracketsIncluded = FALSE; 
   }
   else // ! addr-spec
   {
      // Is there a display name on the front?
      mDisplayName.remove(0);
      RegEx displayName(DisplayName);
      if (displayName.SearchAt(urlString, workingOffset))
      {
         displayName.MatchString(&mDisplayName, 1); // does not include whitespace
         workingOffset = displayName.AfterMatch(0);
      }

      // Are there angle brackets around the URI?
      RegEx angleBrackets(AngleBrackets);
      if (angleBrackets.SearchAt(urlString, workingOffset))
      {

         workingOffset = angleBrackets.MatchStart(1); // inside the angle brackets
         afterAngleBrackets = angleBrackets.AfterMatch(0); // following the '>'
         
         /*
          * Note: We do not set mAngleBracketsIncluded just because we saw them
          *       That is only used for explicit control from the outside.
          *       The local knowledge of whether or not there are angle brackets
          *       is whether or not afterAngleBrackets == UTL_NOT_FOUND
          */
      }
   }

   UtlString foundScheme;

   // Parse the url type (aka scheme)
   RegEx anyScheme(AnyScheme);
   if (   (anyScheme.SearchAt(urlString,workingOffset))
       && (anyScheme.MatchStart(0) == workingOffset)
       )
   {
      anyScheme.MatchString(&foundScheme,1);
      mScheme = scheme(foundScheme);

      /*
       * There is a potential ambiguity here.
       * Consider the url 'foo:333' - it could be:
       *       scheme 'foo' host '333' ('333' is a valid local host name - bad idea, but legal)
       *   or  host   'foo' port '333' (and scheme 'sip' is implied)
       *
       * Now make it worse by using 'sips' as a hostname:
       *   'sips:333'     
       *       scheme 'sips' host '333'
       *   or  host   'sips' port '333' (and scheme 'sip' is implied)
       *
       * We resolve the first case by treating anything left of the colon as a scheme if
       * it is one of the supported schemes.  Otherwise, we set the scheme to the
       * default (sip) and go on so that it will be parsed as a hostname.  This does not
       * do the right thing for the (scheme 'sips' host '333') case, but they get what
       * they deserve.
       */
      if (UnknownUrlScheme != mScheme)
      {
         workingOffset = anyScheme.AfterMatch(0); // past the ':'
      }
      else
      {
         // foundScheme is not a supported scheme, so guess that it's a host and "sip:" is implied
         mScheme = SipUrlScheme;
      }
   }
   else
   {
      // no scheme specified, so assume 'sip'
      mScheme = SipUrlScheme;
   }

   // skip over any '//' following the scheme for the ones that use that
   switch (mScheme)
   {
   case FileUrlScheme:
   case FtpUrlScheme:
   case HttpUrlScheme:
   case HttpsUrlScheme:
      if (0==strncmp("//", urlString+workingOffset, 2))
      {
         workingOffset += 2;
      }
      break;

   case UnknownUrlScheme:
   case SipUrlScheme:
   case SipsUrlScheme:
   case MailtoUrlScheme:
   default:
      break;
   }
   
   if (FileUrlScheme != mScheme) // no user part in file urls
   {
      // Parse the username and password
      RegEx usernameAndPassword(UsernameAndPassword);
      if (   (usernameAndPassword.SearchAt(urlString, workingOffset))
          && usernameAndPassword.MatchStart(0) == workingOffset 
          )
      {
         usernameAndPassword.MatchString(&mUserId, 1);
         usernameAndPassword.MatchString(&mPassword, 4);
         workingOffset = usernameAndPassword.AfterMatch(0);
      }
   }

   // Parse the hostname and port
   RegEx hostAndPort(HostAndPort);
   if (   (hostAndPort.SearchAt(urlString,workingOffset))
       && (hostAndPort.MatchStart(0) == workingOffset)
       )
   {
      hostAndPort.MatchString(&mHostAddress,1);
      UtlString portStr;
      if (hostAndPort.MatchString(&portStr,3))
      {
         mHostPort = atoi(portStr.data());
      }
      else
      {
         mHostPort = PORT_NONE;
      }

      workingOffset = hostAndPort.AfterMatch(0);
   }
   else
   {
      if (FileUrlScheme != mScheme) // no host is ok in a file URL
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "Url::parseString no valid host found at char %d in '%s', "
                       "isAddrSpec = %d",
                       workingOffset, urlString, isAddrSpec
                       );
      }
   }
   
   // Next is a path if http, https, or ftp,
   //      OR url parameters if sip or sips.
   // There can be no Url parameters for http, https, or ftp
   //    because semicolon is a valid part of the path value
   switch ( mScheme )
   {
   case FileUrlScheme:
   case FtpUrlScheme:
   case HttpUrlScheme:
   case HttpsUrlScheme:
   {
      // this is an http, https, or ftp URL, so get the path
      RegEx urlPath(UrlPath);
      if (   (urlPath.SearchAt(urlString, workingOffset))
          && (urlPath.MatchStart(0) == workingOffset)
          )
      {
         urlPath.MatchString(&mPath,1);
         workingOffset = urlPath.AfterMatch(1);
      }
#     ifdef _WIN32
      {
         // Massage Data under Windows:  C|/foo.txt --> C:\foo.txt
         mPath.replace('|', ':');
         mPath.replace('/', '\\');
      }
#     endif
   }
   break;

   case SipUrlScheme:
   case SipsUrlScheme:
   {
      // it may have url parameters of the form ";" param "=" value ...
      //                iff it meets the right conditions:
      if (   isAddrSpec                          // in addr-spec, any param is a url param
          || afterAngleBrackets != UTL_NOT_FOUND // inside angle brackets there may be a url param
          ) 
      {
         RegEx urlParams(UrlParams);
         if (   (urlParams.SearchAt(urlString, workingOffset))
             && (urlParams.MatchStart(0) == workingOffset)
             )
         {
            urlParams.MatchString(&mRawUrlParameters, 1);
            workingOffset = urlParams.AfterMatch(1);

            // actual parsing of the parameters is in parseUrlParameters
            // so that it only happens if someone asks for them.
         }
      }
   }
   break;
   
   case MailtoUrlScheme:
   default:
      // no path component
      break;
   }

   // Parse any header or query parameters
   RegEx headerOrQueryParams(HeaderOrQueryParams);
   if(   (headerOrQueryParams.SearchAt(urlString, workingOffset))
      && (headerOrQueryParams.MatchStart(0) == workingOffset)
      )
   {
      headerOrQueryParams.MatchString(&mRawHeaderOrQueryParameters, 1);
      workingOffset = headerOrQueryParams.AfterMatch(0);
            
      // actual parsing of the parameters is in parseHeaderOrQueryParameters
      // so that it only happens if someone asks for them.
   }

   // Parse the field parameters
   if (!isAddrSpec) // can't have field parameters in an addrspec
   {
      if (afterAngleBrackets != UTL_NOT_FOUND)
      {
         workingOffset = afterAngleBrackets;
      }

      RegEx fieldParameters(FieldParams);
      if (   (fieldParameters.SearchAt(urlString, workingOffset))
          && (fieldParameters.MatchStart(0) == workingOffset)
          )
      {
         fieldParameters.MatchString(&mRawFieldParameters, 1);

         // actual parsing of the parameters is in parseFieldParameters
         // so that it only happens if someone asks for them.
      }
   }
}

UtlBoolean Url::isUserHostPortEqual(const Url &url) const
{
   // Compare the relevant components of the URI.
   return (   mHostAddress.compareTo(url.mHostAddress.data(), UtlString::ignoreCase) == 0
           && mUserId.compareTo(url.mUserId.data()) == 0
           && mHostPort == url.mHostPort );
}


void Url::getIdentity(UtlString &identity) const
{
   identity.remove(0);
   identity.append(mUserId);
   identity.append("@", 1);
   UtlString lowerHostAddress(mHostAddress);
   lowerHostAddress.toLower();
   identity.append(lowerHostAddress);

   // If the port designates an actual port, it must be specified.
   if(portIsValid(mHostPort))
   {
      char portBuffer[20];
      sprintf(portBuffer, ":%d", mHostPort);
      identity.append(portBuffer);
   }
}

/// Translate a scheme string (not including the terminating colon) to a Scheme enum.
Url::Scheme Url::scheme( const UtlString& schemeName )
{
   Scheme theScheme;
   
   RegEx supportedSchemes(SupportedSchemes);
   if (supportedSchemes.Search(schemeName.data()))
   {
      theScheme = static_cast<Scheme>(supportedSchemes.Matches()-1);
   }
   else
   {
      theScheme = UnknownUrlScheme;
   }
   return theScheme;
}


/// Get the canonical (lowercase) name of a supported Scheme.
const char* Url::schemeName( Url::Scheme scheme )
{
   const char* theName;
   if (scheme > UnknownUrlScheme && scheme < NUM_SUPPORTED_URL_SCHEMES)
   {
      theName = SchemeName[scheme];
   }
   else
   {
      theName = SchemeName[UnknownUrlScheme];
   }
   return theName;
}
   

/* //////////////////////////// PRIVATE /////////////////////////////////// */

bool Url::parseUrlParameters() 
{
   if (!mpUrlParameters && !mRawUrlParameters.isNull())
   {
      mpUrlParameters = new UtlDList();

      HttpRequestContext::parseCgiVariables(mRawUrlParameters,
                                            *mpUrlParameters, ";", "=",
                                            TRUE, &HttpMessage::unescape);
      mRawUrlParameters.remove(0);
   }

   return mpUrlParameters != NULL;
}

bool Url::parseHeaderOrQueryParameters() 
{
   if (!mpHeaderOrQueryParameters && !mRawHeaderOrQueryParameters.isNull())
   {
      mpHeaderOrQueryParameters = new UtlDList();

      HttpRequestContext::parseCgiVariables(mRawHeaderOrQueryParameters,
                                            *mpHeaderOrQueryParameters, "&", "=",
                                            TRUE, &HttpMessage::unescape);
      mRawHeaderOrQueryParameters.remove(0);
   }

   return mpHeaderOrQueryParameters != NULL;
}

bool Url::parseFieldParameters() 
{
   if (!mpFieldParameters && !mRawFieldParameters.isNull())
   {
      mpFieldParameters = new UtlDList();

#if 0
      printf("Url::parseFieldParameters mRawFieldParameters = '%s'\n",
             mRawFieldParameters.data());
#endif
      HttpRequestContext::parseCgiVariables(mRawFieldParameters,
                                            *mpFieldParameters, ";", "=",
                                            TRUE, &Url::gen_value_unescape);
      mRawFieldParameters.remove(0);
   }

   return mpFieldParameters != NULL;
}

void Url::gen_value_unescape(UtlString& escapedText)
{
#if 0
   printf("Url::gen_value_unescape before escapedText = '%s'\n",
          escapedText.data());
#endif

    //UtlString unescapedText;
    int numUnescapedChars = 0;
    const char* unescapedTextPtr = escapedText;
    // The number of unescaped characters is always less than
    // or equal to the number of escaped characters.  Therefore
    // we will cheat a little and used the escapedText as
    // the destiniation to directly write characters in place
    // as the append method is very expensive
    char* resultPtr = new char[escapedText.length() + 1];

    // Skip initial whitespace, which may be before the starting double-quote
    // of a quoted string.  Tokens and hosts are not allowed to start with
    // whitespace.
    while (*unescapedTextPtr &&
           (*unescapedTextPtr == ' ' || *unescapedTextPtr == '\t'))
    {
       // Consume the whitespace character.
       unescapedTextPtr++;
       numUnescapedChars++;
    }

    // Examine the first character to see if it is a double-quote.
    if (*unescapedTextPtr == '"')
    {
       // Skip the initial double-quote.
       unescapedTextPtr++;
       while (*unescapedTextPtr)
       {
          // Substitute a (backslash-)quoted-pair.
          if (*unescapedTextPtr == '\\')
          {
             // Get the next char.
             unescapedTextPtr++;
             // Don't get deceived if there is no next character.
             if (*unescapedTextPtr)
             {
                // The next character is copied unchanged.
                resultPtr[numUnescapedChars] = *unescapedTextPtr;
                numUnescapedChars++;
             }
          }
          // A double-quote without backslash ends the string.
          else if (*unescapedTextPtr == '"')
          {
             break;
          }
          // Char is face value.
          else
          {
             resultPtr[numUnescapedChars] = *unescapedTextPtr;
             numUnescapedChars++;
          }
          // Go to the next character
          unescapedTextPtr++;
       }
    }
    else
    {
       // It is a token or host, and can be copied unchanged.
       while (*unescapedTextPtr)
       {
          resultPtr[numUnescapedChars] = *unescapedTextPtr;
          numUnescapedChars++;
          // Go to the next character
          unescapedTextPtr++;
       }
    }
    
    // Copy back into the UtlString.
    resultPtr[numUnescapedChars] = '\0';
    escapedText.replace(0, numUnescapedChars, resultPtr);
    escapedText.remove(numUnescapedChars);
    delete[] resultPtr;

#if 0
   printf("Url::gen_value_unescape after escapedText = '%s'\n",
          escapedText.data());
#endif
}

void Url::gen_value_escape(UtlString& unEscapedText)
{
   // Check if there are any characters in unEscapedText that need to be
   // escaped in a field parameter value.
   if (strspn(unEscapedText.data(),
              // Alphanumerics
              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
              "abcdefghijklmnopqrstuvwxyz"
              "0123456789"
              // Characters allowed in tokens
              "-.!%*_+`'~"
              // Additional characters allowed by the syntax of "host"
              "[]:") != unEscapedText.length())
   {
      // Temporary string to construct the escaped value in.
      UtlString escapedText;
      // Pre-size it to the size of the un-escaped test, plus 2 for
      // the starting and ending double-quotes.
      escapedText.capacity((size_t) unEscapedText.length() + 2);
      const char* unescapedTextPtr = unEscapedText.data();

      // Start with double-quote.
      escapedText.append("\"");

      // Process each character of the un-escaped value.
      while(*unescapedTextPtr)
      {
         char unEscapedChar = *unescapedTextPtr;
         if (unEscapedChar == '"' || unEscapedChar == '\\')
         {
            // Construct a little 2-character string and append it.
            char escapedChar[2];
            escapedChar[0] = '\\';
            escapedChar[1] = *unescapedTextPtr;
            escapedText.append(&unEscapedChar, 2);
        }
        else
        {
           // Append the character directly.
           escapedText.append(&unEscapedChar, 1);
        }
         // Consider the next character.
         unescapedTextPtr++;
      }

      // End with double-quote.
      escapedText.append("\"");

      // Write the escaped string into the argumemt.
      unEscapedText = escapedText;
   }
}

/* ============================ FUNCTIONS ================================= */
