// 
// 
// Copyright (C) 2005 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <utl/UtlInt.h>
#include <utl/UtlBool.h>
#include <utl/UtlDateTime.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlHashMapIterator.h>
#include <net/Url.h>
#include <net/XmlRpcRequest.h>
#include <net/XmlRpcResponse.h>
#include <net/XmlRpcDispatch.h>


class AddExtension : public XmlRpcMethod
{
public:

   /// Get the instance of this method.
   static AddExtension* get()
      {
         if ( spInstance == NULL )
         {
            spInstance = new AddExtension();
         }

         return spInstance;   
      };

   /// Destructor
   virtual ~AddExtension()
      {
      };

   bool addParam(int index, UtlContainable* value, XmlRpcResponse& faultResponse)
      {
#ifdef PRINT_OUT
         printf("index = %d\n", index);
         UtlString paramType(value->getContainableType());
         if (paramType.compareTo("UtlInt") == 0)
         {
            UtlInt* paramValue = (UtlInt *)value;
            printf("value = %d\n", paramValue->getValue());
         }
         
         if (paramType.compareTo("UtlString") == 0)
         {
            UtlString* paramValue = (UtlString *)value;
            printf("value = %s\n", paramValue->data());
         }

         if (paramType.compareTo("UtlSList") == 0)
         {
            UtlSList* list = (UtlSList *)value;
            UtlSListIterator iterator(*list);
            UtlContainable* pObject;
            while(pObject = iterator())
            {
               UtlString elementType(pObject->getContainableType());
               if (elementType.compareTo("UtlInt") == 0)
               {
                  UtlInt* paramValue = (UtlInt *)pObject;
                  printf("value = %d\n", paramValue->getValue());
               }
               
               if (elementType.compareTo("UtlString") == 0)
               {
                  UtlString* paramValue = (UtlString *)pObject;
                  printf("value = %s\n", paramValue->data());
               }
            }
         }

         if (paramType.compareTo("UtlHashMap") == 0)
         {
            UtlHashMap* map = (UtlHashMap *)value;
            UtlHashMapIterator iterator(*map);
            UtlString* pName;
            while(pName = (UtlString *)iterator())
            {
               printf("name = %s\n", pName->data());
               
               UtlContainable* pObject = map->findValue(pName);
               UtlString elementType(pObject->getContainableType());
               if (elementType.compareTo("UtlInt") == 0)
               {
                  UtlInt* paramValue = (UtlInt *)pObject;
                  printf("value = %d\n", paramValue->getValue());
               }
               
               if (elementType.compareTo("UtlString") == 0)
               {
                  UtlString* paramValue = (UtlString *)pObject;
                  printf("value = %s\n", paramValue->data());
               }
               
               if (elementType.compareTo("UtlSList") == 0)
               {
                  UtlSList* list = (UtlSList *)pObject;
                  UtlSListIterator iterator(*list);
                  UtlContainable* pList;
                  while(pList = iterator())
                  {
                     UtlString elementType(pList->getContainableType());
                     if (elementType.compareTo("UtlInt") == 0)
                     {
                        UtlInt* paramValue = (UtlInt *)pList;
                        printf("value = %d\n", paramValue->getValue());
                     }
                     
                     if (elementType.compareTo("UtlString") == 0)
                     {
                        UtlString* paramValue = (UtlString *)pList;
                        printf("value = %s\n", paramValue->data());
                     }
                  }
               }
            }
         }
#endif         
         return true;
      };

   bool execute(const HttpRequestContext& context, XmlRpcResponse& response, XmlRpcMethod::ExecutionStatus status)
      {
         status = XmlRpcMethod::OK;
         UtlString responseText("method call \"AddExtension\" successful");
         response.setResponse(&responseText);
         return true;
      };

private:
   AddExtension() {};

   static AddExtension* spInstance;
};

AddExtension* AddExtension::spInstance = 0;

/**
 * Unit test for XmlRpc
 */
class XmlRpcTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(XmlRpcTest);
   CPPUNIT_TEST(testXmlRpcRequestCreation);
   CPPUNIT_TEST(testXmlRpcRequestParse);
   CPPUNIT_TEST(testXmlRpcResponseParse);
   CPPUNIT_TEST_SUITE_END();

