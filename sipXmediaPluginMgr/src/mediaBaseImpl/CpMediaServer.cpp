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
#include <assert.h>

// APPLICATION INCLUDES
#include "mediaBaseImpl/CpMediaServer.h"
#include "mediaBaseImpl/CpMediaConnectionSocket.h"
#include "mediaBaseImpl/CpMediaNetTask.h"


CpMediaServer::CpMediaServer(CpMediaConnectionSocket* pOwner,
                                               const char*  szLocalIp,
                                               int  localPort)
    : mpOwner(pOwner),
      mpServerSocket(NULL),
      mpConnection(NULL),
      mLocalPort(localPort),
      mBindAddr(szLocalIp),
      mSocketLock(OsMutex::Q_FIFO)      
{
    start();
}                       

CpMediaServer::~CpMediaServer()
{
    requestShutdown();
    OsNatConnectionSocket* pSocket = mpConnection;
    if (pSocket)
    {
        mpConnection = NULL;
        pSocket->close();
    }
    delete pSocket;

    OsNatServerSocket* pSeverSocket = mpServerSocket;
    mpServerSocket = NULL;
    pSeverSocket->close();    
    
    waitUntilShutDown();
    delete pSeverSocket;
}

OsNatConnectionSocket* CpMediaServer::acquireConnectionSocket()
{
    mSocketLock.acquireRead();
    return mpConnection;
}

void CpMediaServer::releaseConnectionSocket()
{
    mSocketLock.releaseRead();
}

int CpMediaServer::run(void* pArg)
{
    mSocketLock.acquireWrite();
    mpServerSocket = new OsNatServerSocket(1, mLocalPort, mBindAddr.data());
    mSocketLock.releaseWrite();

    while (!isShuttingDown() && !isShutDown())
    {
        // accept the initial connection 
        if (mpServerSocket && !mpConnection)
        {
            mpConnection = dynamic_cast<OsNatConnectionSocket*>(mpServerSocket->accept());
            if (mpConnection)
            {
                IMediaSocket* pStunSocket = dynamic_cast<IMediaSocket*>(mpOwner);
                CpMediaNetTask::getCpMediaNetTask()->addInputSource(pStunSocket);
                mpOwner->setRole(RTP_TCP_ROLE_PASSIVE);
            }
        }
        OsTask::delay(100);
    } 
    return 0;
}
