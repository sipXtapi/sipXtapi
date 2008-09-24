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


//#include "rtcp/RtcpConfig.h"

#define LOGEM
#undef LOGEM

// SYSTEM INCLUDES

#include "os/OsDefs.h"
#include <assert.h>
#include <string.h>
#include "os/OsTask.h"
//#include "os/IOsNatSocket.h"
#include "mediaInterface/IMediaSocket.h"

#ifdef WIN32 /* [ */
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#endif /* WIN32 ] */

#if defined(__pingtel_on_posix__) /* [ */
#include <stdlib.h>
#include <sys/time.h>
typedef unsigned int UINT;
#endif /* __pingtel_on_posix__ ] */

// APPLICATION INCLUDES

#include "os/OsDefs.h"
#include "os/OsTask.h"
#include "os/OsServerSocket.h"
#include "os/OsConnectionSocket.h"
#include "os/OsEvent.h"
#include "mediaBaseImpl/CpMediaNetTask.h"
#include "mediaBaseImpl/CpMediaSocketMonitorTask.h"

// EXTERNAL FUNCTIONS

// EXTERNAL VARIABLES

// CONSTANTS
#define NET_TASK_MAX_ERRORS 20

#define NET_TASK_MAX_MSG_LEN sizeof(CpMediaNetTaskMsg)
#define NET_TASK_MAX_FD_PAIRS 100

// MACROS
#ifndef max
#define max(a,b) (((a) > (b))?(a):(b))
#endif


struct __CpMediaNetTaskMsg 
{
   OsProtectedEvent* notify;

   enum { ADD, REMOVE, SHUTDOWN } operation ;
   IMediaSocket* pSocket ;
   int nErrors ;
   
};

typedef struct __CpMediaNetTaskMsg CpMediaNetTaskMsg, *CpMediaNetTaskMsgPtr;

// STATIC VARIABLE INITIALIZATIONS

static  CpMediaNetTaskMsg pairs[NET_TASK_MAX_FD_PAIRS];
static  int numPairs;

CpMediaNetTask* CpMediaNetTask::spInstance = 0;
OsRWMutex     CpMediaNetTask::sLock(OsBSem::Q_PRIORITY);

const int CpMediaNetTask::DEF_NET_IN_TASK_PRIORITY  = 100; // default task priority
const int CpMediaNetTask::DEF_NET_IN_TASK_OPTIONS   = 0;   // default task options
#ifdef USING_NET_EQ /* [ */
const int CpMediaNetTask::DEF_NET_IN_TASK_STACKSIZE = 40960;//default task stacksize
#else /* USING_NET_EQ ] [ */
const int CpMediaNetTask::DEF_NET_IN_TASK_STACKSIZE = 4096;// default task stacksize
#endif /* USING_NET_EQ ] */

#define DEBUG
#undef  DEBUG


/************************************************************************/


OsConnectionSocket* CpMediaNetTask::getWriteSocket()
{
    return mpWriteSocket;
}

OsConnectionSocket* CpMediaNetTask::createWriteSocket()
{
    if (NULL == mpWriteSocket) 
    {
        OsConnectionSocket* pWriteSocket = NULL;

        int trying = 1000;        
        while (trying > 0)
        {
            OsTask::delay(5);
            pWriteSocket = new OsConnectionSocket(mCmdPort, "127.0.0.1", TRUE, "127.0.0.1") ;            
            if (pWriteSocket && pWriteSocket->isConnected()) 
            {
                break;
            }
            else
            {
                delete pWriteSocket ;
                pWriteSocket = NULL ;
            }
            trying--;
        }
        mpWriteSocket = pWriteSocket ;
    }
    return mpWriteSocket;
}


OsConnectionSocket* CpMediaNetTask::getReadSocket()
{
    return mpReadSocket;
}


