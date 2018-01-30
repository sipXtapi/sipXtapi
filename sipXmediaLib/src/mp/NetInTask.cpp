//  
// Copyright (C) 2006-2017 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#include "rtcp/RtcpConfig.h"

// SYSTEM INCLUDES

#include <assert.h>
#include <string.h>
#ifdef _VXWORKS /* [ */
#include <selectLib.h>
#include <iosLib.h>
#include <inetlib.h>
#endif /* _VXWORKS ] */

#ifdef WIN32 /* [ */
#   ifndef WINCE
#       include <io.h>
#   endif
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#endif /* WIN32 ] */

#ifdef __pingtel_on_posix__ /* [ */
#include <os/OsIntTypes.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/time.h>
#endif /* __pingtel_on_posix__ ] */

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsTask.h>
#include <os/OsServerSocket.h>
#include <os/OsConnectionSocket.h>
#include <os/OsEvent.h>
#include <mp/NetInTask.h>
#include <mp/MpUdpBuf.h>
#include <mp/MprFromNet.h>
#include <utl/UtlRandom.h>
#ifdef _VXWORKS /* [ */
#ifdef CPU_XSCALE /* [ */
#include <mp/pxa255.h">
#define OSTIMER_COUNTER_POINTER ((int*) PXA250_OSTIMER_OSCR)
#else /* CPU_XSCALE ] [ */
#include "mp/sa1100.h"
#define OSTIMER_COUNTER_POINTER ((int*) SA1100_OSTIMER_COUNTER)
#endif /* CPU_XSCALE ] */
#else /* _VXWORKS ][ */
#define OSTIMER_COUNTER_POINTER (&dummy0)
static int dummy0 = 0;
#endif /* _VXWORKS ] */

//#define RTL_ENABLED
#ifdef RTL_ENABLED
#  include <rtl_macro.h>
#else
#  define RTL_START(x)
#  define RTL_BLOCK(x)
#  define RTL_EVENT(x,y)
#  define RTL_WRITE(x)
#  define RTL_STOP
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define NET_TASK_MAX_MSG_LEN sizeof(netInTaskMsg)
#define NET_TASK_MAX_FD_PAIRS 100

#define MAX_RTP_BYTES 1500

struct netInTaskMsg {
   OsSocket* pRtpSocket;
   OsSocket* pRtcpSocket;
   MprFromNet* fwdTo;
   OsNotification* notify;
   int fdRtp;
   int fdRtcp;
};

// STATIC VARIABLE INITIALIZATIONS
volatile int* pOsTC = OSTIMER_COUNTER_POINTER;

static  netInTaskMsg pairs[NET_TASK_MAX_FD_PAIRS];
static  int numPairs;

NetInTask* NetInTask::spInstance = 0;
OsRWMutex     NetInTask::sLock(OsBSem::Q_PRIORITY);

const int NetInTask::DEF_NET_IN_TASK_PRIORITY  = 0;   // default task priority: HIGHEST
const int NetInTask::DEF_NET_IN_TASK_OPTIONS   = 0;   // default task options
#ifdef USING_NET_EQ /* [ */
const int NetInTask::DEF_NET_IN_TASK_STACKSIZE = 40960;//default task stacksize
#else /* USING_NET_EQ ] [ */
const int NetInTask::DEF_NET_IN_TASK_STACKSIZE = 64*1024;// default task stacksize
#endif /* USING_NET_EQ ] */

#define SIPX_DEBUG
#undef  SIPX_DEBUG

/************************************************************************/

/**
*  @brief Help create writer side of connection for NetInTask internal use.
*
*  This task is used to create connection to socket we're listening. Main task
*  create OsServerConnection and wait call OsServerConnection::accept() to
*  get reader side of connection. On the side of this task writer side of
*  connection is connected.
*/
class NetInTaskHelper : public OsTask
{
public:
    NetInTaskHelper(int port);
    ~NetInTaskHelper();

    /// Do the task.
    virtual int run(void* pArg);

    /// Wait for helper task to finish and return connected socket.
    inline OsConnectionSocket* getSocket();

private:
   OsConnectionSocket* mpSocket;
   int                 mPort;
};


