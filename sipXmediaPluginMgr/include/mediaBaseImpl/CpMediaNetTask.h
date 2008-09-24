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

#ifndef _INCLUDED_CpMediaNetTask_H /* [ */
#define _INCLUDED_CpMediaNetTask_H

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/qsTypes.h"
#include "os/OsTask.h"
#include "os/OsLock.h"
#include "os/OsSocket.h"
#include "os/OsRWMutex.h"
#include "os/OsProtectEvent.h"

class OsNotification;
class CpMediaNetTask ;

// DEFINES

#define RTP_DIR_IN  1
#define RTP_DIR_OUT 2
#define RTP_DIR_NEW 4

#define RTCP_DIR_IN  1
#define RTCP_DIR_OUT 2
#define RTCP_DIR_NEW 4

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS

class OsConnectionSocket;
class OsServerSocket;
class OsSocket;
class IMediaSocket ;

#ifdef foo
struct rtpHeader {
        UCHAR vpxcc;
        UCHAR mpt;
        USHORT seq;      /* Big Endian! */
        UINT timestamp;  /* Big Endian! */
        UINT ssrc;       /* Big Endian, but random */
};

struct rtpSession {
        UCHAR vpxcc; /* Usually: ((2<<6) | (0<<5) | (0<<4) | 0) */
        UCHAR mpt;   /* Usually: ((0<<7) | 0) */
        USHORT seq;
        UINT timestamp;
        UINT ssrc;
        OsSocket* socket;
        int dir;
        UINT packets;
        UINT octets;
        USHORT cycles;
};
#endif

typedef struct rtpSession *rtpHandle;
typedef struct rtcpSession *rtcpHandle;

// FORWARD DECLARATIONS
// extern UINT rand_timer32(void);

extern OsStatus startCpMediaNetTask();
extern OsStatus shutdownCpMediaNetTask();
/*
extern OsStatus addNetInputSources(OsSocket* pRtpSocket,
            OsSocket* pRtcpSocket, MprFromNet* fwdTo, OsNotification* note);
extern OsStatus removeNetInputSources(MprFromNet* fwdTo, OsNotification* note);
*/
class CpMediaNetTask : public OsTask
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static const int DEF_NET_IN_TASK_PRIORITY;      // default task priority
   static const int DEF_NET_IN_TASK_OPTIONS;       // default task options
   static const int DEF_NET_IN_TASK_STACKSIZE;     // default task stacksize

/* ============================ CREATORS ================================== */

   static CpMediaNetTask* getCpMediaNetTask();
     //:Return a pointer to the NetIn task

   static CpMediaNetTask* createCpMediaNetTask();
     //:Return a newly created pointer to the NetIn task, or an existing instance

   virtual
   ~CpMediaNetTask();
     //:Destructor
   
/* ============================ MANIPULATORS ============================== */
    virtual int run(void* pArg);

    virtual OsStatus addInputSource(IMediaSocket* pSocket) ;
    virtual OsStatus removeInputSource(IMediaSocket* pSocket, OsProtectedEvent* pEvent = NULL) ;

/* ============================ ACCESSORS ================================= */

   OsConnectionSocket* getWriteSocket(void);
   OsConnectionSocket* getReadSocket(void);

   // void openWriteFD(void);

   static OsRWMutex& getLockObj() { return sLock; }

/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   CpMediaNetTask(
      int prio    = DEF_NET_IN_TASK_PRIORITY,      // default task priority
      int options = DEF_NET_IN_TASK_OPTIONS,       // default task options
      int stack   = DEF_NET_IN_TASK_STACKSIZE);    // default task stacksize
     //:Default constructor

   int processControlSocket(int last, bool* pbShutdown) ;


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   // Static data members used to enforce Singleton behavior
   OsConnectionSocket* createWriteSocket(void);
   static CpMediaNetTask* spInstance;    // pointer to the single instance of
                                    //  the MpCpMediaNetTask class
   static OsRWMutex     sLock;         // semaphore used to ensure that there
                                    //  is only one instance of this class

   OsConnectionSocket* mpWriteSocket;
   OsConnectionSocket* mpReadSocket;

   int               mCmdPort;      // internal socket port number

   CpMediaNetTask(const CpMediaNetTask& rCpMediaNetTask);
     //:Copy constructor (not implemented for this task)

   CpMediaNetTask& operator=(const CpMediaNetTask& rhs);
     //:Assignment operator (not implemented for this task)
};

#endif /* _INCLUDED_CpMediaNetTaskH ] */