OsStatus CpMediaNetTask::addInputSource(IMediaSocket* pSocket) 
{    
    sLock.acquireWrite();
    int wrote = 0;

    assert(pSocket != NULL) ;

    OsConnectionSocket* writeSocket = getWriteSocket() ; 
    if (writeSocket)
    {
        CpMediaNetTaskMsg msg;
        memset((void*)&msg, 0, sizeof(CpMediaNetTaskMsg));
        msg.operation = CpMediaNetTaskMsg::ADD ;
        msg.pSocket = pSocket ;

        wrote = writeSocket->write((char *) &msg, NET_TASK_MAX_MSG_LEN);
        if (wrote != NET_TASK_MAX_MSG_LEN)
        {
            OsSysLog::add(FAC_MP, PRI_ERR, 
                    "CpMediaNetTask::addInputSource - writeSocket error: 0x%08x,%d wrote %d",
                    (int)writeSocket, writeSocket->getSocketDescriptor(), wrote);
        }
    }
    else
    {
        assert(FALSE) ;
    }
    sLock.releaseWrite();
    return ((NET_TASK_MAX_MSG_LEN == wrote) ? OS_SUCCESS : OS_BUSY);
}


OsStatus CpMediaNetTask::removeInputSource(IMediaSocket* pSocket, OsProtectedEvent* pEvent) 
{
    sLock.acquireWrite();    
    int wrote = 0;

    assert(pSocket != NULL) ;

    OsConnectionSocket* writeSocket = getWriteSocket() ; 
    if (writeSocket)
    {
        CpMediaNetTaskMsg msg;
        memset((void*)&msg, 0, sizeof(CpMediaNetTaskMsg));
        msg.operation = CpMediaNetTaskMsg::REMOVE ;
        msg.pSocket = pSocket ;
        msg.notify = pEvent;

        wrote = writeSocket->write((char *) &msg, NET_TASK_MAX_MSG_LEN);
        if (wrote != NET_TASK_MAX_MSG_LEN)
        {
            OsSysLog::add(FAC_MP, PRI_ERR, 
                    "CpMediaNetTask::removeInputSource - writeSocket error: 0x%08x,%d wrote %d",
                    (int)writeSocket, writeSocket->getSocketDescriptor(), wrote);
        }
    }
    else
    {
        assert(FALSE) ;
    }
    sLock.releaseWrite();
    return ((NET_TASK_MAX_MSG_LEN == wrote) ? OS_SUCCESS : OS_BUSY);
}

int XXisFdPoison(int fd)
{
        fd_set fdset;
        fd_set *fds;
        int     numReady;
        struct  timeval tv, *ptv;

        if (0 > fd) {
            return TRUE;
        }

        tv.tv_sec = tv.tv_usec = 0;
        ptv = &tv;
        fds = &fdset;
        FD_ZERO(fds);
        FD_SET((UINT) fd, fds);
        numReady = select(fd+1, fds, NULL, NULL, ptv);
        return (0 > numReady) ? TRUE : FALSE;
}

int XXfindPoisonFds(int pipeFD)
{
        int i;
        int n = 0;
        CpMediaNetTaskMsgPtr ppr;

        // Check control Socket
        if (XXisFdPoison(pipeFD)) 
        {
            OsSysLog::add(FAC_MP, PRI_ERR, " *** CpMediaNetTask: pipeFd socketDescriptor=%d busted!\n", pipeFD);
            return -1;
        }

        // Check data sockets
        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) 
        {
            if (ppr->pSocket && // not NULL socket and
                    XXisFdPoison(ppr->pSocket->getSocket()->getSocketDescriptor())) 
            {
                if (ppr->nErrors > NET_TASK_MAX_ERRORS)
                {
                    OsSysLog::add(FAC_MP, PRI_ERR, " *** CpMediaNetTask: Removing fdRtp[%d], socket=0x%08x, socketDescriptor=%d\n", ppr-pairs,(int)ppr->pSocket, (int)ppr->pSocket->getSocket()->getSocketDescriptor());
                    n++;
                    ppr->pSocket = NULL;
                    ppr->nErrors = 0 ;
                }
                else
                {
                    ppr->nErrors++ ;
                }
            }
            ppr++;
        }
        return n;
}

