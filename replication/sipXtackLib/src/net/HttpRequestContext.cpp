//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <utl/UtlDListIterator.h>
#include <net/HttpRequestContext.h>
#include <net/HttpMessage.h>
#include <net/NameValueTokenizer.h>
#include <net/NameValuePair.h>
#include <net/NameValuePairInsensitive.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
//#define TEST_DEBUG

#ifdef TEST_DEBUG
#  include <os/OsSysLog.h>
#endif

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */
/* ============================ CREATORS ================================== */
// Constructor
HttpRequestContext::HttpRequestContext(const char* requestMethod,
                                       const char* rawUrl,
                                       const char* mappedFile,
                                       const char* serverName,
                                       const char* userId) :
   mUsingInsensitive(false)
{
   if(requestMethod)
   {
       mEnvironmentVars[HTTP_ENV_REQUEST_METHOD].append(requestMethod);
       mEnvironmentVars[HTTP_ENV_REQUEST_METHOD].toUpper();

   }

   if(rawUrl)
   {
       mEnvironmentVars[HTTP_ENV_RAW_URL].append(rawUrl);
       mEnvironmentVars[HTTP_ENV_UNMAPPED_FILE].append(rawUrl);
       int fileEndIndex = mEnvironmentVars[HTTP_ENV_RAW_URL].index('?');
       if(fileEndIndex > 0)
       {
           mEnvironmentVars[HTTP_ENV_UNMAPPED_FILE].remove(fileEndIndex);
           mEnvironmentVars[HTTP_ENV_QUERY_STRING].append((&(mEnvironmentVars[HTTP_ENV_RAW_URL].data())[fileEndIndex + 1]));
           parseCgiVariables(mEnvironmentVars[HTTP_ENV_QUERY_STRING].data());
       }
   }

   if(mappedFile)
   {
       mEnvironmentVars[HTTP_ENV_MAPPED_FILE].append(mappedFile);
   }

   if(serverName)
   {
       mEnvironmentVars[HTTP_ENV_SERVER_NAME].append(serverName);
   }

   if(userId)
   {
       mEnvironmentVars[HTTP_ENV_USER].append(userId);
   }
}

// Copy constructor
HttpRequestContext::HttpRequestContext(const HttpRequestContext& rHttpRequestContext)
{
   //copy mEnvironmentVars[HTTP_ENV_LAST]
   int count = 0;
   for (count = HttpRequestContext::HTTP_ENV_RAW_URL;
        count < HttpRequestContext::HTTP_ENV_LAST ;
        count ++)
        {
           if(!rHttpRequestContext.mEnvironmentVars[count].isNull())
           {
              mEnvironmentVars[count].remove(0);
              mEnvironmentVars[count].append(rHttpRequestContext.mEnvironmentVars[count]);
           }
        }

   // delete the old values in the UtlDList
   if(!mCgiVariableList.isEmpty())
   {
      mCgiVariableList.destroyAll();
   }

   //copy mCgiVariableList memebers individually
   mUsingInsensitive = rHttpRequestContext.mUsingInsensitive;
        UtlDListIterator iterator((UtlDList&)rHttpRequestContext.mCgiVariableList);
        NameValuePair* nameValuePair = NULL;
   UtlString value;
   UtlString name;
   int index = 0;
   do
   {
      nameValuePair = (NameValuePair*)iterator();
      if(nameValuePair)
      {
         name = *nameValuePair;
         value = nameValuePair->getValue();
         NameValuePair* newNvPair = ( mUsingInsensitive
                                     ? new NameValuePair(name, value)
                                     : new NameValuePairInsensitive(name, value)
                                     );
         mCgiVariableList.insertAt(index, newNvPair);
         index ++;
      }
   }
   while (nameValuePair != NULL);
}

