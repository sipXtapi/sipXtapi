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

// APPLICATION INCLUDES
#include <net/SipUdpServer.h>
#include <net/SipUserAgent.h>
#include <net/Url.h>
#include <os/OsDateTime.h>

#if defined(_VXWORKS)
#   include <socket.h>
#   include <resolvLib.h>
//#   include <netinet/ip.h>
#elif defined(__pingtel_on_posix__)
//#   include <netinet/tcp.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipUdpServer::SipUdpServer(int port, 
                           SipUserAgent* userAgent,
                           const char* natPingUrl,
                           int natPingFrequencySeconds,
                           const char* natPingMethod,
                           int udpReadBufferSize) :
   SipProtocolServerBase(userAgent, "UDP", "SipUdpServer-%d")
{
   serverPort = port;
   serverSocket = new OsDatagramSocket(0, NULL, port, NULL);

   int sockbufsize = 0;
   int size = sizeof(int);
   getsockopt(serverSocket->getSocketDescriptor(), 
              SOL_SOCKET, 
              SO_RCVBUF, 
              (char*)&sockbufsize, 
#if defined(__pingtel_on_posix__) 
              (socklen_t*) // caste
#endif
              &size);
#ifdef TEST_PRINT
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipUdpServer::SipUdpServer UDP buffer size: %d size: %d\n",
                sockbufsize, size);
#endif

    if(udpReadBufferSize > 0)
    {
        setsockopt(serverSocket->getSocketDescriptor(), 
        SOL_SOCKET, 
        SO_RCVBUF, 
        (char*)&udpReadBufferSize, 
//#if defined(__pingtel_on_posix__) 
//        (socklen_t*) // caste
//#endif
        sizeof(int));

        getsockopt(serverSocket->getSocketDescriptor(), 
        SOL_SOCKET, 
        SO_RCVBUF, 
        (char*)&sockbufsize, 
#if defined(__pingtel_on_posix__) 
        (socklen_t*) // caste
#endif
        &size);
#ifdef TEST_PRINT
    OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipUdpServer::SipUdpServer reset UDP buffer size: %d size: %d\n",
                sockbufsize, size);
#endif
    }

   server = NULL;

   if(natPingUrl && *natPingUrl) 
       mNatPingUrl = natPingUrl;
   if(natPingMethod && *natPingMethod)
       mNatPingMethod = natPingMethod;
   else
       mNatPingMethod = "PING";

   mNatPingFrequencySeconds = natPingFrequencySeconds;
}

// Copy constructor
SipUdpServer::SipUdpServer(const SipUdpServer& rSipUdpServer) :
	SipProtocolServerBase(NULL, "UDP", "SipUdpServer-%d")
{
}

// Destructor
SipUdpServer::~SipUdpServer()
{
	if(server)
	{
		delete server;
		server = NULL;
	}
}

/* ============================ MANIPULATORS ============================== */

