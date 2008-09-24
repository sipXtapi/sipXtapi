// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
//
//////////////////////////////////////////////////////////////////////////////
// Author: Mike Cohen
// 
// SYSTEM INCLUDES
// APPLICATION INCLUDES
#if defined(_WIN32)
#       include "resparse/wnt/sysdep.h"
#       include <resparse/wnt/netinet/in.h>
#       include <resparse/wnt/arpa/nameser.h>
#       include <resparse/wnt/resolv/resolv.h>
#       include <winsock2.h>
extern "C" {
#       include "resparse/wnt/inet_aton.h"       
}
#elif defined(_VXWORKS)
#       include <stdio.h>
#       include <netdb.h>
#       include <netinet/in.h>
#       include <vxWorks.h>
/* Use local lnameser.h for info missing from VxWorks version --GAT */
/* lnameser.h is a subset of resparse/wnt/arpa/nameser.h                */
#       include <resolv/nameser.h>
#       include <resparse/vxw/arpa/lnameser.h>
/* Use local lresolv.h for info missing from VxWorks version --GAT */
/* lresolv.h is a subset of resparse/wnt/resolv/resolv.h               */
#       include <resolv/resolv.h>
#       include <resparse/vxw/resolv/lresolv.h>
/* #include <sys/socket.h> used sockLib.h instead --GAT */
#       include <sockLib.h>
#       include <resolvLib.h>
#elif defined(__pingtel_on_posix__)
#       include <arpa/inet.h>
#       include <netinet/in.h>
#       include <sys/socket.h>
#       include <resolv.h>
#       include <netdb.h>
#else
#       error Unsupported target platform.
#endif

#ifndef __pingtel_on_posix__
extern struct __res_state _sip_res;
#endif
#ifdef WINCE
#   include <types.h>
#else
#   include <sys/types.h>
#endif
// Standard C includes.
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "net/SipSrvLookup.h"
#include "net/NetTurnSelector.h"
#include "os/OsDateTime.h"
#include "os/OsNatDatagramSocket.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif

//////////////////////////////////////////////////////////////////////////////
NetTurnSelector::NetTurnSelector() :
    mTransferTestPeriodSeconds(3),
    mTransferTestThroughputKBS(64)
{
}

//////////////////////////////////////////////////////////////////////////////
NetTurnSelector::~NetTurnSelector()
{
}