// Destructor
HttpRequestContext::~HttpRequestContext()
{
        mCgiVariableList.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
HttpRequestContext&
HttpRequestContext::operator=(const HttpRequestContext& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;
   else
   {
      //copy mEnvironmentVars[HTTP_ENV_LAST]
      int count = 0;
      for (count = HttpRequestContext::HTTP_ENV_RAW_URL;
           count < HttpRequestContext::HTTP_ENV_LAST ;
           count ++)
           {
              if(!rhs.mEnvironmentVars[count].isNull())
              {
                 mEnvironmentVars[count].remove(0);
                 mEnvironmentVars[count].append(rhs.mEnvironmentVars[count]);
              }
           }

      // delete the old values in the UtlDList
      if(!mCgiVariableList.isEmpty())
      {
         mCgiVariableList.destroyAll();
      }
      //copy mCgiVariableList memebers individually
      mUsingInsensitive = rhs.mUsingInsensitive;
           UtlDListIterator iterator((UtlDList&)rhs.mCgiVariableList);
           NameValuePair* nameValuePair = NULL;
      UtlString value;
      UtlString name;

      int index = 0;
      do
      {
         nameValuePair = (NameValuePair*)iterator();
         if(nameValuePair)
         {
            name.append(*nameValuePair);
            value.append(nameValuePair->getValue());
            NameValuePair* newNvPair = ( mUsingInsensitive
                                        ? new NameValuePair(name, value)
                                        : new NameValuePairInsensitive(name, value)
                                        );
            mCgiVariableList.insertAt(index, newNvPair);
            index ++;
            value.remove(0);
            name.remove(0);
         }
      }
      while (nameValuePair != NULL);
   }
   return *this;
}

void HttpRequestContext::extractPostCgiVariables(const HttpBody& body)
{
    int length;
    UtlString bodyBytes;

    body.getBytes(&bodyBytes, &length);
    parseCgiVariables(bodyBytes.data());
    bodyBytes.remove(0);
}

/* ============================ ACCESSORS ================================= */

void HttpRequestContext::getEnvironmentVariable(enum RequestEnvironmentVariables envVariable,
                            UtlString& value) const
{
    if(envVariable >= 0 && envVariable < HTTP_ENV_LAST)
    {
        value = mEnvironmentVars[envVariable];
    }
    else
    {
        value.remove(0);
    }
}

UtlBoolean HttpRequestContext::getCgiVariable(const char* name,
                                             UtlString& value,
                                             int occurance) const
{
   UtlDListIterator iterator((UtlDList&)mCgiVariableList);
   NameValuePair* nameValuePair = NULL;
   int fieldIndex = 0;
   UtlString upperCaseName;
   UtlBoolean foundName = FALSE;
   
   value.remove(0);
 
#  ifdef TEST_DEBUG
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "HttpRequestContext::getCgiVariable %p (\"%s\",<val>,%d)",
                 &mCgiVariableList, name, occurance
                 );
#  endif

   if(name)
   {
      upperCaseName.append(name);
      upperCaseName.toUpper();
   }
   NameValuePair *matchName = ( mUsingInsensitive
                               ? new NameValuePair(upperCaseName)
                               : new NameValuePairInsensitive(upperCaseName)
                               );

   // For each name value:
   for (fieldIndex = 0, nameValuePair = (NameValuePair*) iterator.findNext(matchName);
        fieldIndex < occurance;
        fieldIndex++
        )
   {
      nameValuePair = (NameValuePair*) iterator.findNext(matchName);

#     ifdef TEST_DEBUG
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "HttpRequestContext::getCgiVariable(name,val,occ) %p skipping %d '%s' -> '%s'",
                    &mCgiVariableList,
                    fieldIndex,
                    nameValuePair ? nameValuePair->data() : "UNFOUND",
                    nameValuePair ? nameValuePair->getValue() : "UNFOUND"
                    );
#     endif
   }
   delete matchName;

#  ifdef TEST_DEBUG
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "HttpRequestContext::getCgiVariable(name,val,occ) %p stopped at %d '%s' -> '%s'",
                 &mCgiVariableList, fieldIndex,
                 nameValuePair ? nameValuePair->data() : "UNFOUND",
                 nameValuePair ? nameValuePair->getValue() : "UNFOUND"
                 );
#  endif

   if(fieldIndex == occurance && nameValuePair)
   {
      value.append(nameValuePair->getValue());
      foundName = TRUE;
   }

   return(foundName);
}

