//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
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

#define LOGEM
#undef LOGEM

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
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/time.h>
#endif /* __pingtel_on_posix__ ] */

// APPLICATION INCLUDES

#include "os/OsDefs.h"
#include "os/OsTask.h"
#include "os/OsServerSocket.h"
#include "os/OsConnectionSocket.h"
#include "os/OsEvent.h"
#include "mp/NetInTask.h"
#include "mp/MpUdpBuf.h"
#include "mp/MprFromNet.h"
#include "mp/MpBufferMsg.h"
#include "mp/dmaTask.h"
#ifdef _VXWORKS /* [ */
#ifdef CPU_XSCALE /* [ */
#include "mp/pxa255.h"
#define OSTIMER_COUNTER_POINTER ((int*) PXA250_OSTIMER_OSCR)
#else /* CPU_XSCALE ] [ */
#include "mp/sa1100.h"
#define OSTIMER_COUNTER_POINTER ((int*) SA1100_OSTIMER_COUNTER)
#endif /* CPU_XSCALE ] */
#else /* _VXWORKS ][ */
#define OSTIMER_COUNTER_POINTER (&dummy0)
static int dummy0 = 0;
#endif /* _VXWORKS ] */

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

struct __netInTaskMsg {
   OsSocket* pRtpSocket;
   OsSocket* pRtcpSocket;
   MprFromNet* fwdTo;
   OsNotification* notify;
};

typedef struct __netInTaskMsg netInTaskMsg, *netInTaskMsgPtr;

struct rtpSession {
   uint8_t vpxcc; ///< Usually: ((2<<6) | (0<<5) | (0<<4) | 0)
   uint8_t mpt;   ///< Usually: ((0<<7) | 0)
   RtpSeq seq;
   RtpTimestamp timestamp;
   RtpSRC ssrc;
   OsSocket* socket;
   int dir;
   uint32_t packets;
   uint32_t octets;
   uint16_t cycles;
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
const int NetInTask::DEF_NET_IN_TASK_STACKSIZE = 4096;// default task stacksize
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
: OsTask("NetInTaskHelper-%d", NULL, 25, 0, 2000)
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
    while (numTry < 1000)
    {
       // Delay for some time on consecutive runs.
       if (numTry > 0)
       {
          OsTask::delay(1);
       }

       // Connect...
       mpSocket = new OsConnectionSocket(mPort, "127.0.0.1");
       if (mpSocket && mpSocket->isConnected()) break;

       numTry++;
    }

    OsSysLog::add(FAC_MP, PRI_INFO,
       "NetInTaskHelper::run()... returning 0, after %d tries\n", numTry);

    return 0;
}

OsConnectionSocket* NetInTaskHelper::getSocket()
{
   waitUntilShutDown();
   return mpSocket;
}

/************************************************************************/

int NetInTask::getWriteFD()
{
    int writeSocketDescriptor = -1;
    if (NULL != mpWriteSocket)
    {
        writeSocketDescriptor = mpWriteSocket->getSocketDescriptor();
    }
    else
    {
       assert(FALSE);
    }
    return(writeSocketDescriptor);
}

OsConnectionSocket* NetInTask::getWriteSocket()
{
    if (NULL == mpWriteSocket) {
        getWriteFD();
    }
    return mpWriteSocket;
}

/************************************************************************/

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
                fwdTo->pushPacket(ib, isRtcp);
            } 
            else 
            {
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
            if ((nRead < 1) && !pRxpSkt->isOk()) 
            {
                return OS_NO_MORE_DATA;
            }
        }
        return OS_SUCCESS;
}

static int selectErrors;

int isFdPoison(int fd)
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
        FD_SET((unsigned) fd, fds);
        numReady = select(fd+1, fds, NULL, NULL, ptv);
        return (0 > numReady) ? TRUE : FALSE;
}