//////////////////////////////////////////////////////////////////////////////
bool NetTurnSelector::selectBestTurnServer(OsHostPort& selectedServer,
                                           int& numAvailable,
                                           UtlString* domainName[],
                                           int ports[],
                                           const int numDomains,
                                           const char* szInterfaceForOutgoingDns,
                                           const char* szUsername,
                                           int& selectedRtt,
                                           int& selectedBandwidth,
                                           double& selectedPacketLossPercent,
                                           UtlSList& results) const
{
    OsHostPort* arrCandidates[MAX_TURN_SERVERS];
    OsSocket::IpProtocolSocketType transport = OsSocket::UDP;
    bool bRet = false;

    server_t *server_list;
    int numServers = 0;
    for (int i = 0; i < numDomains; i++)
    {
        server_list = SipSrvLookup::servers(domainName[i]->data(),
                                            "turn",
                                            transport,
                                            ports[i],
                                            szInterfaceForOutgoingDns);
        for (int j=0; server_list[j].isValidServerT(); j++)
        {
            UtlString ip_addr;
            server_list[j].getIpAddressFromServerT(ip_addr);
            arrCandidates[numServers] = new OsHostPort(ip_addr, server_list[j].getPortFromServerT());
            numServers++;
        }
    }

    numAvailable = numServers;
    if (numServers >= 1)
    {
        const OsHostPort* pSelected = selectBestTurnServer(arrCandidates,
                                         numServers,
                                         szUsername,
                                         selectedRtt,
                                         selectedBandwidth,
                                         selectedPacketLossPercent,
                                         results);
        if (pSelected)
        {
            selectedServer = *pSelected;
        }
        if (numServers > 1)
        {
            bRet = true;
        }
    }
    for (int i = 0; i < numServers; i++)
    {
        delete arrCandidates[i];
    }
    return bRet;
}
//////////////////////////////////////////////////////////////////////////////
const OsHostPort* const NetTurnSelector::selectBestTurnServer(OsHostPort* const arrHostPorts[],
                                                       const int numHostPorts,
                                                       const char* szUsername,
                                                       int& selectedRtt,
                                                       int& selectedBandwidth,
                                                       double& selectedPacketLossPercent,
                                                       UtlSList& results) const
{
    int bestIndex = 0;
    int rtt = -1;
    int bestRtt = -1;
    const OsHostPort* pSelectedHostPort = NULL;

    selectedBandwidth = -1 ;

    // serially check for rtt
    for (int i = 0; i < numHostPorts; i++)
    {
        OsNatDatagramSocket* pSocket = (OsNatDatagramSocket*)getRtt(*arrHostPorts[i], rtt);
        if (rtt > -1)
        {
//            int tempBandwidth = -1;
//            double tempPacketLossPercent = -1.0;
//            getBandwidth(*arrHostPorts[i],
//                szUsername,
//                tempBandwidth);


            int jitterMin, jitterMax, jitterAvg;
            double tempPacketLossPercent = -1.0;

//            if (getPacketLossPercent(*arrHostPorts[i],
//                    szUsername,
//                    tempPacketLossPercent,
//                    jitterMin,
//                    jitterMax,
//                    jitterAvg))
//            {
                char szResults[256];
                snprintf(szResults,
                    sizeof(szResults),
//                    "Server: %s\t rtt=%d\t packetLoss%%=%2.3lf\t Jitter Avg %d\t Min %d\t Max %d", 
                    "Server: %s\t rtt=%d", 
                    arrHostPorts[i]->getHost().data(), 
                    rtt);
//                    tempPacketLossPercent,
//                    jitterAvg,
//                    jitterMin,
//                    jitterMax
                results.append(new UtlString(szResults));
//            }
//            else
//            {
//                char szResults[256];
//                snprintf(szResults,
//                    sizeof(szResults),
//                    "Server: %s\t rtt=%d\t Turn failed.", 
//                    arrHostPorts[i]->getHost().data(), 
//                    rtt);
//                results.append(new UtlString(szResults));
//            }

            if (rtt < bestRtt || -1 == bestRtt)
            {
                bestRtt = rtt;
                bestIndex = i;
                pSelectedHostPort = arrHostPorts[bestIndex];
//                selectedBandwidth = tempBandwidth;
                selectedPacketLossPercent = tempPacketLossPercent;
            }
        }
        else
        {
            char szResults[256];
            snprintf(szResults,
                sizeof(szResults),
                "Server: %s\t %s", 
                arrHostPorts[i]->getHost().data(), 
                "No STUN response"
                );
            results.append(new UtlString(szResults));
        }
    }
    selectedRtt = bestRtt;
    return pSelectedHostPort;
}

//////////////////////////////////////////////////////////////////////////////
const bool NetTurnSelector::getRtt(const OsHostPort& turnServer,
                            int& roundTripTime) const
{
    bool bSuccess = false;
    OsNatDatagramSocket* pSocket = new  OsNatDatagramSocket(0, NULL) ;

    roundTripTime = -1;
    
    OsTime start;
    OsDateTime::getCurTimeSinceBoot(start);
    pSocket->enableStun(turnServer.getHost().data(), turnServer.getPort(), 0, 0, true) ;
    OsTime end;
    OsDateTime::getCurTimeSinceBoot(end);
    pSocket->setTransparentStunRead(false) ;
    
    UtlString reflectiveIp;
    int       reflectivePort;

    if (pSocket->getMappedIp(&reflectiveIp, &reflectivePort))
    {
        bSuccess = true;
        roundTripTime = end.cvtToMsecs() - start.cvtToMsecs();
    }
    else
    {
    }
    pSocket->close();
    delete pSocket;

    return bSuccess;
}

