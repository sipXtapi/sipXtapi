//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _HttpServer_h_
#define _HttpServer_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <utl/UtlInt.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlHashBag.h>
#include <os/OsTask.h>
#include <os/OsConfigDb.h>


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class HttpMessage;
class HttpBody;
class OsServerSocket;
class HttpRequestContext;
class HttpService;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class HttpServer : public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   HttpServer(OsServerSocket *pSocket, OsConfigDb* userPasswordDb,
                       const char* realm, OsConfigDb* validIpAddressDB = NULL);
     //:Default constructor

   virtual
   ~HttpServer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

    virtual int run(void* runArg);

    // Request processors
    static void processPostFile(const HttpRequestContext& requestContext,
                                const HttpMessage& request,
                                HttpMessage*& response);
    static int doPostFile(const HttpRequestContext& requestContext,
                                const HttpMessage& request,
                                HttpMessage*& response,
                                                                UtlString& status);

    static void processFileRequest(const HttpRequestContext& requestContext,
                                const HttpMessage& request,
                                HttpMessage*& response);

    // Error request processors
    static void processNotSupportedRequest(const HttpRequestContext& requestContext,
                                const HttpMessage& request,
                                HttpMessage*& response);
    static void processFileNotFound(const HttpRequestContext& requestContext,
                                const HttpMessage& request,
                                HttpMessage*& response);

        static void processUserNotAuthorized(const HttpRequestContext& requestContext,
                                     const HttpMessage& request,
                                     HttpMessage*& response,
                                     const char* text = 0);

    static void createHtmlResponse(int responseCode, const char* responseCodeText,
                   const char* htmlBodyText, HttpMessage*& response);

    static void testCgiRequest(const HttpRequestContext& requestContext,
                                const HttpMessage& request,
                                HttpMessage*& response);

    static UtlBoolean mapUri(OsConfigDb& configDb, const char* uri, UtlString& mappedUri);

    void addUriMap(const char* fromUri, const char* toUri);

    void addRequestProcessor(const char* fileUrl, void (*requestProcessor)(const HttpRequestContext& requestContext,
                const HttpMessage& request, HttpMessage*& response));

    void addHttpService(const char* fileUrl, HttpService* service);

    void setPasswordDigest(const char* user, const char* password,
                           UtlString& userPasswordDigest);


        void setPasswordDigest(const char* user, const char* passwordDigest);
          //: Sets the password, given an already digested password.



        void getDigest(const char* user, const char* password,
                                   UtlString& userPasswordDigest) ;

        void setPasswordBasic(const char* user, const char* password);

        void removeUser(const char* user, const char* password) ;

        static void constructFileList(UtlString & indexText, UtlString uri, UtlString uriFilename) ;

        //get current http server status
        OsStatus getStatus();


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    void processRequest(const HttpMessage& request, HttpMessage*& response);

    UtlBoolean processRequestIpAddr(const UtlString& remoteIp,
       const HttpMessage& request,
       HttpMessage*& response);


    UtlBoolean isRequestAuthorized(const HttpMessage& request,
                                  HttpMessage*& response,
                                  UtlString& userId);

    void processPutRequest(const HttpRequestContext& requestContext,
                           const HttpMessage& request,
                           HttpMessage*& response);

    void getFile(const char* fileName, HttpBody*& body);

    void putFile(const char* fileName, HttpBody& body);

    UtlBoolean findRequestProcessor(const char* fileUri,
            void (*&requestProcessor)(const HttpRequestContext& requestContext,
                const HttpMessage& request, HttpMessage*& response));

    UtlBoolean findHttpService(const char* fileUri, HttpService*& service);

    void loadValidIpAddrList();
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   HttpServer(const HttpServer& rHttpServer);
     //:Copy constructor (disabled)
   HttpServer& operator=(const HttpServer& rhs);
     //:Assignment operator (disabled)
   OsStatus httpStatus;
   int mServerPort;
   OsServerSocket* mpServerSocket;
   OsConfigDb * mpUserPasswordDigestDb;
   OsConfigDb * mpUserPasswordBasicDb;
   OsConfigDb * mpValidIpAddressDB;
   OsConfigDb mUriMaps;
   OsConfigDb * mpNonceDb;
   UtlString mRealm;
   UtlHashMap mRequestProcessorMethods;
   UtlHashMap mHttpServices;

        UtlHashBag mValidIpAddrList;


};

/* ============================ INLINE METHODS ============================ */

#endif  // _HttpServer_h_
