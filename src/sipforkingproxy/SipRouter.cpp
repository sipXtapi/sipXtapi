// $Id: //depot/OPENDEV/sipXproxy/src/sipforkingproxy/SipRouter.cpp#7 $
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

// APPLICATION INCLUDES
#include <sipforkingproxy/SipRouter.h>
#include <sipforkingproxy/ForwardRules.h>
#include <net/SipUserAgent.h>
#include <os/OsConfigDb.h>
#include <os/OsSysLog.h>

//#define TEST_PRINT 1

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipRouter::SipRouter(SipUserAgent& sipUserAgent, 
                     ForwardRules& forwardingRules,
                     OsBoolean useAuthServer,
                     const char* authServer,
                     OsBoolean shouldRecordRoute) :
   OsServerTask("SipRouter-%d", NULL, 2000)
{
   mpSipUserAgent = &sipUserAgent;
   mpForwardingRules = &forwardingRules;

   mAuthEnabled = useAuthServer;
   mAuthServer = authServer ? authServer : "";
   if(useAuthServer &&
       mAuthServer.isNull())
   {
       mAuthEnabled = FALSE;
       OsSysLog::add(FAC_SIP, PRI_WARNING, "SipRouter::SipRouter auth server enabled but not set");
       //osPrintf("WARNING SipRouter::SipRouter auth server enabled but not set\n");
   }

   // Register to get incoming requests
   OsMsgQ* queue = getMessageQueue();
   sipUserAgent.addMessageObserver(*queue,
                                   "", // All methods
                                   TRUE, // Requests,
                                   FALSE, //Responses,
                                   TRUE, //Incoming,
                                   FALSE, //OutGoing,
                                   "", //eventName,
                                   NULL, //   SipSession* pSession,
                                   NULL); //observerData)

   mShouldRecordRoute = shouldRecordRoute;
}

// Copy constructor
SipRouter::SipRouter(const SipRouter& rSipRouter)
{
}

// Destructor
SipRouter::~SipRouter()
{
}

/* ============================ MANIPULATORS ============================== */