int findPoisonFds(int pipeFD)
{
        int i;
        int n = 0;
        netInTaskMsgPtr ppr;

        if (isFdPoison(pipeFD)) {
            OsSysLog::add(FAC_MP, PRI_ERR, " *** NetInTask: pipeFd socketDescriptor=%d busted!\n", pipeFD);
            return -1;
        }
        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
            if (ppr->pRtpSocket && // not NULL socket and
                isFdPoison(ppr->pRtpSocket->getSocketDescriptor())) {
                OsSysLog::add(FAC_MP, PRI_ERR, " *** NetInTask: Removing fdRtp[%d], socket=0x%08x, socketDescriptor=%d\n", ppr-pairs,(int)ppr->pRtpSocket, (int)ppr->pRtpSocket->getSocketDescriptor());
                n++;
                ppr->pRtpSocket = NULL;
                if (NULL == ppr->pRtcpSocket) ppr->fwdTo = NULL;
            }
            if (ppr->pRtcpSocket && // not NULL socket and
                isFdPoison(ppr->pRtcpSocket->getSocketDescriptor())) {
                OsSysLog::add(FAC_MP, PRI_ERR, " *** NetInTask: Removing fdRtcp[%d], socket=0x%08x, socketDescriptor=%d\n", ppr-pairs,(int)ppr->pRtcpSocket, (int)ppr->pRtcpSocket->getSocketDescriptor());
                n++;
                ppr->pRtcpSocket = NULL;
                if (NULL == ppr->pRtpSocket) ppr->fwdTo = NULL;
            }
            ppr++;
        }
        return n;
}

static volatile int selectCounter = 0;
int whereSelectCounter()
{
    int save;
    save = selectCounter;
    selectCounter = 0;
    return save;
}

int showNetInTable() {
   int     last = 1234567;
   int     pipeFd;
   int     i;
   netInTaskMsgPtr ppr;
   NetInTask* pInst = NetInTask::getNetInTask();

   pipeFd = pInst->getWriteFD();

   for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
      if (NULL != ppr->fwdTo) {
         if (NULL != ppr->pRtpSocket)
            last=max(last, ppr->pRtpSocket->getSocketDescriptor());
         if (NULL != ppr->pRtcpSocket)
            last=max(last, ppr->pRtcpSocket->getSocketDescriptor());
      }
      ppr++;
   }
   Zprintf("pipeFd = %d (last = %d)\n", pipeFd, last, 0,0,0,0);
   for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
      if (NULL != ppr->fwdTo) {
         Zprintf(" %2d: MprFromNet=0x%X, pRtpSocket: 0x%x, pRtcpSocket: 0x%x\n",
            i, (int) (ppr->fwdTo), (int) (ppr->pRtpSocket),
            (int) (ppr->pRtcpSocket), 0,0);
      }
      ppr++;
   }
   return last;
}