int CpMediaNetTask::processControlSocket(int last, bool* pbShutdown)
{
    CpMediaNetTaskMsg    msg;
    CpMediaNetTaskMsgPtr ppr;
    int i ;
    int newFd ;

    memset((void*)&msg, 0, sizeof(CpMediaNetTaskMsg));
    if (NET_TASK_MAX_MSG_LEN != mpReadSocket->read((char *) &msg, NET_TASK_MAX_MSG_LEN)) 
    {
        OsSysLog::add(FAC_MP, PRI_ERR, "CpMediaNetTask::run: Invalid request!") ;
    } 
    else if (-2 == (int) msg.pSocket) 
    {
        // Request to exit

        OsSysLog::add(FAC_MP, PRI_ERR, " *** CpMediaNetTask: closing pipeFd (%d)\n",
            mpReadSocket->getSocketDescriptor());

        if (mpReadSocket)
        {
            mpReadSocket->close();
        }
    } 
    else 
    {
        switch (msg.operation)
        {
            case CpMediaNetTaskMsg::ADD:                            

                // Insert into the pairs list
                newFd  = (msg.pSocket) ? msg.pSocket->getSocket()->getSocketDescriptor() : -1;

                OsSysLog::add(FAC_MP, PRI_DEBUG, " *** CpMediaNetTask: Adding new sockets (socket: %p,%d)\n",
                          msg.pSocket, newFd);

                // Look for duplicate file descriptors; remove if found
                for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) 
                {
                    if (ppr->pSocket != NULL) 
                    {
                        int existingFd  = (ppr->pSocket)  ? ppr->pSocket->getSocket()->getSocketDescriptor()  : -1;
                        UtlBoolean foundDupFd  = FALSE;

                        if ((existingFd >= 0) && (existingFd == newFd))
                        {
                            OsSysLog::add(FAC_MP, PRI_ERR, 
                                    " *** CpMediaNetTask: Using a dup descriptor (New:%p,%d, Old:%p,%d)\n",
                                    msg.pSocket, newFd, ppr->pSocket, existingFd) ;
                            ppr->pSocket = NULL;
                            numPairs-- ;
                        }
                    }
                    ppr++;
                }

                // Add pair
                for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) 
                {
                    if (ppr->pSocket == NULL) 
                    {
                        ppr->pSocket = msg.pSocket;
                        ppr->nErrors = 0 ;
                        numPairs++;
                        OsSysLog::add(FAC_MP, PRI_DEBUG, 
                                " *** CpMediaNetTask: Add socket Fds: (New=%p,%d)\n",
                                msg.pSocket, newFd);
                        break ;
                    }
                    ppr++;
                }
                
                // Update last
                last = max(last, newFd);

                break ;
            case CpMediaNetTaskMsg::REMOVE:
                for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) 
                {
                    if (msg.pSocket == ppr->pSocket) 
                    {
                        OsSysLog::add(FAC_MP, PRI_DEBUG, 
                                " *** CpMediaNetTask: Remove socket Fds: (Old=%p,%d)\n",
                                ppr->pSocket, ppr->pSocket->getSocket()->getSocketDescriptor()) ;
                        ppr->pSocket = NULL ;
                        ppr->nErrors = 0 ;
                    }
                    last = OS_INVALID_SOCKET_DESCRIPTOR ;
                    numPairs-- ;
                    ppr++;
                }
                if (msg.notify && msg.notify->isInUse())
                {
                    msg.notify->signal(0);
                }
                else
                {
                    OsSysLog::add(FAC_MP, PRI_ERR, 
                                " *** CpMediaNetTask: CpMediaNetTaskMsg::REMOVE: already signaled\n") ;
                }
                break ;
            case CpMediaNetTaskMsg::SHUTDOWN:
                if (pbShutdown)
                {
                    *pbShutdown = TRUE ;
                }
                break ;
            default:
                break ;
        }
    }

    return last ;
}

