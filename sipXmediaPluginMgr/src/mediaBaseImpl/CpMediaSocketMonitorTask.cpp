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
// Copyright (C) 2005-2008 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "tapi/sipXtapi.h"
#include "mediaBaseImpl/CpMediaSocketMonitorTask.h"
#include "utl/UtlHashMapIterator.h"
#include "utl/UtlVoidPtr.h"
#include "os/OsLock.h"
#include "mediaInterface/IMediaSocket.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
CpMediaSocketMonitorTask* CpMediaSocketMonitorTask::spInstance = NULL;


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpMediaSocketMonitorTask::CpMediaSocketMonitorTask(const int pollingFrequency):
    OsServerTask("CpMediaSocketMonitorTask"),
    mPollingFrequency(pollingFrequency),
    mMapMutex(OsMutex::Q_FIFO)
{
    mpTimer = new OsTimer(getMessageQueue(), NULL);
    mpTimer->periodicEvery(OsTime(pollingFrequency, 0), OsTime(pollingFrequency, 0));
    
}


// Destructor
CpMediaSocketMonitorTask::~CpMediaSocketMonitorTask()
{
    mMapMutex.acquire();
    mpTimer->stop() ;
    spInstance = NULL ;
    requestShutdown();
    waitUntilShutDown();
    delete mpTimer ;    

    mSocketMap.destroyAll() ;
    mSinkMap.destroyAll() ;
    mSocketPurposeMap.destroyAll() ;
    mSocketEnabledTime.destroyAll() ;
    mMapMutex.release() ;
}


/* ============================ MANIPULATORS ============================== */

bool CpMediaSocketMonitorTask::monitor(OsSocket* pSocket, ISocketEvent* pSink, SocketPurpose purpose)
{
    mMapMutex.acquire();
    bool bRet = false;
    
    if (!mSocketMap.find(& UtlInt((int)pSocket)))
    {
        mSinkMap.insertKeyAndValue(new UtlInt((int)pSink), new UtlInt((int)pSocket));
        mSocketMap.insertKeyAndValue(new UtlInt((int)pSocket), new UtlInt((int)pSink));
        // register as an observer of the object implementing the idle sink
        UtlObservable* pObservable = dynamic_cast<UtlObservable*>(pSink);
        if (pObservable)
        {
            pObservable->registerObserver(this);
        }
       
        bRet = true;
    }

    if (!mSocketPurposeMap.find(&UtlInt((int)pSocket)))
    {
        mSocketPurposeMap.insertKeyAndValue(new UtlInt((int)pSocket), new UtlInt((int)purpose));
    }
    mMapMutex.release();
    
    return bRet;
}

bool CpMediaSocketMonitorTask::unmonitor(OsSocket* pSocket)
{
    bool bRet = false;

    assert(pSocket != NULL) ;
    if (pSocket)
    {
        mMapMutex.acquire();        
        UtlInt* pSinkPtr = NULL;
        
        if (pSinkPtr = (UtlInt*)mSocketMap.findValue(& UtlInt((int)pSocket)))
        {
            ISocketEvent* pSink;
            
            if (pSinkPtr)
            {
                pSink = (ISocketEvent*)pSinkPtr->getValue();
                
                if (pSink)
                {
                    mSinkMap.destroy( & UtlInt((int) pSink));
                }
            }

            mSocketMap.destroy( & UtlInt((int)pSocket));
            bRet = true;
        }
        if (mSocketPurposeMap.find(& UtlInt((int)pSocket)))
        {
            mSocketPurposeMap.destroy( & UtlInt((int)pSocket));
        }
        
        disableSocket(pSocket) ;

        mMapMutex.release();        
    }
    return bRet;
}

