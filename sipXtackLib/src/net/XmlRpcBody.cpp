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

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <os/OsDateTime.h>
#include <utl/UtlInt.h>
#include <utl/UtlBool.h>
#include <utl/UtlDateTime.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/XmlContent.h>
#include <net/XmlRpcBody.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
XmlRpcBody::XmlRpcBody()
{
   mBody = XML_VERSION_1_0;
}

// Copy constructor
XmlRpcBody::XmlRpcBody(const XmlRpcBody& rXmlRpcBody)
{
}

// Destructor
XmlRpcBody::~XmlRpcBody()
{
}

/* ============================ MANIPULATORS ============================== */


// Assignment operator
XmlRpcBody&
XmlRpcBody::operator=(const XmlRpcBody& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;
}


void XmlRpcBody::append(const char* string)
{
   mBody.append(string);
}


/* ============================ ACCESSORS ================================= */

int XmlRpcBody::getLength() const
{
   return (mBody.length());
}

void XmlRpcBody::getBytes(const char** bytes, int* length) const
{
   // This version of getBytes exists so that a caller who is
   // calling this method through an HttpBody will get the right
   // thing - we fill in the mBody string and then return that.
   UtlString tempBody;
   getBytes( &tempBody, length );
   ((XmlRpcBody*)this)->mBody = tempBody.data();
   *bytes = mBody.data();
}

void XmlRpcBody::getBytes(UtlString* bytes, int* length) const
{
   *bytes = mBody;
   *length = bytes->length();
}

bool XmlRpcBody::addValue(UtlContainable* value)
{
   bool result = false;

   UtlString paramType(value->getContainableType());
   UtlString paramValue; 

   // UtlInt
   char temp[10];
   if (paramType.compareTo("UtlInt") == 0)
   {
      UtlInt* pValue = (UtlInt *)value;
      sprintf(temp, "%d", pValue->getValue());
      paramValue = BEGIN_INT + UtlString(temp) + END_INT;
      result = true;
   }
   else
   {
      // UtlBool
      if (paramType.compareTo("UtlBool") == 0)
      {
         UtlBool* pValue = (UtlBool *)value;
         if (pValue->getValue())
         {
            sprintf(temp, "1");
         }
         else
         {
            sprintf(temp, "0");
         }
               
         paramValue = BEGIN_BOOLEAN + UtlString(temp) + END_BOOLEAN;
         result = true;
      }
      else
      {
         // UtlString
         if (paramType.compareTo("UtlString") == 0)
         {
            UtlString* pValue = (UtlString *)value;
            paramValue = BEGIN_STRING + *pValue + END_STRING;
            result = true;
         }
         else
         {
            // UtlDateTime
            if (paramType.compareTo("UtlDateTime") == 0)
            {
               UtlDateTime* pTime = (UtlDateTime *)value;
               OsDateTime time;
               pTime->getTime(time);
               UtlString isoTime;
               time.getIsoTimeStringZ(isoTime);               
               paramValue = BEGIN_TIME + isoTime + END_TIME;
               result = true;
            }
            else
            {
               // UtlHashMap
               if (paramType.compareTo("UtlHashMap") == 0)
               {
                  result = addStruct((UtlHashMap *)value);
               }
               else
               {
                  // UtlSList
                  if (paramType.compareTo("UtlSList") == 0)
                  {
                     result = addArray((UtlSList *)value);
                  }
                  else
                  {
                     OsSysLog::add(FAC_SIP, PRI_WARNING,
                                   "XmlRpcBody::addValue unspported type = %s\n", paramType.data());                     
                  }                     
               }
            }
         }
      }
   }
            
   mBody.append(paramValue);
   return result;
}


bool XmlRpcBody::addArray(UtlSList* array)
{
   bool result = false;
   mBody.append(BEGIN_ARRAY);
   
   UtlSListIterator iterator(*array);
   UtlContainable* pObject;
   while (pObject = iterator())
   {
      result = addValue(pObject);
      if (!result)
      {
         break;
      }
   }
   
   mBody.append(END_ARRAY);
   return result;
}

bool XmlRpcBody::addStruct(UtlHashMap* members)
{
   bool result = false;
   mBody.append(BEGIN_STRUCT);
   
   UtlHashMapIterator iterator(*members);
   UtlString* pName;
   UtlContainable* pObject;
   UtlString structName;
   while (pName = (UtlString *)iterator())
   {
      mBody.append(BEGIN_MEMBER);

      structName = BEGIN_NAME + *pName + END_NAME;
      mBody.append(structName); 
      
      pObject = members->findValue(pName);
      result = addValue(pObject);
      if (!result)
      {
         mBody.append(END_MEMBER);
         break;
      }
      
      mBody.append(END_MEMBER);
   }
   
   mBody.append(END_STRUCT);
   return result;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */

