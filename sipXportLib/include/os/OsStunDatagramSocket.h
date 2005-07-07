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

#ifndef _OsStunDatagramSocket_h_
#define _OsStunDatagramSocket_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDatagramSocket.h"
#include "os/OsMsgQ.h"
#include "os/OsTimer.h"

// DEFINES
#define STUN_TIMEOUT_RESPONSE_MS                500  /**< Wait at most 500ms for a stun response    */
#define STUN_INITIAL_REFRESH_REPORT_THRESHOLD   2    /**< First complain after failing N times */
#define STUN_REFRESH_REPORT_THRESHOLD           120  /**< After success, complain after N times */
#define STUN_ABORT_THRESHOLD                    60   /**< Abort after failing to keep-alive N times 
                                                          if we never succeeded. */


// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class StunRefreshEvent ;
class StunMsg ;

//: OsStunDatagramSocket creates a OsDatagramSocket and automatically initiates
//: stun lookups and keep-alives.  
//
// Note: The STUN refreshes will not work properly unless someone is 
//       constantly draining the socket.
class OsStunDatagramSocket : public OsDatagramSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
    OsStunDatagramSocket(int remoteHostPort, 
                         const char* remoteHostName, 
                         int localHostPort = 0, 
                         const char* localHostName = NULL,
                         bool bEnable = TRUE,
                         const char* szStunServer = "larry.gloo.net",
                         int iRefreshPeriodInSec = 28) ;

      //: Constructor accepting remote host port, name and optional local
      //: host name and port, and stun attributes.

    virtual ~OsStunDatagramSocket();
       //:Destructor

/* ============================ MANIPULATORS ============================== */
  
    virtual int read(char* buffer, int bufferLength) ;
      //:Standard read, see OsDatagramSocket for details.

    virtual int read(char* buffer, int bufferLength,
            UtlString* ipAddress, int* port);
      //:Standard read, see OsDatagramSocket for details.

    virtual int read(char* buffer, int bufferLength,
            struct in_addr* ipAddress, int* port);
      //:Standard read, see OsDatagramSocket for details.

    virtual int read(char* buffer, int bufferLength, long waitMilliseconds);
      //:Standard read, see OsDatagramSocket for details.

    virtual void setKeepAlivePeriod(int secs) ;
      //:Set the time period for STUN keep alives packets.

    virtual void setStunServer(const char* szHostname) ;
      //:Set the target stun server

    virtual void enableStun(bool bEnable) ;
      //: Enable to disable stun.  Disabling stun will stop keep alives 
      //  and cause getExternalIp to return the local addresses.

    virtual void refreshStunBinding(UtlBoolean bFromReadSocket) ;
      //: Force the transmission and reception of a stun binding

    virtual int readStunPacket(char* buffer, int bufferLength, const OsTime& rTimeout) ;
      //: Read the next stun packet off the socket.  

/* ============================ ACCESSORS ================================= */

   virtual UtlBoolean getExternalIp(UtlString* ip, int* port) ;
     //:Return the external IP address for this socket.
     // OsStunDatagramSocket will return the NATted address if available

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    int mKeepAlivePeriod ;      /**< Keep alive/refresh period */
    UtlString mStunServer ;     /**< stun server name */
    int mStunPort ;             /**< port reported by stun process */
    UtlString mStunAddress ;    /**< ip address reported by stun process */
    OsMsgQ mStunMsgQ ;          /**< MsgQ used to store inbound stun messages */
    OsTimer* mpTimer ;          /**< timer used for keep alives */
    StunRefreshEvent* mpRefreshEvent ; /** Refresh event (performs keep alive) */
    OsMutex mShutdownMutex ;    /**< Used to synchronized shut down / timer */
    bool mbEnabled ;            /**< Is stun enabled? */
    int mStunRefreshErrors ;    /**< Number of consecutive STUN refresh errors */
};

/* ============================ INLINE METHODS ============================ */



/* ///////////////////////// HELPER CLASSES /////////////////////////////// */

//: OsStunRefreshEvent is a helper class that spans a stun binding check
//: for the purpose of keep-alive / rediscovery.
class StunRefreshEvent : public OsNotification
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    StunRefreshEvent(OsStunDatagramSocket* pSocket) ;
    virtual ~StunRefreshEvent() ;

/* ============================ MANIPULATORS ============================== */

    virtual OsStatus signal(const int eventData) ;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    OsStunDatagramSocket* mpSocket ;

};

//: Basic StunMsg -- relies on external bodies to allocate and free memory.
class StunMsg : public OsMsg
{
public:
/* ============================ CREATORS ================================== */
   StunMsg(char* szBuffer, int nLength);
     //:Constructor

   StunMsg(const StunMsg& rStunMsg);
     //:Copy constructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)

   virtual
      ~StunMsg();
     //:Destructor
/* ============================ MANIPULATORS ============================== */

   StunMsg& operator=(const StunMsg& rhs);
     //:Assignment operator
/* ============================ ACCESSORS ================================= */

   char* getBuffer() ;
   
   int getLength() ;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    char* mBuffer ;
    int   mLength ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
};


#endif  // _OsStunDatagramSocket_h_

