// $Id: //depot/OPENDEV/sipXproxy/include/sipforkingproxy/ForwardRules.h#7 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ForwardRules_h_
#define _ForwardRules_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include "os/OsStatus.h"
#include "xmlparser/tinyxml.h"

// DEFINES
#define XML_TAG_ROUTES              "routes"
#define XML_TAG_ROUTEMATCH          "route"
#define XML_ATT_MAPPINGTYPE         "mappingType"
#define XML_TAG_ROUTEFROM           "routeFrom"
#define XML_TAG_METHODMATCH         "methodMatch"
#define XML_TAG_METHODPATTERN       "methodPattern"
#define XML_TAG_FIELDMATCH          "fieldMatch"
#define XML_ATT_FIELDNAME           "fieldName"
#define XML_TAG_FIELDPATTERN        "fieldPattern"
#define XML_TAG_ROUTETO             "routeTo"

class TiXmlNode;
class Url;
class SipMessage;

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class ForwardRules 
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
	
/* ============================ CREATORS ================================== */

   ForwardRules();

   virtual ~ForwardRules();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */
   OsStatus loadMappings(const OsString configFileName,
                         const OsString mediaserver = "",
                         const OsString& voicemail = "",
                         const OsString& localhost = "");
   
   void buildDefaultRules(const char* domain,
                         const char* hostname,
                         const char* ipAddress,
                         const char* fqhn,
                         int localPort);

   static void buildDefaultRules(const char* domain,
                                 const char* hostname,
                                 const char* ipAddress,
                                 const char* fqhn,
                                 int localPort,
                                 TiXmlDocument& xmlDoc);

   OsStatus getRoute(const Url& requestUri,
      const SipMessage& request,
      OsString& RouteToString,
      OsString& mappingType);
   
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   TiXmlDocument *mDoc;
   OsString mVoicemail;
   OsString mLocalhost;
   OsString mMediaServer;

   OsStatus parseRouteMatchContainer(const Url& requestUri,
      const SipMessage& request,
      OsString& RouteToString,
      OsString& mappingType,
      TiXmlNode* routesNode,
      TiXmlNode* previousRouteMatchNode = NULL);
   
   OsStatus parseMethodMatchContainer(const SipMessage& request,
      OsString& RouteToString,
      TiXmlNode* routeMatchNode,
      TiXmlNode* previousMethodMatchNode = NULL);

   OsStatus parseFieldMatchContainer(const SipMessage& request,
      OsString& RouteToString,
      TiXmlNode* methodMatchNode,
      TiXmlNode* previousFieldMatchNode = NULL);

   OsStatus getRouteTo(OsString& RouteToString,
      TiXmlNode* fieldMatchNode);


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   
/*////////////////////////////////////////////////////////////////////////// */
};

/* ============================ INLINE METHODS ============================ */

#endif  // _ForwardRules_h_