NetInTaskHelper::NetInTaskHelper(int port)
: OsTask("NetInTaskHelper-%d", NULL, 25, 0, 256*1024)
, mpSocket(NULL)
, mPort(port)
{
}

NetInTaskHelper::~NetInTaskHelper()
{
    waitUntilShutDown();
}

int NetInTaskHelper::run(void*)
{
    int numTry = 0;
    const int maxTries = 1000;
    while (numTry < maxTries)
    {
       // Delay for some time on consecutive runs.
       if (numTry > 0)
       {
          OsTask::delay(1);
       }

       // Connect...
       mpSocket = new OsConnectionSocket(mPort, "127.0.0.1");
       if (mpSocket && mpSocket->isConnected())
       {
          break;
       }

       numTry++;
    }

    if (numTry < maxTries)
    {
       OsSysLog::add(FAC_MP, PRI_INFO,
                     "NetInTaskHelper::run() returning after %d tries\n",
                     numTry);
    }
    else
    {
       OsSysLog::add(FAC_MP, PRI_ERR,
                     "NetInTaskHelper::run() failed to connect after %d tries\n",
                     numTry);
       assert(!"NetInTaskHelper::run() failed to connect!");
    }

    return 0;
}

OsConnectionSocket* NetInTaskHelper::getSocket()
{
   waitUntilShutDown();
   return mpSocket;
}

/************************************************************************/

static void flushReadQueue(OsSocket* pSock)
{
   char junk[MAX_RTP_BYTES];

   // Read all pending data from the socket
   while (pSock->isReadyToRead())
   {
      // Bail if a read error occurs
      if (pSock->read(junk, MAX_RTP_BYTES) <= 0)
         break;
   }
   return;
}

static OsStatus get1Msg(OsSocket* pRxpSkt, MprFromNet* fwdTo, bool isRtcp,
                        int ostc)
{
        MpUdpBufPtr ib;
        int nRead;
        struct in_addr fromIP;
        int      fromPort;

static  int numFlushed = 0;
static  int flushedLimit = 125;

        if (numFlushed >= flushedLimit) {
            Zprintf("get1Msg: flushed %d packets! (after %d DMA frames).\n",
               numFlushed, showFrameCount(1), 0,0,0,0);
            if (flushedLimit<1000000) {
                flushedLimit = flushedLimit << 1;
            } else {
                numFlushed = 0;
                flushedLimit = 125;
            }
        }

        // Get new buffer for incoming packet
        ib = MpMisc.UdpPool->getBuffer();

        if (ib.isValid()) {
            // Read packet data.
            // Note: nRead could not be greater then buffer size.
            nRead = pRxpSkt->read(ib->getDataWritePtr(), ib->getMaximumPacketSize()
                                 , &fromIP, &fromPort);
            // Set size of received data
            ib->setPacketSize(nRead);

            // Set IP address and port of this packet
            ib->setIP(fromIP);
            ib->setUdpPort(fromPort);

            // Set time we receive this packet.
            ib->setTimecode(ostc);

            if (nRead > 0) 
            {
                RTL_BLOCK("NetInTask.pushPacket");
                fwdTo->pushPacket(ib, isRtcp);
            } 
            else 
            {
                OsSysLog::add(FAC_MP, PRI_DEBUG,
                        "NetInTask::get1Msg read %d from socket: %p descriptor: %d errno: %d",
                        nRead, pRxpSkt, pRxpSkt->getSocketDescriptor(), errno);

                if (!pRxpSkt->isOk())
                {
                    Zprintf(" *** get1Msg: read(%d) returned %d, errno=%d=0x%X)\n",
                        (int) pRxpSkt, nRead, errno, errno, 0,0);
                    return OS_NO_MORE_DATA;
                }                                
            }
        } else {
            // Flush packet if could not get buffer for it.
            char buffer[UDP_MTU];
            nRead = pRxpSkt->read(buffer, UDP_MTU, &fromIP, &fromPort);
            if (numFlushed++ < 10) {
                Zprintf("get1Msg: flushing a packet! (%d, %d, %d)"
                    " (after %d DMA frames).\n",
                    nRead, errno, (int) pRxpSkt, showFrameCount(1), 0,0);
            }
            if(nRead < 1)
            {
                OsSysLog::add(FAC_MP, PRI_DEBUG,
                        "NetInTask::get1Msg read %d flushed packet from socket: %p descriptor: %d errno: %d",
                        nRead, pRxpSkt, pRxpSkt->getSocketDescriptor(), errno);
            }

            if ((nRead < 1) && !pRxpSkt->isOk()) 
            {
                return OS_NO_MORE_DATA;
            }
        }
        return OS_SUCCESS;
}