OsBoolean SipRouter::handleMessage(OsMsg& eventMessage)
{
	int msgType = eventMessage.getMsgType();
	// int msgSubType =
        eventMessage.getMsgSubType();

	if(msgType == OsMsg::PHONE_APP)
	 //&& msgSubType == CP_SIP_MESSAGE)
	{
		SipMessage* sipRequest = (SipMessage*)((SipMessageEvent&)eventMessage).getMessage();
        int messageType = ((SipMessageEvent&)eventMessage).getMessageStatus();
		UtlString callId;

        if(messageType == SipMessageEvent::TRANSPORT_ERROR)
        {
           OsSysLog::add(FAC_SIP, PRI_ERR, "ERROR: SipRouter::handleMessage received transport error message");
           //osPrintf("ERROR: SipRouter::handleMessage received transport error message\n");
        }

		else if(sipRequest)
		{
            if(sipRequest->isResponse())
            {
               OsSysLog::add(FAC_SIP, PRI_ERR, "ERROR: SipRouter::handleMessage received response");
               //osPrintf("ERROR: SipRouter::handleMessage received response\n");
            }

            else
            {
                UtlString firstRouteUri;
                OsBoolean routeExists = sipRequest->getRouteUri(0, &firstRouteUri);
                // If there is a route header just send it on its way
                if(routeExists)
                {
#ifdef TEST_PRINT
                    osPrintf("SipRoute::handleMessage found a route\n");
#endif
                    // CHeck the URI.  If the URI has lw the
                    // previous proxy was a strict router
                    UtlString requestUri;
                    sipRequest->getRequestUri(&requestUri);
                    Url routeUrlParser(requestUri, TRUE);
                    UtlString dummyValue;
                    OsBoolean previousHopStrictRoutes = routeUrlParser.getUrlParameter("lr", dummyValue, 0);
                    OsBoolean uriIsMe = mpSipUserAgent->isMyHostAlias(routeUrlParser);
                    Url firstRouteUriUrl(firstRouteUri);
                    OsBoolean firstRouteIsMe = mpSipUserAgent->isMyHostAlias(firstRouteUriUrl);

                    // If the URI is not this server and the
                    // URI is not marked as a loose route and
                    // the first route is not this server then
                    // this server was not in the routes and we
                    // really do not know if the route set is
                    // set up as loose or strict routing.  We
                    // have to assume that it is strict routing.
                    if(!previousHopStrictRoutes &&
                       !uriIsMe &&
                       !firstRouteIsMe)
                    {
                        previousHopStrictRoutes = TRUE;
                    }

                    // If the URI does not have the loose route
                    // tag and the URI is pointed to this server
                    // we assume the previous hop strict routed
                    else if(!previousHopStrictRoutes &&
                       uriIsMe)
                    {
                        previousHopStrictRoutes = TRUE;
                    }

                    if(previousHopStrictRoutes)
                    {
#ifdef TEST_PRINT
                        osPrintf("SipRoute::handleMessage previous hop strict routed\n");
#endif
                        // If this is NOT my host and port in the URI
                        if(! uriIsMe)
                        {
                            OsSysLog::add(FAC_SIP, PRI_WARNING, "Strict route: %s not to this server",
                                requestUri.data());
                            // Put the route back on as this URI is
                            // not this server.
                            sipRequest->addRouteUri(requestUri);
                        }

                        // We have to pop the last route and
                        // put it in the URI
                        UtlString contactUri;
                        int lastRouteIndex;
                        sipRequest->getLastRouteUri(contactUri, lastRouteIndex);
#ifdef TEST_PRINT
                        osPrintf("SipRouter::handleMessage setting new URI: %s\n",
                            contactUri.data());
#endif

                        sipRequest->removeRouteUri(lastRouteIndex, &contactUri);
#ifdef TEST_PRINT
                        osPrintf("SipRouter::handleMessage route removed: %s\n",
                            contactUri.data());
#endif
                        // Put the last route in a the URI
                        Url newUri(contactUri);
                        newUri.getUri(contactUri);
                        sipRequest->changeRequestUri(contactUri);
#ifdef TEST_PRINT
                        UtlString bytes;
                        int len;
                        sipRequest->getBytes(&bytes, &len);
                        osPrintf("SipRouter: \nStricttttttttttttttttttttttt\n%s\nNowLLLLLLLLLLLLLLLLLLLLLoooooose\n",
                            bytes.data());
#endif
                    }
                    else
                    {
#ifdef TEST_PRINT
                        osPrintf("SipRoute::handleMessage previous hop loose routed\n");
#endif
                        // If this route is to me, pop it off
                        if(firstRouteIsMe)
                        {
                            // THis is a loose router pop my route off
                            UtlString dummyUri;
                            sipRequest->removeRouteUri(0, &dummyUri);
                        }
                        else
                        {
                            OsSysLog::add(FAC_SIP, PRI_WARNING, "Loose route: %s not to this server",
                                firstRouteUri.data());
                        }
                    }
                }
                

                // Check that there is no route again as we may have  
                // popped off a route if it was to this server.
                // If there is no routes left check if the URI
                // is mapped to something local
                UtlString dummyRoute;
                if(!routeExists ||
	               !sipRequest->getRouteUri(0, &dummyRoute))
                {
#ifdef TEST_PRINT
                    osPrintf("SipRoute::handleMessage no found a route\n");
#endif
                    UtlString uri;
                    sipRequest->getRequestUri(&uri);
                    Url originalUri(uri);
                    //UtlString domain;
                    //originalUri.getHostAddress(domain);
                    //int port = originalUri.getHostPort();
                    //if(port <= 0) port = SIP_PORT;
                    //char portBuf[64];
                    //sprintf(portBuf, "%d", port);
                    //domain.append(':');
                    //domain.append(portBuf);
#ifdef TEST_PRINT
                    //osPrintf("SipRouter::handleMessage uri domain: %s\n",
                    //    domain.data());
#endif

                    UtlString mappedTo;
                    UtlString routeType;
                    if(mpForwardingRules &&
                       mpForwardingRules->getRoute(originalUri, *sipRequest, 
                          mappedTo, routeType) == OS_SUCCESS)
                    {
#ifdef TEST_PRINT
                        osPrintf("SipRouter::handleMessage domain mapped to: %s\n",
                            mappedTo.data());
#endif
                        Url nextHopUrl(mappedTo);
                        // Add a loose route to the mapped server
                        nextHopUrl.setUrlParameter("lr", "");
                        UtlString routeString = nextHopUrl.toString();

                        sipRequest->addRouteUri(routeString.data());
#ifdef TEST_PRINT
                        osPrintf("SipRouter::handleMessage added route: %s\n",
                            routeString.data());
#endif
                    }

                    // We are not routing this to a server baed upon
                    // domain, therefore we need to be sure it goes
                    // through the aaa server if it exist.
                    else if(mAuthEnabled && 
                        !mAuthServer.isNull())
                    {
                        // Add a loose route
                        Url nextHopUrl(mAuthServer);
                        // Add a loose route to the mapped server
                        nextHopUrl.setUrlParameter("lr", "");
                        UtlString routeString = nextHopUrl.toString();

                        sipRequest->addRouteUri(routeString.data());
                    }

                    else
                    {
#ifdef TEST_PRINT
                        osPrintf("SipRouter::handleMessage domain not mapped\n");
#endif
                    }
                }

                // If record route is enabled
                if(mShouldRecordRoute)
                {
                    
                    UtlString uriString;
                    int port;
                    //sipRequest->getRequestUri(&uriString);
                    mpSipUserAgent->getViaInfo(OsSocket::UDP,
                              uriString,
                              port);
#ifdef TEST_PRINT
                    osPrintf("SipRouter:handleMessage Record-Route address: %s port: %d\n",
                        uriString.data(), port);
#endif
                    Url routeUrl;
                    routeUrl.setHostAddress(uriString.data());
                    routeUrl.setHostPort(port);
                    routeUrl.setUrlParameter("lr", "");
                    //routeUrl.setAngleBrackets();
                    UtlString recordRoute = routeUrl.toString();
                    sipRequest->addRecordRouteUri(recordRoute);
                }

                // Decrement max forwards
                int maxForwards;
                if(sipRequest->getMaxForwards(maxForwards))
                {
                    maxForwards--;
                }
                else
                {
                    maxForwards = mpSipUserAgent->getMaxForwards();
                }
                sipRequest->setMaxForwards(maxForwards);

                sipRequest->resetTransport();
                mpSipUserAgent->send(*sipRequest);
            }
        }
    }

    return(TRUE);
}

// Assignment operator
SipRouter& 
SipRouter::operator=(const SipRouter& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