int NetInTask::run(void *pNotUsed)
{
        fd_set fdset;
        fd_set *fds;
        int     last;
        int     i;
        OsStatus  stat;
        int     numReady;
        netInTaskMsg    msg;
        netInTaskMsgPtr ppr;
        int     ostc;

        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
            ppr->pRtpSocket =  NULL;
            ppr->pRtcpSocket = NULL;
            ppr->fwdTo = NULL;
            ppr++;
        }
        numPairs = 0;

        selectErrors = 0;

        fds = &fdset;
        last = OS_INVALID_SOCKET_DESCRIPTOR;
        Zprintf(" *** NetInTask: pipeFd is %d\n",
                       mpReadSocket->getSocketDescriptor(), 0,0,0,0,0);

        while (mpReadSocket && mpReadSocket->isOk()) {
            if (OS_INVALID_SOCKET_DESCRIPTOR == last) {
               last = mpReadSocket->getSocketDescriptor();
               for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
                  if (NULL != ppr->fwdTo) {
                    if (NULL != ppr->pRtpSocket)
                       last=max(last, ppr->pRtpSocket->getSocketDescriptor());
                    if (NULL != ppr->pRtcpSocket)
                       last=max(last, ppr->pRtcpSocket->getSocketDescriptor());
                  }
                  ppr++;
               }
            }
            FD_ZERO(fds);
            FD_SET((unsigned) mpReadSocket->getSocketDescriptor(), fds);
            for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
              if (NULL != ppr->fwdTo) {
                if (NULL != ppr->pRtpSocket)
                {
                  int fd = ppr->pRtpSocket->getSocketDescriptor();
                  if (fd > 0)
                    FD_SET(fd, fds);
                }
                if (NULL != ppr->pRtcpSocket)
                {
                  int fd = ppr->pRtcpSocket->getSocketDescriptor();
                  if (fd > 0)
                    FD_SET(fd, fds);
                }
              }
              ppr++;
            }
            errno = 0;
            numReady = select(last+1, fds, NULL, NULL, NULL);
            ostc = *pOsTC;
            selectCounter++;
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
                if (NET_TASK_MAX_MSG_LEN !=
                     mpReadSocket->read((char *) &msg, NET_TASK_MAX_MSG_LEN)) {
                    osPrintf("NetInTask::run: Invalid request!\n");
                } else if (-2 == (int) msg.pRtpSocket) {
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
                        last = max(last,msg.pRtpSocket->getSocketDescriptor());
                        last = max(last,msg.pRtcpSocket->getSocketDescriptor());
#define CHECK_FOR_DUP_DESCRIPTORS
#ifdef CHECK_FOR_DUP_DESCRIPTORS
                        int newRtpFd  = (msg.pRtpSocket)  ? msg.pRtpSocket->getSocketDescriptor()  : -1;
                        int newRtcpFd = (msg.pRtcpSocket) ? msg.pRtcpSocket->getSocketDescriptor() : -1;

                        OsSysLog::add(FAC_MP, PRI_DEBUG, " *** NetInTask: Adding new RTP/RTCP sockets (RTP:%p,%d, RTCP:%p,%d)\n",
                                      msg.pRtpSocket, newRtpFd, msg.pRtcpSocket, newRtcpFd);

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
                        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
                            if (NULL == ppr->fwdTo) {
                                ppr->pRtpSocket  = msg.pRtpSocket;
                                ppr->pRtcpSocket = msg.pRtcpSocket;
                                ppr->fwdTo   = msg.fwdTo;
                                i = NET_TASK_MAX_FD_PAIRS;
                                numPairs++;
                                OsSysLog::add(FAC_MP, PRI_DEBUG, " *** NetInTask: Add socket Fds: RTP=%p, RTCP=%p, Q=%p\n",
                                              msg.pRtpSocket, msg.pRtcpSocket, msg.fwdTo);
                            }
                            ppr++;
                        }
                        if (NULL != msg.notify) {
                            msg.notify->signal(0);
                        }
                    } else {
                        /* remove a pair of file descriptors */
                        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
                            if (msg.fwdTo == ppr->fwdTo) {
                                OsSysLog::add(FAC_MP, PRI_DEBUG, " *** NetInTask: Remove socket Fds: RTP=%p, RTCP=%p, Q=%p\n",
                                              ppr->pRtpSocket, ppr->pRtcpSocket, ppr->fwdTo);
                                ppr->pRtpSocket = NULL;
                                ppr->pRtcpSocket = NULL;
                                ppr->fwdTo = NULL;
                                numPairs--;
                                last = -1;
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
                if ((NULL != ppr->pRtpSocket) &&
                  (FD_ISSET(ppr->pRtpSocket->getSocketDescriptor(), fds))) {
                    stat = get1Msg(ppr->pRtpSocket, ppr->fwdTo,
                       false, ostc);
                    if (OS_SUCCESS != stat) {
                        Zprintf(" *** NetInTask: removing RTP#%d pSkt=0x%x due"
                            " to read error.\n", ppr-pairs,
                            (int) ppr->pRtpSocket, 0,0,0,0);
                        if (last == ppr->pRtpSocket->getSocketDescriptor())
                           last = OS_INVALID_SOCKET_DESCRIPTOR;
                        ppr->pRtpSocket = NULL;
                        if (NULL == ppr->pRtcpSocket) ppr->fwdTo = NULL;
                    }
                    numReady--;
                }
                if ((NULL != ppr->pRtcpSocket) &&
                  (FD_ISSET(ppr->pRtcpSocket->getSocketDescriptor(), fds))) {
                    stat = get1Msg(ppr->pRtcpSocket, ppr->fwdTo,
                       true, ostc);
                    if (OS_SUCCESS != stat) {
                        Zprintf(" *** NetInTask: removing RTCP#%d pSkt=0x%x due"
                            " to read error.\n", ppr-pairs,
                            (int) ppr->pRtcpSocket, 0,0,0,0);
                        if (last == ppr->pRtcpSocket->getSocketDescriptor())
                           last = OS_INVALID_SOCKET_DESCRIPTOR;
                        ppr->pRtcpSocket = NULL;
                        if (NULL == ppr->pRtpSocket) ppr->fwdTo = NULL;
                    }
                    numReady--;
                }
                ppr++;
            }
        }
        return 0;
}

/* possible args are: buffer pool to take from, max message size */