int isFdPoison(int fd)
{
        fd_set fdset;
        fd_set *fds;
        int     numReady;
        timeval tv, *ptv;

        if (0 > fd) {
            return TRUE;
        }

        tv.tv_sec = tv.tv_usec = 0;
        ptv = &tv;
        fds = &fdset;
        FD_ZERO(fds);
        FD_SET((unsigned) fd, fds);
        numReady = select(fd+1, fds, NULL, NULL, ptv);
        return (0 > numReady) ? TRUE : FALSE;
}

int findPoisonFds(int pipeFD)
{
        int i;
        int n = 0;
        netInTaskMsg* ppr;

        if (isFdPoison(pipeFD)) {
            OsSysLog::add(FAC_MP, PRI_ERR, " *** NetInTask: pipeFd socketDescriptor=%d busted! (poison)\n", pipeFD);
            return -1;
        }
        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
            if (ppr->pRtpSocket && // not NULL socket and
                isFdPoison(ppr->pRtpSocket->getSocketDescriptor())) {
                OsSysLog::add(FAC_MP, PRI_ERR, " *** NetInTask: Removing fdRtp[%" PRIdPTR "], socket=%p, socketDescriptor=%d (poison)\n", ppr-pairs, ppr->pRtpSocket, ppr->pRtpSocket->getSocketDescriptor());
                n++;
                ppr->pRtpSocket = NULL;
                if (NULL == ppr->pRtcpSocket) ppr->fwdTo = NULL;
            }
            if (ppr->pRtcpSocket && // not NULL socket and
                isFdPoison(ppr->pRtcpSocket->getSocketDescriptor())) {
                OsSysLog::add(FAC_MP, PRI_ERR, " *** NetInTask: Removing fdRtcp[%" PRIdPTR "], socket=%p, socketDescriptor=%d (poison)\n", ppr-pairs, ppr->pRtcpSocket, ppr->pRtcpSocket->getSocketDescriptor());
                n++;
                ppr->pRtcpSocket = NULL;
                if (NULL == ppr->pRtpSocket) ppr->fwdTo = NULL;
            }
            ppr++;
        }
        return n;
}

OsStatus NetInTask::destroy()
{
   int wrote;

   // Lock access to write socket.
   getLockObj().acquireWrite();

   // Request shutdown here, so next msg will unblock select() and request
   // will be processed.
   requestShutdown();

   // Write message to socket to release sockets.
   {
      netInTaskMsg msg;
      msg.pRtpSocket = (OsSocket*) -2;
      msg.pRtcpSocket = (OsSocket*) -1;
      msg.fwdTo = NULL;
      msg.notify = NULL;

      wrote = mpWriteSocket->write((char*)&msg, NET_TASK_MAX_MSG_LEN);
   }

   // Close write side of the command socket.
   mpWriteSocket->close();
   delete mpWriteSocket;

   getLockObj().releaseWrite();

   delete this;

   return ((NET_TASK_MAX_MSG_LEN == wrote) ? OS_SUCCESS : OS_BUSY);
}