//////////////////////////////////////////////////////////////////////////////
const bool NetTurnSelector::getBandwidth(const OsHostPort& turnServer,
                                        const char* szUsername,
                                        int& bandwidth) const
{
    bool bSuccess = false;

    bandwidth = -1;
    return bSuccess;
}

//////////////////////////////////////////////////////////////////////////////
const bool NetTurnSelector::getPacketLossPercent(const OsHostPort& turnServer,
                                                const char* szUsername,
                                                double& packetLossPercent,
                                                int& jitterMin,
                                                int& jitterMax,
                                                int& jitterAvg) const
{
    bool bSuccess = false;
    packetLossPercent = -1.0;

    OsNatDatagramSocket* pSenderSocket = new  OsNatDatagramSocket(0, NULL) ;
    OsNatDatagramSocket* pReceiverSocket = new  OsNatDatagramSocket(0, NULL) ;

    UtlString reflectiveIp;
    int       reflectivePort;

    pReceiverSocket->enableStun(turnServer.getHost().data(), turnServer.getPort(), 0, 0, true);
    if (pReceiverSocket->getMappedIp(&reflectiveIp, &reflectivePort))
    {
        pSenderSocket->enableTurn(turnServer.getHost().data(),
            turnServer.getPort(),
            300,
            szUsername,
            "",
            true);

        UtlString relayIp;
        int       relayPort;
        if (pSenderSocket->getTurnIp(&relayIp, &relayPort))
        {
            // create a receiver, and start it
            OsTurnReceiver turnReceiver(pReceiverSocket,
                                        mTransferTestPeriodSeconds*2,
                                        getTransferSize());
            turnReceiver.start();

            // create the sender net task and start it
            NetTurnSenderNetTask turnSenderNetTask(pSenderSocket);
            turnSenderNetTask.start();

            pSenderSocket->primeTurnDestination(reflectiveIp, reflectivePort);
            OsTask::delay(200);
            pSenderSocket->setTurnDestination(reflectiveIp, reflectivePort);

            // create a sender, and start it
            OsTurnSender turnSender(pSenderSocket,
                                    getTransferSize());
            turnSender.start();

            // wait both receiver and sender are done
            while (!turnSender.isDone() || !turnReceiver.isDone())
            {
                OsTask::delay(10);
            }
            int bytesRead = turnReceiver.getBytesRead();
            packetLossPercent = 100.0 * 
                ((getTransferSize() - (double)bytesRead) / getTransferSize() );
            pSenderSocket->close();
            while (!turnSenderNetTask.isDone())
            {
                OsTask::delay(10);
            }
            delete pSenderSocket;
            
            pReceiverSocket->close();
            delete pReceiverSocket;

            jitterMin = turnReceiver.getJitterMin();
            jitterMax = turnReceiver.getJitterMax();
            jitterAvg = turnReceiver.getJitterAvg();

            bSuccess = true;
        }
        else
        {
            packetLossPercent = 100.0 ;
            jitterMin = -1 ;
            jitterMax = -1 ;
            jitterAvg = -1 ;

            pSenderSocket->close();
            delete pSenderSocket;
            pReceiverSocket->close();
            delete pReceiverSocket;
        }
    }
    else
    {
        packetLossPercent = 100.0 ;
        jitterMin = -1 ;
        jitterMax = -1 ;
        jitterAvg = -1 ;

        pSenderSocket->close();

        delete pSenderSocket;
        pReceiverSocket->close();
        delete pReceiverSocket;
    }

    return bSuccess;
}


