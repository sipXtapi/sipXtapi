//  
// Copyright (C) 2006-2016 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _INCLUDED_NETINTASK_H /* [ */
#define _INCLUDED_NETINTASK_H

#include "rtcp/RtcpConfig.h"

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsTask.h"
#include "os/OsLock.h"
#include "os/OsSocket.h"
#include "os/OsRWMutex.h"
#include "os/OsMutex.h"
#include "mp/MpTypes.h"
#include "mp/MpRtpBuf.h"
#include "mp/MpMisc.h"

// DEFINES
#define IP_HEADER_SIZE  20    ///< Size of IP packet header
#define UDP_HEADER_SIZE 8     ///< Size of UDP packet header
#define ETHERNET_MTU    1500  ///< Maximum Transmission Unit for Ethernet frame
#define UDP_MTU  (ETHERNET_MTU - IP_HEADER_SIZE - UDP_HEADER_SIZE)
                              ///< Maximum Transmission Unit for UDP packet.
#define RTP_MTU  (UDP_MTU-12) ///< Maximum Transmission Unit for RTP packet.
#define RTCP_MTU (UDP_MTU-12)

#define RTP_DIR_IN  1
#define RTP_DIR_OUT 2
#define RTP_DIR_NEW 4

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class MprFromNet;
class OsConnectionSocket;
class OsServerSocket;
class OsSocket;
class OsNotification;
struct rtpSession;

// STRUCTS
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

#ifndef INCLUDE_RTCP /* [ */
struct __MprRtcpStats {
   RtpSRC ssrc;
   short seqNumCycles;
   uint16_t highSeqNum;
};

// TYPEDEFS

typedef struct __MprRtcpStats  MprRtcpStats;
typedef struct __MprRtcpStats* MprRtcpStatsPtr;
#endif /* INCLUDE_RTCP ] */

typedef struct rtpSession *rtpHandle;

// FORWARD DECLARATIONS
extern uint32_t rand_timer32();
extern rtpHandle StartRtpSession(OsSocket* socket, int direction, char type);
extern void FinishRtpSession(rtpHandle h);

/**
*  @brief Task that listen for packets in incoming RTP streams.
*
*  @nosubgrouping
*/
class NetInTask : public OsTask
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static const int DEF_NET_IN_TASK_PRIORITY;      ///< default task priority
   static const int DEF_NET_IN_TASK_OPTIONS;       ///< default task options
   static const int DEF_NET_IN_TASK_STACKSIZE;     ///< default task stacksize

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Return a pointer to the NetIn task, creating it if necessary
   static NetInTask* getNetInTask();

     /// Shutdown NetInTask instance and free it.
   OsStatus destroy();

     /// Destructor
   virtual ~NetInTask();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Task loop
   virtual int run(void* pArg);

   OsStatus addNetInputSources(OsSocket* pRtpSocket,
                               OsSocket* pRtcpSocket,
                               MprFromNet* fwdTo,
                               OsNotification* note);

   OsStatus removeNetInputSources(MprFromNet* fwdTo, OsNotification* note);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   // Static data members used to enforce Singleton behavior
   static NetInTask* spInstance;    ///< pointer to the single instance of
                                    ///<  the MpNetInTask class
   static OsRWMutex  sLock;         ///< semaphore used to ensure that there
                                    ///<  is only one instance of this class

   OsConnectionSocket* mpWriteSocket;
   OsConnectionSocket* mpReadSocket;
   int                 mCmdPort;    ///< internal socket port number
   OsMutex             mEventMutex;

     /// Default constructor
   NetInTask(
      int prio    = DEF_NET_IN_TASK_PRIORITY,      ///< default task priority
      int options = DEF_NET_IN_TASK_OPTIONS,       ///< default task options
      int stack   = DEF_NET_IN_TASK_STACKSIZE);    ///< default task stack size

     /// Return sLock object.
   static OsRWMutex& getLockObj() { return sLock; }

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor (not implemented for this task)
   NetInTask(const NetInTask& rNetInTask);

     /// Assignment operator (not implemented for this task)
   NetInTask& operator=(const NetInTask& rhs);

};

#endif /* _INCLUDED_NETINTASKH ] */