int NetInTask::run(void *pNotUsed)
{
        fd_set fdset;
        fd_set *fds;
        int     last;
        int     i;
        OsStatus  stat;
        int     numReady;
        netInTaskMsg  msg;
        netInTaskMsg *ppr;
        int     ostc;

        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
            ppr->pRtpSocket =  NULL;
            ppr->pRtcpSocket = NULL;
            ppr->fwdTo = NULL;
            ppr++;
        }
        numPairs = 0;

        fds = &fdset;
        last = OS_INVALID_SOCKET_DESCRIPTOR;
        Zprintf(" *** NetInTask: pipeFd is %d\n",
                       mpReadSocket->getSocketDescriptor(), 0,0,0,0,0);

        while (mpReadSocket && mpReadSocket->isOk()) {
            FD_ZERO(fds);
            FD_SET((unsigned) mpReadSocket->getSocketDescriptor(), fds);
            for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
              if (NULL != ppr->fwdTo) {
                if (NULL != ppr->pRtpSocket)
                {
                  int fd = ppr->pRtpSocket->getSocketDescriptor();
                  ppr->fdRtp = fd;
                  if (fd > 0)
                    FD_SET(fd, fds);
                   else
                   {
                       findPoisonFds(mpReadSocket->getSocketDescriptor());
                       last = OS_INVALID_SOCKET_DESCRIPTOR;
                   }
                }
                if (NULL != ppr->pRtcpSocket)
                {
                  int fd = ppr->pRtcpSocket->getSocketDescriptor();
                  ppr->fdRtcp = fd;
                  if (fd > 0)
                    FD_SET(fd, fds);
                   else
                   {
                       findPoisonFds(mpReadSocket->getSocketDescriptor());
                       last = OS_INVALID_SOCKET_DESCRIPTOR;
                   }
                }
              }
              ppr++;
            }
            if (OS_INVALID_SOCKET_DESCRIPTOR == last) {
               last = mpReadSocket->getSocketDescriptor();
               for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
                  if (NULL != ppr->fwdTo) {
                    if (NULL != ppr->pRtpSocket)
                       last=sipx_max(last, ppr->pRtpSocket->getSocketDescriptor());
                    if (NULL != ppr->pRtcpSocket)
                       last=sipx_max(last, ppr->pRtcpSocket->getSocketDescriptor());
                  }
                  ppr++;
               }
            }
            errno = 0;
            RTL_EVENT("NetInTask.run", 0);
            numReady = select(last+1, fds, NULL, NULL, NULL);
            RTL_EVENT("NetInTask.run", 1);
            ostc = *pOsTC;
            if (0 > numReady) {
                OsSysLog::add(FAC_MP, PRI_ERR, " *** NetInTask: select returned %d, errno=%d=0x%X\n",
                    numReady, errno, errno);
                i = findPoisonFds(mpReadSocket->getSocketDescriptor());
                if (i < 0) {
                    OsSysLog::add(FAC_MP, PRI_ERR, " *** NetInTask: My comm socket failed! Quitting!\n");
                    mpReadSocket->close();
                } else if (0 < i) {
                    last = OS_INVALID_SOCKET_DESCRIPTOR;
                }
/*
                if (selectErrors++ > 10) {
                    OsSysLog::add(FAC_MP, PRI_ERR, " *** NetInTask: Quitting!\n");
                    mpReadSocket->close();
                }
*/
                continue;
            }

#ifdef DEBUG_BY_SUSPEND /* [ */
            if (0 == numReady) {
                Zprintf(" !!! select() returned 0!!!  errno=%d\n",
                    errno, 0,0,0,0,0);
                taskSuspend(0);
                continue;
            }