//////////////////////////////////////////////////////////////////////////////
const double NetTurnSelector::getTransferSize() const
{
    double transferSize = ((double)mTransferTestPeriodSeconds * (double)mTransferTestThroughputKBS / 8.0) * 1000.0;
    return transferSize;
}

//////////////////////////////////////////////////////////////////////////////
NetTurnSenderNetTask::NetTurnSenderNetTask(void* pArg) :
    NetTurnTask("NetTurnSenderNetTask %d",
           pArg)
{
}

//////////////////////////////////////////////////////////////////////////////
int NetTurnSenderNetTask::run(void* pArg)
{
    mbDone = false;
    OsNatDatagramSocket* pSocket = (OsNatDatagramSocket*) pArg;

    char buff[20480];
    memset(buff, 0, sizeof(buff));

    pSocket->setTransparentStunRead(false);
    int rc = 0;
    while (pSocket->isOk() && rc >= 0)
    {
        rc = pSocket->read(buff, sizeof(buff), 10);
    }
    mbDone = true;
    return 0;
}


//////////////////////////////////////////////////////////////////////////////
OsTurnSender::OsTurnSender(void* pArg,
                           int transferSize) :
    NetTurnTask("OsTurnSender %d",
           pArg),
    mTransferSize(transferSize)
{
}

//////////////////////////////////////////////////////////////////////////////
int OsTurnSender::run(void* pArg)
{
    mbDone = false;
    OsNatDatagramSocket* pSocket = (OsNatDatagramSocket*) pArg;

    // send 200 bytes every 25 msecs
    // that's 24000 bytes every 3 second,
    // or, 64 kilobits per second
    char buff[200];
    memset(buff, 0xDD, sizeof(buff));

    int numWrites = mTransferSize / 200;
    for (int i = 0; i < numWrites; i++)
    {
        pSocket->write(buff, 200);
        OsTask::delay(25);
    }

    mbDone = true;
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
OsTurnReceiver::OsTurnReceiver(void* pArg,
                               int timeout,
                               int transferSize) :
    NetTurnTask("OsTurnReceiver %d",
        pArg),
    mTimeout(timeout),
    mTransferSize(transferSize),
    mJitterMin(-1),
    mJitterMax(0),
    mJitterAvg(0),
    mJitterCount(0)
{
}

//////////////////////////////////////////////////////////////////////////////
int OsTurnReceiver::run(void* pArg)
{
    char szBuff[2048];
    mbDone = false;

    OsTime start;
    OsDateTime::getCurTimeSinceBoot(start);

    OsTime now;
    OsTime jitterNow;
    OsDateTime::getCurTimeSinceBoot(jitterNow);
    memset(szBuff, 0, sizeof(szBuff));
    OsNatDatagramSocket* pSocket = (OsNatDatagramSocket*) pArg;
    mBytesRead = 0;
    int bytesRead = 0;
    pSocket->setTransparentStunRead(false);
    while (mBytesRead < mTransferSize)
    {
        bytesRead = pSocket->read(szBuff, sizeof(szBuff), 200);
        if (bytesRead == 200)
        {
            mBytesRead += bytesRead;
            OsTime last = jitterNow;
            OsDateTime::getCurTimeSinceBoot(jitterNow);
            mJitterCount++;
            int diff = abs(25 - abs(jitterNow.cvtToMsecs() - last.cvtToMsecs()));
            if (mJitterCount == 1)
            {
                continue;
            }
            mJitterMax = MAX(diff, mJitterMax);
            if (mJitterMin == -1)
            {
                mJitterMin = diff;
            }
            mJitterMin = MIN(mJitterMin, diff);
            mJitterAvg += diff;
        }
        OsDateTime::getCurTimeSinceBoot(now);
        if (now.cvtToMsecs() - start.cvtToMsecs() > (mTimeout*1000))
        {
            break;
        }
    }
    mJitterCount--;
    mJitterAvg = mJitterAvg / mJitterCount;
    mbDone = true;
    return 0;
}