OsStatus startNetInTask()
{
        NetInTask *pTask;

        pTask = NetInTask::getNetInTask();
        return (NULL != pTask) ? OS_SUCCESS : OS_TASK_NOT_STARTED;
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
   getLockObj().acquireRead();
   if (spInstance == NULL) {
       spInstance = new NetInTask();
   }
   isStarted = spInstance->isStarted();
   if (!isStarted)
   {
      isStarted = spInstance->start();
      assert(isStarted);
   }
   getLockObj().releaseRead();

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

void NetInTask::shutdownSockets()
{
   getLockObj().acquireWrite();
   if (mpWriteSocket)
   {
      mpWriteSocket->close();
      delete mpWriteSocket;
      mpWriteSocket = NULL;
   }
   getLockObj().releaseWrite();

}
// Default constructor (called only indirectly via getNetInTask())
NetInTask::NetInTask(int prio, int options, int stack)
:  OsTask("NetInTask", NULL, prio, options, stack),
   mpWriteSocket(NULL),
   mpReadSocket(NULL)
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

// $$$ These messages need to contain OsSocket* instead of fds.

OsStatus shutdownNetInTask()
{
   int wrote;

   // Lock access to write socket.
   NetInTask::getLockObj().acquireWrite();

   NetInTask* pInst = NetInTask::getNetInTask();

   // Request shutdown here, so next msg will unblock select() and request
   // will be processed.
   pInst->requestShutdown();

   // Write message to socket to release sockets.
   {
      OsConnectionSocket* writeSocket = pInst->getWriteSocket();

      netInTaskMsg msg;
      msg.pRtpSocket = (OsSocket*) -2;
      msg.pRtcpSocket = (OsSocket*) -1;
      msg.fwdTo = NULL;
      msg.notify = NULL;

      wrote = writeSocket->write((char *) &msg, NET_TASK_MAX_MSG_LEN);
   }

   NetInTask::getLockObj().releaseWrite();

   pInst->shutdownSockets();

   delete pInst;

   return ((NET_TASK_MAX_MSG_LEN == wrote) ? OS_SUCCESS : OS_BUSY);
}

// $$$ This is quite Unix-centric; on Win/NT these are handles...
/* neither fd can be < -1, at least one must be > -1, and fwdTo non-NULL */

OsStatus addNetInputSources(OsSocket* pRtpSocket, OsSocket* pRtcpSocket,
      MprFromNet* fwdTo, OsNotification* notify)
{
        netInTaskMsg msg;
        int wrote = 0;
        NetInTask* pInst = NetInTask::getNetInTask();
        OsConnectionSocket* writeSocket;

        // pInst->getWriteFD();
        writeSocket = pInst->getWriteSocket();

        if (NULL != fwdTo) {
            msg.pRtpSocket = pRtpSocket;
            msg.pRtcpSocket = pRtcpSocket;
            msg.fwdTo = fwdTo;
            msg.notify = notify;

            wrote = writeSocket->write((char *) &msg, NET_TASK_MAX_MSG_LEN);
            if (wrote != NET_TASK_MAX_MSG_LEN)
            {
                OsSysLog::add(FAC_MP, PRI_ERR,
                    "addNetInputSources - writeSocket error: 0x%08x,%d wrote %d",
                    (int)writeSocket, writeSocket->getSocketDescriptor(), wrote);
            }
        }
        else
        {
            osPrintf("fwdTo NULL\n");
        }
        return ((NET_TASK_MAX_MSG_LEN == wrote) ? OS_SUCCESS : OS_BUSY);
}

OsStatus removeNetInputSources(MprFromNet* fwdTo, OsNotification* notify)
{
        netInTaskMsg msg;
        int wrote = NET_TASK_MAX_MSG_LEN;
        NetInTask* pInst = NetInTask::getNetInTask();
        OsConnectionSocket* writeSocket;

        // pInst->getWriteFD();
        writeSocket = pInst->getWriteSocket();

        if (NULL != fwdTo) {
            msg.pRtpSocket = NULL;
            msg.pRtcpSocket = NULL;
            msg.fwdTo = fwdTo;
            msg.notify = notify;
            wrote = writeSocket->write((char *) &msg, NET_TASK_MAX_MSG_LEN);
            if (wrote != NET_TASK_MAX_MSG_LEN)
            {
                OsSysLog::add(FAC_MP, PRI_ERR,
                    "removeNetInputSources - writeSocket error: 0x%08x,%d wrote %d",
                    (int)writeSocket, writeSocket->getSocketDescriptor(), wrote);
            }
        }

        return ((NET_TASK_MAX_MSG_LEN == wrote) ? OS_SUCCESS : OS_BUSY);
}

#define LOGEM
#undef LOGEM

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
        unsigned int x, y, z;

        if (firstTime) {
            assert(RAND_MAX > 0xfff);
            srand((unsigned)time(NULL));
            firstTime = 0;
        }
        x = rand();
        y = rand();
        z = rand();
        return ((x&0xFFF) | ((y<<12)&0xFFF000) | ((z<<24)&0xFF000000));
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