UtlBoolean CpMediaSocketMonitorTask::handleMessage(OsMsg& rMsg)
{
    UtlBoolean bRet(FALSE);
    OsDateTime now;
    OsDateTime::getCurTime(now);
    OsDateTime socketAdded ;
    
    mMapMutex.acquire();
    
    // don't even bother looking at the message. It should be
    // a timer message.
    
    // check all of the sockets in our map for timeouts,
    // and notify, if necessary
    UtlHashMapIterator iterator(mSocketMap);
    UtlInt* pSocketPtr;
    UtlInt* pSinkPtr;
    ISocketEvent* pSink;
    OsSocket* pSocket;
    while (pSocketPtr = (UtlInt*)iterator())
    {
        OsDateTime check ;
        pSocket = (OsSocket*)pSocketPtr->getValue();
        
        // Only check/fire if we have a socket, that socket is enabled,
        // and we have actually written some data
        if (pSocket && 
                isSocketEnabled(pSocket, socketAdded) && 
                (pSocket->getLastWriteTime(check) || !pSocket->isOk()))
        {
            OsDateTime then;
            if (!pSocket->getLastReadTime(then))
            {
                then = socketAdded ;
            }
            else
            {
                // If the last read was before the socket enable, go with the 
                // socket enable timestamp.
                if (then.getSecsSinceEpoch() < socketAdded.getSecsSinceEpoch())
                    then = socketAdded ;
            }

               
            OsTime now2;
            OsTime then2;
            now.cvtToTimeSinceEpoch(now2);
            then.cvtToTimeSinceEpoch(then2);
            
            OsTime diff(now2 - then2);
            long millisecondsIdle = diff.cvtToMsecs();
            if (!pSocket->isOk() || (then2 != OsTime::OS_INFINITY && (millisecondsIdle > (mIdleTimeoutSeconds * 1000))))
            {
                // notify the sink that we timed out.
                pSinkPtr = (UtlInt*)mSocketMap.findValue(pSocketPtr);
                UtlInt* pPurposeContainer = (UtlInt*)mSocketPurposeMap.findValue(pSocketPtr);
                SocketPurpose purpose = UNKNOWN;

                if (pPurposeContainer)
                {
                    purpose = (SocketPurpose)pPurposeContainer->getValue();
                }
                if (pSinkPtr)
                {
                    pSink = (ISocketEvent*)pSinkPtr->getValue();
                    if (pSink)
                    {
                        pSink->onIdleData(pSocket, purpose, millisecondsIdle);
                    }
                }
            }
        }
        
    }
    mMapMutex.release();
    return bRet;
}

void CpMediaSocketMonitorTask::enableSocket(OsSocket* pSocket) 
{    
    OsDateTime when; 
    OsLock lock(mMapMutex) ;

    assert(pSocket != NULL) ;
    if (pSocket && !isSocketEnabled(pSocket, when))
    {
        OsDateTime* pNow = new OsDateTime() ;
        if (pNow)
        {
            OsDateTime::getCurTime(*pNow) ;
            mSocketEnabledTime.insertKeyAndValue(
                    new UtlVoidPtr(pSocket),
                    new UtlVoidPtr(pNow)) ;

            assert(isSocketEnabled(pSocket, when) == (UtlBoolean) true) ;
        }
    }
}

void CpMediaSocketMonitorTask::disableSocket(OsSocket* pSocket) 
{            
    assert(pSocket != NULL) ;
    if (pSocket)
    {
        OsLock lock(mMapMutex) ;    
        UtlVoidPtr key(pSocket) ;        

        UtlVoidPtr* pValue = (UtlVoidPtr*) mSocketEnabledTime.findValue(&key) ;
        if (pValue)
        {
            OsDateTime* pAddedTime = (OsDateTime*) pValue->getValue() ;
            delete pAddedTime ;
            mSocketEnabledTime.destroy(&key) ;
        }
    
        OsDateTime when ;
        assert(isSocketEnabled(pSocket, when) == (UtlBoolean) false) ;   
    }
}
    
UtlBoolean CpMediaSocketMonitorTask::isSocketEnabled(OsSocket* pSocket,
                                                         OsDateTime&  when) 
{
    UtlBoolean bRC = false ;    
    assert(pSocket != NULL) ;
    if (pSocket != NULL)
    {
        UtlVoidPtr key(pSocket) ;    
        OsLock lock(mMapMutex) ;

        UtlVoidPtr* pValue = (UtlVoidPtr*) mSocketEnabledTime.findValue(&key) ;
        if (pValue)
        {
            OsDateTime* pAddedTime = (OsDateTime*) pValue->getValue() ;
            assert(pAddedTime != NULL) ;
            if (pAddedTime)
            {
                when = *pAddedTime ;
                bRC = true ;
            }
        }
    }
            
    return bRC ;
}


void CpMediaSocketMonitorTask::onNotify(UtlObservable* observable, int code, void *pUserData)
{
    // we are only notified about the destruction of the observable, 
    // so, no need to check the int-code
    mMapMutex.acquire();    
    
    ISocketEvent* pSink = static_cast<ISocketEvent*>(pUserData);

    // lookup socket
    UtlInt* pSocketContainer = (UtlInt*)mSinkMap.findValue(&UtlInt((int)pSink));
    if (pSocketContainer)
    {
        OsSocket* pSocket = NULL;
        pSocket = (OsSocket*)pSocketContainer->getValue();
        unmonitor(pSocket);
    }

    mMapMutex.release();    
}

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