#endif /* DEBUG_BY_SUSPEND ] */

            /* is it a request to modify the set of file descriptors? */
            if (FD_ISSET(mpReadSocket->getSocketDescriptor(), fds)) {
                numReady--;
                int readBytes;
                
                getLockObj().acquireWrite();
                readBytes = mpReadSocket->read((char *) &msg, NET_TASK_MAX_MSG_LEN);
                getLockObj().releaseWrite();


                if (NET_TASK_MAX_MSG_LEN != readBytes) {
                    OsSysLog::add(FAC_MP, PRI_DEBUG,
                        "NetInTask::run read %d from mpReadSocket socket: %p descriptor: %d errno: %d",
                        readBytes, mpReadSocket, mpReadSocket ? mpReadSocket->getSocketDescriptor() : -111, errno);
                } else if (-2 == (intptr_t) msg.pRtpSocket) {
                    /* request to exit... */
                    Nprintf(" *** NetInTask: closing pipeFd (%d)\n",
                        mpReadSocket->getSocketDescriptor(), 0,0,0,0,0);
                    OsSysLog::add(FAC_MP, PRI_DEBUG, " *** NetInTask: closing pipeFd (%d)\n",
                        mpReadSocket->getSocketDescriptor());
                    getLockObj().acquireWrite();
                    if (mpReadSocket)
                    {
                        mpReadSocket->close();
                        delete mpReadSocket;
                        mpReadSocket = NULL;
                    }
                    getLockObj().releaseWrite();
                } else if (NULL != msg.fwdTo) {
                    if ((NULL != msg.pRtpSocket) || (NULL != msg.pRtcpSocket)) {
                        /* add a new pair of file descriptors */

                        int newRtpFd  = (msg.pRtpSocket)  ? msg.pRtpSocket->getSocketDescriptor()  : -1;
                        int newRtcpFd = (msg.pRtcpSocket) ? msg.pRtcpSocket->getSocketDescriptor() : -1;

                        last = sipx_max(last,newRtpFd);
                        last = sipx_max(last,newRtcpFd);

                        OsSysLog::add(FAC_MP, PRI_DEBUG, " *** NetInTask: Adding new RTP/RTCP sockets (RTP:%p,%d, RTCP:%p,%d)\n",
                                      msg.pRtpSocket, newRtpFd, msg.pRtcpSocket, newRtcpFd);

// I don't know how this can come that sockets are added twice, and I can't see
// any useful use cases for this. Tell me if you know why this is needed.
// -- ipse
//#define CHECK_FOR_DUP_DESCRIPTORS
#ifdef CHECK_FOR_DUP_DESCRIPTORS
                        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
                            if (NULL != ppr->fwdTo) {
                                int existingRtpFd  = (ppr->pRtpSocket)  ? ppr->pRtpSocket->getSocketDescriptor()  : -1;
                                int existingRtcpFd = (ppr->pRtcpSocket) ? ppr->pRtcpSocket->getSocketDescriptor() : -1;
                                UtlBoolean foundDupRtpFd  = FALSE;
                                UtlBoolean foundDupRtcpFd = FALSE;

                                if (existingRtpFd >= 0 &&
                                    (existingRtpFd == newRtpFd || existingRtpFd == newRtcpFd))
                                {
                                    foundDupRtpFd = TRUE;
                                }

                                if (existingRtcpFd >= 0 &&
                                    (existingRtcpFd == newRtpFd || existingRtcpFd == newRtcpFd))
                                {
                                    foundDupRtcpFd = TRUE;
                                }

                                if (foundDupRtpFd || foundDupRtcpFd)
                                {
                                    OsSysLog::add(FAC_MP, PRI_ERR, " *** NetInTask: Using a dup descriptor (New RTP:%p,%d, New RTCP:%p,%d, Old RTP:%p,%d, Old RTCP:%p,%d)\n",
                                                  msg.pRtpSocket, newRtpFd, msg.pRtcpSocket, newRtcpFd, ppr->pRtpSocket, existingRtpFd, ppr->pRtcpSocket, existingRtcpFd);

                                    if (foundDupRtpFd)
                                        ppr->pRtpSocket = NULL;
                                    if (foundDupRtcpFd)
                                        ppr->pRtcpSocket = NULL;
                                    if (ppr->pRtpSocket == NULL && ppr->pRtcpSocket == NULL)
                                        ppr->fwdTo = NULL;
                                }
                            }
                            ppr++;
                        }
#endif
                        // Put this socket pair in the first available array position
                        UtlBoolean newPairAdded = FALSE;
                        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
                            if (NULL == ppr->fwdTo) {
                                ppr->pRtpSocket  = msg.pRtpSocket;
                                ppr->pRtcpSocket = msg.pRtcpSocket;
                                ppr->fwdTo   = msg.fwdTo;
                                numPairs++;

                                // Clear out any packets residing in the socket's
                                // buffer to prevent our dejitter from a burst of
                                // packets on startup.
                                if (msg.pRtpSocket)
                                   flushReadQueue(msg.pRtpSocket);
                                if (msg.pRtcpSocket)
                                   flushReadQueue(msg.pRtcpSocket);

                                OsSysLog::add(FAC_MP, PRI_DEBUG,
                                              " *** NetInTask: Add socket Fds:"
                                              " RTP=%p, RTCP=%p, receiver=%p\n",
                                              msg.pRtpSocket, msg.pRtcpSocket, msg.fwdTo);
                                newPairAdded = TRUE;
                                break;
                            }
                            ppr++;
                        }
                        // Could not find an empty slot
                        if(!newPairAdded)
                        {
                            OsSysLog::add(FAC_MP, PRI_ERR, 
                                    "NetInTask::run no room for more RTP/RTCP (descriptors: %d/%d %p %p) socket pairs in socket array size: %d",
                                    newRtpFd, newRtcpFd, msg.pRtpSocket, msg.pRtcpSocket, NET_TASK_MAX_FD_PAIRS);
                        }

                        if (NULL != msg.notify) {
                            msg.notify->signal(0);
                        }
                    } else {
                        /* remove a pair of file descriptors */
                        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
                            if (msg.fwdTo == ppr->fwdTo) {
                                OsSysLog::add(FAC_MP, PRI_DEBUG,
                                              " *** NetInTask: Remove socket Fds:"
                                              " RTP=%p, RTCP=%p, receiver=%p\n",
                                              ppr->pRtpSocket, ppr->pRtcpSocket, ppr->fwdTo);
                                ppr->pRtpSocket = NULL;
                                ppr->pRtcpSocket = NULL;
                                ppr->fwdTo = NULL;
                                numPairs--;
                                last = OS_INVALID_SOCKET_DESCRIPTOR;
                                break;
                            }
                            ppr++;
                        }
                        if (NULL != msg.notify) {
                            msg.notify->signal(0);
                        }
                    }
                }
                else // NULL FromNet, not good
                {
                    osPrintf("NetInTask::run msg with NULL FromNet\n");
                }
            }
            ppr=pairs;
            for (i=0; ((i<NET_TASK_MAX_FD_PAIRS)&&(numReady>0)); i++) {
                int tfd;
                if (NULL != ppr->pRtpSocket) {
                  tfd = ppr->pRtpSocket->getSocketDescriptor();
                  // assert(ppr->fdRtp == tfd);
                  if ((-1 < tfd) && (FD_ISSET(tfd, fds))) {
                    stat = get1Msg(ppr->pRtpSocket, ppr->fwdTo, false, ostc);
                    if (OS_SUCCESS != stat) {
                        OsSysLog::add(FAC_MP, PRI_ERR, 
                            " *** NetInTask: removing RTP#%ld pSkt=%p due"
                            " to read error.\n", ppr-pairs,
                            ppr->pRtpSocket);
                        last = OS_INVALID_SOCKET_DESCRIPTOR;
                        ppr->pRtpSocket = NULL;
                        if (NULL == ppr->pRtcpSocket) ppr->fwdTo = NULL;
                    }
                    numReady--;
                  }
                }
                if (NULL != ppr->pRtcpSocket) {
                  tfd = ppr->pRtcpSocket->getSocketDescriptor();
                  // assert(ppr->fdRtcp == tfd);
                  if ((-1 < tfd) && (FD_ISSET(tfd, fds))) {
                    stat = get1Msg(ppr->pRtcpSocket, ppr->fwdTo, true, ostc);
                    if (OS_SUCCESS != stat) {
                        OsSysLog::add(FAC_MP, PRI_ERR, 
                            " *** NetInTask: removing RTCP#%ld pSkt=%p due"
                            " to read error.\n", ppr-pairs,
                            ppr->pRtcpSocket);
                        last = OS_INVALID_SOCKET_DESCRIPTOR;
                        ppr->pRtcpSocket = NULL;
                        if (NULL == ppr->pRtpSocket) ppr->fwdTo = NULL;
                    }
                    numReady--;
                  }
                }
                ppr++;
            }
        }

        OsSysLog::add(FAC_MP, PRI_DEBUG, 
                "NetInTask::run exiting mpReadSocket: %p mpReadSocket->isOk() = %s",
                mpReadSocket, mpReadSocket ? (mpReadSocket->isOk() ? "true" : "false") : "N/A");
        return 0;
}