UtlBoolean HttpRequestContext::getCgiVariable(int index, UtlString& name, UtlString& value) const
{
    NameValuePair* nameValuePair = NULL;
    name.remove(0);
    value.remove(0);

    if((int)(mCgiVariableList.entries()) > index && index >= 0)
    {
        nameValuePair = (NameValuePair*)mCgiVariableList.at(index);
        if(nameValuePair)
        {
            name = *nameValuePair;
            value.append(nameValuePair->getValue());
        }
    }

    return(nameValuePair != NULL);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void HttpRequestContext::parseCgiVariables(const char* queryString)
{
        parseCgiVariables(queryString, mCgiVariableList,
                "&",
                "=");
}

void HttpRequestContext::parseCgiVariables(const char* queryString,
                                           UtlList& cgiVariableList,
                                           const char* pairSeparator,
                                           const char* nameValueSeparator,
                                           UtlBoolean nameIsCaseInsensitive,
                                           UnEscapeFunction unescape)
{
   //UtlString nameAndValue;
   const char* nameAndValuePtr;
   int nameAndValueLength;
   //UtlString name;
   const char* namePtr;
   int nameLength;
   int nameValueIndex = 0;
   UtlString value;
   int lastCharIndex = 0;
   int relativeIndex;
   int nameValueRelativeIndex;
   int queryStringLength = strlen(queryString);

   do
   {
      // Pull out a name value pair
      //osPrintf("HttpRequestContext::parseCgiVariables parseCgiVariables: \"%s\" lastCharIndex: %d",
      //    &(queryString[lastCharIndex]), lastCharIndex);
      NameValueTokenizer::getSubField(&(queryString[lastCharIndex]),
                                      queryStringLength - lastCharIndex,
                                      0,
                                      pairSeparator,
                                      nameAndValuePtr,
                                      nameAndValueLength,
                                      &relativeIndex);
      lastCharIndex += relativeIndex;

      if(nameAndValuePtr && nameAndValueLength > 0)
      {
         // Separate the name and value
         NameValueTokenizer::getSubField(nameAndValuePtr,
                                         nameAndValueLength,
                                         0,
                                         nameValueSeparator,
                                         namePtr,
                                         nameLength,
                                         &nameValueRelativeIndex);

         // Get rid of leading white space in the name
         while(nameLength > 0 &&
               (*namePtr == ' ' ||
                *namePtr == '\t'))
         {
            nameLength--;
            namePtr++;
         }

         if(nameLength > 0)
         {
            // Ignore any subsequent name value separators should they exist
            //int nvSeparatorIndex = nameAndValue.index(nameValueSeparator);
            int valueSeparatorOffset = strspn(&(namePtr[nameLength]),
                                              nameValueSeparator);
            const char* valuePtr = &(namePtr[nameLength]) + valueSeparatorOffset;
            int valueLength = nameAndValueLength -
               (valuePtr - nameAndValuePtr);

            // If there is a value
            if(valueSeparatorOffset <= 0 ||
               *valuePtr == '\0' ||
               valueLength <= 0)
            {
               valuePtr = NULL;
               valueLength = 0;
            }

            // Construct the new pair of the right subclass of NameValuePair
            // to have the compareTo method we want.
            NameValuePair* newNvPair =
               nameIsCaseInsensitive ?
               new NameValuePairInsensitive("") :
               new NameValuePair("");

            newNvPair->append(namePtr, nameLength);
            if(valuePtr)
            {
               value.remove(0);
               value.append(valuePtr, valueLength);
               NameValueTokenizer::frontBackTrim(&value, " \t\n\r");
               unescape(value);
               newNvPair->setValue(value);
            }
            else
            {
               newNvPair->setValue("");
            }

            // Unescape the name.
            unescape(*newNvPair);
            NameValueTokenizer::frontBackTrim(newNvPair, " \t\n\r");

#           ifdef TEST_DEBUG
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "HttpRequestContext::parseCgiVariables adding %p '%s' -> '%s'",
                          &cgiVariableList, newNvPair->data(), newNvPair->getValue()
                          );
#           endif

            // Add the name, value pair to the list
            cgiVariableList.insert(newNvPair);

            nameValueIndex++;
         }
      }
   } while(nameAndValuePtr &&
           nameAndValueLength > 0 &&
           queryString[lastCharIndex] != '\0');
}