public:

   void testXmlRpcRequestCreation()
      {
         const char *ref =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodCall>\n"
            "<methodName>addExtension</methodName>\n"
            "<params>\n"
            "<param>\n"
            "<value><string>acd@pingtel.com</string></value>\n"
            "</param>\n"
            "<param>\n"
            "<value><int>162</int></value>\n"
            "</param>\n"
            "<param>\n"
            "<value>\n"
            "<array>\n"
            "<data>\n"
            "<value><string>160@pingtel.com</string></value>\n"
            "<value><string>167@pingtel.com</string></value>\n"
            "<value><int>1000</int></value>\n"
            "<value><boolean>1</boolean></value>\n"
            "</data>\n"
            "</array>\n"
            "</value>\n"
            "</param>\n"
            "<param>\n"
            "<value>\n"
            "<struct>\n"
            "<member>\n"
            "<name>acd@pingtel.com</name>\n"
            "<value>\n"
            "<array>\n"
            "<data>\n"
            "<value><string>160@pingtel.com</string></value>\n"
            "<value><string>167@pingtel.com</string></value>\n"
            "<value><int>1000</int></value>\n"
            "<value><boolean>1</boolean></value>\n"
            "</data>\n"
            "</array>\n"
            "</value>\n"
            "</member>\n"
            "</struct>\n"
            "</value>\n"
            "</param>\n"
            "</params>\n"
            "</methodCall>\n"
            ;

         Url url;
         XmlRpcRequest request(url, "addExtension");

         UtlString stringValue("acd@pingtel.com");
         request.addParam(&stringValue);

         UtlInt intValue(162);
         request.addParam(&intValue);
         
         UtlSList list;
         UtlString array1("160@pingtel.com");
         list.insert(&array1);
         UtlString array2("167@pingtel.com");
         list.insert(&array2);
         UtlInt array3(1000);
         list.insert(&array3);
         UtlBool array4(true);
         list.insert(&array4);
         request.addParam(&list);
         
         UtlHashMap members;
         members.insertKeyAndValue(&stringValue, &list);
         request.addParam(&members);         

         XmlRpcResponse response;
         request.execute(response);

         UtlString requestBody;
         int length;
         request.mpRequestBody->getBytes(&requestBody, &length);
         //printf("body = \n%s\n", requestBody.data()); 

         CPPUNIT_ASSERT(strcmp(requestBody.data(), ref) == 0);
      }


   void testXmlRpcRequestParse()
      {
         const char *ref =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodCall>\n"
            "<methodName>addExtension</methodName>\n"
            "<params>\n"
            "<param>\n"
            "<value><string>acd@pingtel.com</string></value>\n"
            "</param>\n"
            "<param>\n"
            "<value><int>162</int></value>\n"
            "</param>\n"
            "<param>\n"
            "<value>\n"
            "<array>\n"
            "<data>\n"
            "<value><string>160@pingtel.com</string></value>\n"
            "<value><string>167@pingtel.com</string></value>\n"
            "<value><int>1000</int></value>\n"
            "<value><boolean>1</boolean></value>\n"
            "</data>\n"
            "</array>\n"
            "</value>\n"
            "</param>\n"
            "<param>\n"
            "<value>\n"
            "<struct>\n"
            "<member>\n"
            "<name>acd@pingtel.com</name>\n"
            "<value>\n"
            "<array>\n"
            "<data>\n"
            "<value><string>160@pingtel.com</string></value>\n"
            "<value><string>167@pingtel.com</string></value>\n"
            "<value><int>1000</int></value>\n"
            "<value><boolean>1</boolean></value>\n"
            "</data>\n"
            "</array>\n"
            "</value>\n"
            "</member>\n"
            "</struct>\n"
            "</value>\n"
            "</param>\n"
            "</params>\n"
            "</methodCall>\n"
            ;

         const char *faultResponse =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodResponse>\n"
            "<fault>\n"
            "<value>\n"
            "<struct>\n"
            "<member>\n"
            "<name>faultCode</name>\n"
            "<value><int>-3</int></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>faultString</name>\n"
            "<value><string>Method has not been registered</string></value>\n"
            "</member>\n"
            "</struct>\n"
            "</value>\n"
            "</fault>\n"
            "</methodResponse>\n"
            ;

         const char *successResponse =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodResponse>\n"
            "<params>\n"
            "<param>\n"
            "<value><string>method call \"AddExtension\" successful</string></value>\n"
            "</param>\n"
            "</params>\n"
            "</methodResponse>\n"
            ;

         XmlRpcDispatch dispatch(8200, false, "/RPC2");

         UtlString requestContent(ref);
         XmlRpcResponse response;
         XmlRpcMethod* method;

         bool result = dispatch.parseXmlRpcRequest(requestContent, method, response);
         CPPUNIT_ASSERT(result == false);

         XmlRpcBody *responseBody = response.getBody();

         UtlString body;
         int length;
         responseBody->getBytes(&body, &length);

         CPPUNIT_ASSERT(strcmp(body.data(), faultResponse) == 0);

         XmlRpcResponse newResponse;
         dispatch.addMethod("addExtension", (XmlRpcMethod::Get *)AddExtension::get);
         result = dispatch.parseXmlRpcRequest(requestContent, method, newResponse);
         CPPUNIT_ASSERT(result == true);
         
         HttpRequestContext context;
         XmlRpcMethod::ExecutionStatus status = XmlRpcMethod::OK;
         AddExtension* pAddExtension = AddExtension::get();
         pAddExtension->execute(context, newResponse, status);

         responseBody = newResponse.getBody();

         responseBody->getBytes(&body, &length);
         //printf("body = \n%s\n", body.data());

         CPPUNIT_ASSERT(strcmp(body.data(), successResponse) == 0);
      }


   void testXmlRpcResponseParse()
      {
         const char *faultResponse =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodResponse>\n"
            "<fault>\n"
            "<value>\n"
            "<struct>\n"
            "<member>\n"
            "<name>faultCode</name>\n"
            "<value><int>-3</int></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>faultString</name>\n"
            "<value><string>Method has not been registered</string></value>\n"
            "</member>\n"
            "</struct>\n"
            "</value>\n"
            "</fault>\n"
            "</methodResponse>\n"
            ;

         const char *successResponse1 =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodResponse>\n"
            "<params>\n"
            "<param>\n"
            "<value><string>method call \"AddExtension\" successful</string></value>\n"
            "</param>\n"
            "</params>\n"
            "</methodResponse>\n"
            ;

         const char *successResponse2 =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodResponse>\n"
            "<params>\n"
            "<param>\n"
            "<value>\n"
            "<struct>\n"
            "<member>\n"
            "<name>acd@pingtel.com</name>\n"
            "<value>\n"
            "<array>\n"
            "<data>\n"
            "<value><string>160@pingtel.com</string></value>\n"
            "<value><string>167@pingtel.com</string></value>\n"
            "<value><int>1000</int></value>\n"
            "<value><boolean>1</boolean></value>\n"
            "</data>\n"
            "</array>\n"
            "</value>\n"
            "</member>\n"
            "</struct>\n"
            "</value>\n"
            "</param>\n"
            "</params>\n"
            "</methodResponse>\n"
            ;

         UtlString faultContent(faultResponse);
         XmlRpcResponse response;

         bool result = response.parseXmlRpcResponse(faultContent);
         CPPUNIT_ASSERT(result == false);

         int faultCode;
         UtlString faultString;
         response.getFault(&faultCode, faultString);
         CPPUNIT_ASSERT_EQUAL_MESSAGE("faultCode is not the same",
                                      faultCode, -3);

         CPPUNIT_ASSERT(strcmp(faultString.data(), "Method has not been registered") == 0);

         UtlString successContent1(successResponse1);

         result = response.parseXmlRpcResponse(successContent1);
         CPPUNIT_ASSERT(result == true);

         UtlContainable *containable;
         response.getResponse(containable);
         UtlString* responseString = (UtlString *)containable;

         CPPUNIT_ASSERT(strcmp(responseString->data(), "method call \"AddExtension\" successful") == 0);

         UtlString successContent2(successResponse2);

         result = response.parseXmlRpcResponse(successContent2);
         CPPUNIT_ASSERT(result == true);
      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(XmlRpcTest);