NetInTask* NetInTask::getNetInTask()
{
   UtlBoolean isStarted;

   // If the task object already exists, and the corresponding low-level task
   // has been started, then use it
   if (spInstance != NULL && spInstance->isStarted())
      return spInstance;

   // If the task does not yet exist or hasn't been started, then acquire
   // the lock to ensure that only one instance of the task is started
   getLockObj().acquireWrite();
   if (spInstance == NULL) {
       spInstance = new NetInTask();
   }
   isStarted = spInstance->isStarted();
   if (!isStarted)
   {
      isStarted = spInstance->start();
      assert(isStarted);
   }
   getLockObj().releaseWrite();

   // Synchronize with NetInTask startup
   int numDelays = 0;
   while (spInstance->mCmdPort == -1)
   {
       OsTask::delay(20) ;
       numDelays++;
       if((numDelays % 50) == 0) osPrintf("NetInTask::getNetInTask %d delays\n", numDelays);
   }
   return spInstance;
}

NetInTask::NetInTask(int prio, int options, int stack)
:  OsTask("NetInTask", NULL, prio, options, stack),
   mpWriteSocket(NULL),
   mpReadSocket(NULL),
   mEventMutex(0)
{
    // Create temporary listening socket.
    OsServerSocket *pBindSocket = new OsServerSocket(1, PORT_DEFAULT, "127.0.0.1");
    RTL_EVENT("NetInTask::NetInTask", 1);
    mCmdPort = pBindSocket->getLocalHostPort();
    assert(-1 != mCmdPort);

    // Start our helper thread to go open the socket
    RTL_EVENT("NetInTask::NetInTask", 2);
    NetInTaskHelper* pHelper = new NetInTaskHelper(mCmdPort);
    if (!pHelper->isStarted()) {
       RTL_EVENT("NetInTask::NetInTask", 3);
       pHelper->start();
    }

    RTL_EVENT("NetInTask::NetInTask", 4);
    mpReadSocket = pBindSocket->accept();
    mpReadSocket->makeNonblocking();

    RTL_EVENT("NetInTask::NetInTask", 5);
    pBindSocket->close();

    RTL_EVENT("NetInTask::NetInTask", 6);
    delete pBindSocket;

    // Create socket for write side of connection.
    mpWriteSocket = pHelper->getSocket();
    assert(mpWriteSocket != NULL && mpWriteSocket->isConnected());

    RTL_EVENT("NetInTask::NetInTask", 7);
    delete pHelper;
}