int SipUdpServer::run(void* runArg)
{
    int cseq = 1;
    if(mSipUserAgent)
    {
        UtlString contact;
        mSipUserAgent->getContactUri(&contact);

        // Add a tag to the contact and build the from field
        UtlString from(contact);
        int tagRand1 = rand();
        int tagRand2 = rand();
        char fromTag[80];
        sprintf(fromTag, ";tag=%d%d", tagRand1, tagRand2);
        from.append(fromTag);

        UtlString rawAddress;
        int port;
        Url pingUrl(mNatPingUrl);

        // Create a cannonized version of the ping URL in case
        // it does not specify "sip:", etc.
        UtlString cannonizedPingUrl = pingUrl.toString();

        // Get the address and port in the png URL so that 
        // we can look up the DNS stuff if needed
        port = pingUrl.getHostPort();
        pingUrl.getHostAddress(rawAddress);

        // Resolve the raw address from a DNS SRV, A record
        // to an IP address
        server_t* dnsSrvRecords = 
            SipSrvLookup::servers(rawAddress.data(),
                                  "sip",
                                  OsSocket::UDP,
                                  port);

        // Do a DNS SRV or A record lookup
        // If we started with an IP address, we will still get an IP
        // address in the result
        UtlString address;
        if(SipSrvLookup::isValidServerT(*dnsSrvRecords))
        {
            // Get the highest priority address and port from the
            // list with randomization of those according to the
            // weights.
            // Note: we are not doing any failover here as that is
            // a little tricky with the NAT stuff.  We cannot change
            // addresses with every transaction as we may get different
            // ports and addresses every time we send a ping.  For now
            // we do one DNS SRV lookup at the begining of time and
            // stick to that result.
            SipSrvLookup::getIpAddressFromServerT(*dnsSrvRecords, address);
            port = SipSrvLookup::getPortFromServerT(*dnsSrvRecords);

            // If the ping URL or DNS SRV did not specify a port
            // bind it to the default port.
            if(port <= 0) port = SIP_PORT;
        }

        // Did not get a valid response from the DNS lookup
        else
        {
            // Configured with a bad DNS name that did not resolve.
            // Or the DNS server did not respond.
            if(!rawAddress.isNull())
            {
                OsSysLog::add(FAC_SIP, PRI_INFO, 
                    "SipUdpServer::run DNS lookup failed for ping host: %s in URI: %s",
                    rawAddress.data(), mNatPingUrl.data());
            }
            // Else no ping address, this means we are not supposed to
            // do a ping
        }

        // Get the address to be used in the callId scoping
        int dummyPort;
        UtlString callId;
        mSipUserAgent->getViaInfo(OsSocket::UDP, callId, dummyPort);

        // Make up a call Id
        long epochTime = OsDateTime::getSecsSinceEpoch();
        int randNum = rand();
        char callIdPrefix[80];
        sprintf(callIdPrefix, "%ld%d-ping@", epochTime, randNum);
        callId.insert(0,callIdPrefix);

        while(mNatPingFrequencySeconds > 0 && 
            !mNatPingUrl.isNull() &&
            !mNatPingMethod.isNull() &&
            !address.isNull())
        {
            // Send a no-op SIP message to the
            // server to keep a port open through a NAT
            // based firewall
            SipMessage pingMessage;
            pingMessage.setRequestData(mNatPingMethod, cannonizedPingUrl.data(), 
                from.data(), mNatPingUrl.data(), callId, cseq, contact.data());

            // Get the UDP via info from the SipUserAgent
            UtlString viaAddress;
            int viaPort;
            mSipUserAgent->getViaInfo(OsSocket::UDP, viaAddress, viaPort);
            pingMessage.addVia(viaAddress.data(), viaPort, SIP_TRANSPORT_UDP);

            // Mark the via so the receiver knows we support and want the
            // received port to be set
            pingMessage.setLastViaTag("", "rport");
            osPrintf("Sending ping to %s %d, From: %s\n", 
                address.data(), port, contact.data());

            // Send from the same UDP port that we receive from
            mSipUserAgent->sendSymmetricUdp(pingMessage, address.data(), port);

            cseq++;

            // Wait until it is time to send another ping
            delay(mNatPingFrequencySeconds * 1000);
        }
    }

    return(mNatPingFrequencySeconds);
}

UtlBoolean SipUdpServer::startListener()
{
	osPrintf("SIP Server binding to UDP port %d\n", serverPort);
	if(server == NULL)
	{
		server = new SipClient(serverSocket);
	}
	if(mSipUserAgent)
	{
		server->setUserAgent(mSipUserAgent);
	}
	server->start();

	return(TRUE);
}

void SipUdpServer::shutdownListener()
{
	server->requestShutdown();
}


UtlBoolean SipUdpServer::sendTo(const SipMessage& message,
                               const char* address,
                               int port)
{
    UtlBoolean sendOk = server->sendTo(message, address, port);
    return(sendOk);
}

OsSocket* SipUdpServer::buildClientSocket(int hostPort, const char* hostAddress)
{
    return(new OsDatagramSocket(hostPort, hostAddress, 0, NULL));
}


// Assignment operator
SipUdpServer& 
SipUdpServer::operator=(const SipUdpServer& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */

void SipUdpServer::printStatus()
{
    // The first one is always the UDP server socket
    UtlString clientNames;
    long clientTouchedTime = server->getLastTouchedTime();
    UtlBoolean clientOk = server->isOk();
    server->getClientNames(clientNames);
    osPrintf("UDP server %p last used: %ld ok: %d names: \n%s \n", 
        this, clientTouchedTime, clientOk, clientNames.data());

    SipProtocolServerBase::printStatus();
}

/* ============================ INQUIRY =================================== */
UtlBoolean SipUdpServer::isOk()
{
    return server->isOk();
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

