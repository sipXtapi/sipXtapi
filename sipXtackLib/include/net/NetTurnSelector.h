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
#pragma once
#ifndef _NetTurnSelector_h_
#define _NetTurnSelector_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsNatDatagramSocket.h"
#include "os/OsHostPort.h"
#include "os/OsTask.h"
#include "utl/UtlSList.h"
#include "utl/UtlString.h"

// DEFINES
#define MAX_TURN_SERVERS 256
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//////////////////////////////////////////////////////////////////////////////
class NetTurnSelector
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    NetTurnSelector();
    virtual ~NetTurnSelector();

/* ============================ CREATORS ================================== */
/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
    bool selectBestTurnServer(OsHostPort& selectedServer,
                              int& numAvailable,
                              UtlString* domainName[],
                              int ports[],
                              const int numDomains,
                              const char* szInterfaceForOutgoingDns,
                              const char* szUsername,
                              int& selectedRtt,
                              int& selectedBandwidth,
                              double& selectedPacketLossPercent,
                              UtlSList& results) const;

    const OsHostPort* const selectBestTurnServer(OsHostPort* const arrHostPorts[],
                                                 const int numHostPorts,
                                                 const char* szUsername,
                                                 int& selectedRtt,
                                                 int& selectedBandwidth,
                                                 double& selectedPacketLossPercent,
                                                 UtlSList& results) const;
    void setTransferTestPeriodSeconds(int seconds) { mTransferTestPeriodSeconds = seconds; }
    void setTransferTestThroughputKBS(int throughput) { mTransferTestThroughputKBS = throughput; }

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    const bool getRtt(const OsHostPort& turnServer,
                                int& roundTripTime) const;
    const bool getBandwidth(const OsHostPort& turnServer,
                            const char* szUsername,
                            int& bandwidth) const;
    const bool getPacketLossPercent(const OsHostPort& turnServer,
                                    const char* szUsername,
                                    double& packetLossPercent,
                                    int& jitterMin,
                                    int& jitterMax,
                                    int& jitterAvg) const;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    const double getTransferSize() const;
    int mTransferTestPeriodSeconds;
    int mTransferTestThroughputKBS;
};

//////////////////////////////////////////////////////////////////////////////
class NetTurnTask : public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
    NetTurnTask(const char* const szName,
               void* pArg) :
        OsTask(szName, pArg),
        mbDone(false)
    {
    }
/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
    const bool isDone() const
    {
        return mbDone;
    }

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    bool mbDone;
/* //////////////////////////// PRIVATE /////////////////////////////////// */
};

//////////////////////////////////////////////////////////////////////////////
class OsTurnReceiver : public NetTurnTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
    OsTurnReceiver(void* pArg,
                   int timeout,
                   int transferSize);
/* ============================ MANIPULATORS ============================== */
    int run(void* pArg);
/* ============================ ACCESSORS ================================= */
    int getBytesRead() { return mBytesRead; }
    const int getJitterMin() const { return mJitterMin; }
    const int getJitterMax() const { return mJitterMax; }
    const int getJitterAvg() const { return mJitterAvg; }
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    int mBytesRead;
    int mTimeout;
    int mTransferSize;
    int mJitterMin;
    int mJitterMax;
    int mJitterAvg;
    int mJitterCount;
};

//////////////////////////////////////////////////////////////////////////////
class OsTurnSender : public NetTurnTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
    OsTurnSender(void* pArg,
                 int transferSize);
/* ============================ MANIPULATORS ============================== */
    int run(void* pArg);
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    int mTransferSize;
};

//////////////////////////////////////////////////////////////////////////////
class NetTurnSenderNetTask : public NetTurnTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
    NetTurnSenderNetTask(void* pArg);
/* ============================ MANIPULATORS ============================== */
    int run(void* pArg);
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};


#endif