int CpMediaNetTask::run(void *pNotUsed)
{
    fd_set fdset;
    fd_set *fds;
    int     last;
    int     i;
    int     numReady;
    CpMediaNetTaskMsgPtr ppr;
    OsServerSocket* pBindSocket;
    bool    bShutdown = FALSE ;

    // Setup control socket
    pBindSocket = new OsServerSocket(1, PORT_DEFAULT, "127.0.0.1") ;
    mCmdPort = pBindSocket->getLocalHostPort() ;
    OsSysLog::add(FAC_MP, PRI_DEBUG, "CpMediaNetTask::run mCmdPort = %d", mCmdPort);
    
    assert(-1 != mCmdPort) ;
    mpReadSocket = pBindSocket->accept() ;
    pBindSocket->close();
    delete pBindSocket;
    pBindSocket = NULL;

    // If we failed to setup the control socket, abort...
    if (NULL == mpReadSocket) 
    {
        printf(" *** CpMediaNetTask: accept() failed!\n", 0,0,0,0,0,0);
        return 0;
    }

    // Clean up our control structures
    for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) 
    {
        ppr->pSocket =  NULL;
        ppr->nErrors = 0 ;
        ppr++;
    }
    numPairs = 0;


    fds = &fdset;
    last = OS_INVALID_SOCKET_DESCRIPTOR;

    // Only run while the control socket is stable
    while (!bShutdown && mpReadSocket && mpReadSocket->isOk()) 
    {
        // Figure out the max socket desc number
        if (OS_INVALID_SOCKET_DESCRIPTOR == last) 
        {
           last = mpReadSocket->getSocketDescriptor();
           for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) 
           {
                if (NULL != ppr->pSocket)
                {
                    last=max(last, ppr->pSocket->getSocket()->getSocketDescriptor());
                }
                ppr++;
            }                
        }

        // Build the fdset
        FD_ZERO(fds);
        FD_SET((UINT) mpReadSocket->getSocketDescriptor(), fds);
        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) 
        {
            if (NULL != ppr->pSocket && 
                    (NULL != ppr->pSocket->getSocket()) && 
                    (0 < ppr->pSocket->getSocket()->getSocketDescriptor()))
            {
                FD_SET((UINT)(ppr->pSocket->getSocket()->getSocketDescriptor()), fds);
            }
            ppr++;
        }

        // Wait for some data
        errno = 0;
        numReady = select(last+1, fds, NULL, NULL, NULL);
        if (numReady < 0)  
        {
            OsSysLog::add(FAC_MP, PRI_ERR, " *** CpMediaNetTask: select returned %d, errno=%d=0x%X\n",
                numReady, errno, errno);
            i = XXfindPoisonFds(mpReadSocket->getSocketDescriptor());
            if (i < 0) 
            {
                sLock.acquireWrite();
                OsSysLog::add(FAC_MP, PRI_ERR, " *** CpMediaNetTask: My comm socket failed! Quitting!\n");
                mpReadSocket->close();
                mpReadSocket = NULL;
                sLock.releaseWrite();
            } 
            else if (i > 0) 
            {
                last = OS_INVALID_SOCKET_DESCRIPTOR;
            }
            continue;
        }
        
        // Process the control socket
        if (FD_ISSET(mpReadSocket->getSocketDescriptor(), fds)) 
        {
            numReady-- ;
            sLock.acquireWrite();
            last = processControlSocket(last, &bShutdown) ;
            sLock.releaseWrite();
        }

        if (!bShutdown)
        {
            ppr=pairs;
            for (i=0; ((i<NET_TASK_MAX_FD_PAIRS)&&(numReady>0)); i++) 
            {
                if ((NULL != ppr->pSocket ) &&
                        (NULL != ppr->pSocket->getSocket()) &&
                        (0 < ppr->pSocket->getSocket()->getSocketDescriptor()) &&
                        (FD_ISSET(ppr->pSocket->getSocket()->getSocketDescriptor(), fds))) 
                {
                    ppr->nErrors = 0 ;
                    IMediaSocket* pMediaSocket = dynamic_cast<IMediaSocket*>(ppr->pSocket->getSocket());
                    if (pMediaSocket)
                    {
                        if (!pMediaSocket->pushPacket())
                        {
                            if (ppr->nErrors > NET_TASK_MAX_ERRORS)
                            {
                                OsSysLog::add(FAC_MP, PRI_INFO, " *** Closing socket on pushPacket signal (%p)\n",
                                    ppr->pSocket);

                                ppr->pSocket = NULL;
                                ppr->nErrors = 0 ;
                            }
                            else
                            {
                                ppr->nErrors++ ;
                            }
                        }    
                    }
                }
                ppr++;
            }
        }
        
    }

    if (mpReadSocket)
    {
        mpReadSocket->close();
        delete mpReadSocket;
        mpReadSocket = NULL;
    }
    
    if (mpWriteSocket)
    {
        mpWriteSocket->close();
        delete mpWriteSocket;
        mpWriteSocket = NULL;
    }

    return 0;
}