// Destructor
NetInTask::~NetInTask()
{
   waitUntilShutDown();
   spInstance = NULL;
}

OsStatus NetInTask::addNetInputSources(OsSocket* pRtpSocket, OsSocket* pRtcpSocket,
                                       MprFromNet* fwdTo, OsNotification* notify)
{
   netInTaskMsg msg;
   memset(&msg, 0, NET_TASK_MAX_MSG_LEN);
   int wrote = 0;

   if (NULL != fwdTo)
   {
      pRtpSocket->makeNonblocking();
      msg.pRtpSocket = pRtpSocket;
      pRtcpSocket->makeNonblocking();
      msg.pRtcpSocket = pRtcpSocket;
      msg.fwdTo = fwdTo;
      msg.notify = notify;

      getLockObj().acquireWrite();
      wrote = mpWriteSocket->write((char*)&msg, NET_TASK_MAX_MSG_LEN);
      getLockObj().releaseWrite();

      if (wrote != NET_TASK_MAX_MSG_LEN)
      {
         OsSysLog::add(FAC_MP, PRI_ERR,
                       "addNetInputSources - writeSocket error: %p,%d wrote %d",
                       mpWriteSocket, mpWriteSocket->getSocketDescriptor(),
                       wrote);
      }
   }
   else
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "addNetInputSources - fwdTo is NULL");
      assert(!"NetInTask::addNetInputSources(): fwdTo is NULL!");
   }
   return ((NET_TASK_MAX_MSG_LEN == wrote) ? OS_SUCCESS : OS_BUSY);
}