CpMediaNetTask* CpMediaNetTask::createCpMediaNetTask()
{
   UtlBoolean isStarted;
   // OsStatus  stat;

   // If the task object already exists, and the corresponding low-level task
   // has been started, then use it
   if (spInstance != NULL && spInstance->isStarted())
      return spInstance;

   // If the task does not yet exist or hasn't been started, then acquire
   // the lock to ensure that only one instance of the task is started
   sLock.acquireRead();
   if (spInstance == NULL) {
       spInstance = new CpMediaNetTask();
   }
   isStarted = spInstance->isStarted();
   if (!isStarted)
   {
      isStarted = spInstance->start() ;
      spInstance->createWriteSocket();
      assert(isStarted);
   }
   sLock.releaseRead();
   return spInstance;
}


CpMediaNetTask* CpMediaNetTask::getCpMediaNetTask()
{
   return spInstance;
}


// Default constructor (called only indirectly via getCpMediaNetTask())
CpMediaNetTask::CpMediaNetTask(int prio, int options, int stack)
:  OsTask("CpMediaNetTask", NULL, 85, options, stack),
   mpWriteSocket(NULL),
   mpReadSocket(NULL),
   mCmdPort(-1)
{
    CpMediaSocketMonitorTask::createInstance(5);  // hard code
                                                       // to a 
                                                       // 5 second
                                                       // poll for now
    CpMediaSocketMonitorTask::getInstance()->setIdleTimeout(5);
                                                      
}

// Destructor
CpMediaNetTask::~CpMediaNetTask()
{  
   CpMediaSocketMonitorTask* pSocketMonitor = CpMediaSocketMonitorTask::getInstance() ;
   waitUntilShutDown();

   if (pSocketMonitor)
   {
       pSocketMonitor->requestShutdown() ;
       delete pSocketMonitor ;
   }
   spInstance = NULL;
}

// $$$ These messages need to contain OsSocket* instead of fds.

OsStatus shutdownCpMediaNetTask()
{
    CpMediaNetTask::getLockObj().acquireWrite();
    CpMediaNetTask* pInst = CpMediaNetTask::getCpMediaNetTask();       
    if (pInst)
    {        
        int wrote;
        OsConnectionSocket* writeSocket = pInst->getWriteSocket();
        if (writeSocket)
        {
            CpMediaNetTaskMsg msg;
            memset((void*)&msg, 0, sizeof(CpMediaNetTaskMsg));
            msg.operation = CpMediaNetTaskMsg::SHUTDOWN ;
            wrote = writeSocket->write((char *) &msg, NET_TASK_MAX_MSG_LEN);        
        }
        else
            wrote = NET_TASK_MAX_MSG_LEN ;
        CpMediaNetTask::getLockObj().releaseWrite();
        CpMediaNetTask* pTask = CpMediaNetTask::getCpMediaNetTask();        
        CpMediaNetTask::getLockObj().acquireWrite();        
        pTask->requestShutdown();        
        CpMediaNetTask::getLockObj().releaseWrite();
        delete pInst;
        return ((NET_TASK_MAX_MSG_LEN == wrote) ? OS_SUCCESS : OS_BUSY);
    }
    else
    {  
        CpMediaNetTask::getLockObj().releaseWrite();
        return OS_SUCCESS;
    }
}


#define LOGEM
#undef LOGEM

/************************************************************************/

/* ============================ FUNCTIONS ================================= */