OsStatus NetInTask::removeNetInputSources(MprFromNet* fwdTo, OsNotification* notify)
{
   netInTaskMsg msg;
   int wrote = 0;

   memset((void*)&msg, 0, NET_TASK_MAX_MSG_LEN);
   if (NULL != fwdTo)
   {
      msg.fwdTo = fwdTo;
      msg.notify = notify;

      getLockObj().acquireWrite();
      wrote = mpWriteSocket->write((char*)&msg, NET_TASK_MAX_MSG_LEN);
      getLockObj().releaseWrite();

      if (wrote != NET_TASK_MAX_MSG_LEN)
      {
         OsSysLog::add(FAC_MP, PRI_ERR,
                       "removeNetInputSources - writeSocket error: %p,%d wrote %d",
                       mpWriteSocket, mpWriteSocket->getSocketDescriptor(),
                       wrote);
      }
   }
   else
   {
      OsSysLog::add(FAC_MP, PRI_ERR,
                    "removeNetInputSources - fwdTo is NULL");
      assert(!"NetInTask::removeNetInputSources(): fwdTo is NULL!");
   }

   return ((NET_TASK_MAX_MSG_LEN == wrote) ? OS_SUCCESS : OS_BUSY);
}

/************************************************************************/

// return something random (32 bits)
uint32_t rand_timer32()
{
#ifdef _VXWORKS /* [ */
// On VxWorks, this is based on reading the 3.686400 MHz counter
        uint32_t x;
static  uint32_t last_timer = 0x12345678;

        x = *pOsTC;
        if (x == last_timer) {
                SEM_ID s;
                s = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
                semTake(s, 0);
                semDelete(s);
                x = *pOsTC;
        }
        last_timer = x;
        /* Rotate left 8 */
        return (((x&0xFF)<<8) | ((x&0xFFFF00)<<16) | ((x&0xFF000000)>>24));
#endif /* _VXWORKS ] */

#if defined(_WIN32) || defined(__pingtel_on_posix__) /* [ */
// Otherwise, call rand() 3 times, using 12 or 8 bits from each call (15 max)
        static int firstTime = 1;
        static UtlRandom* spNetInRandom = NULL;
        uint32_t x, y, z;
        uint32_t ret;

        if (firstTime) {
            spNetInRandom = new UtlRandom();
            assert(RAND_MAX > 0xfff);
            firstTime = 0;
        }
        x = spNetInRandom->rand();
        y = spNetInRandom->rand();
        z = spNetInRandom->rand();
        ret = ((x&0xFFF) | ((y<<12)&0xFFF000) | ((z<<24)&0xFF000000));
        // OsSysLog::add(FAC_MP, PRI_DEBUG, "rand_timer32(): x=0x%X=%d, y=0x%X=%d, z=0x%X=%d, ret=0x%08X", x, x, y, y, z, z, ret);
        return ret;
#endif /* _WIN32 || __pingtel_on_posix__ ] */
}

#define RTP_DIR_NEW 4

rtpHandle StartRtpSession(OsSocket* socket, int direction, char type)
{
        struct rtpSession *ret;
        RtpSeq rseq;

        rseq = 0xFFFF & rand_timer32();

        ret = (struct rtpSession *) malloc(sizeof(struct rtpSession));
        if (ret) {
                ret->vpxcc = ((2<<6) | (0<<5) | (0<<4) | 0);
                ret->mpt = ((0<<7) | (type & 0x7f));
                ret->seq = rseq;
                /* ret->timestamp = rand_timer32(); */
#ifdef INCLUDE_RTCP /* [ */
                ret->ssrc = 0;   // Changed by DMG.  SSRC now generated in MpFlowGraph
#else /* INCLUDE_RTCP ] [ */
                ret->ssrc = rand_timer32();
#endif /* INCLUDE_RTCP ] */
                ret->dir = direction | RTP_DIR_NEW;
                ret->socket = socket;
                ret->packets = 0;
                ret->octets = 0;
                ret->cycles = 0;
        }
        return ret;
}

void FinishRtpSession(rtpHandle h)
{
        if (NULL != h) {
            h->socket = NULL;
            free(h);
        }
}

/* ============================ FUNCTIONS ================================= */

