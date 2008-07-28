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
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _OsNatSocketBaseImpl_h_
#define _OsNatSocketBaseImpl_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsMutex.h"
#include "os/IOsNatSocket.h"
#include "os/OsNotification.h"
#include "os/OsNatAgentTask.h"
#include "os/OsContact.h"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS


#define DEFAULT_MEDIA_STUN_KEEPALIVE        28


// STRUCTS
// TYPEDEFS
/**
 * NAT_STATUS is used internally to mark the end-state of a NAT attempt.  NAT
 * attempts range from a discovery lookup (what is my external IP address) to 
 * a TURN allocation (relaying).
 */
typedef enum
{
    NAT_STATUS_UNKNOWN,     /** Unknown NAT status -- either idle or pending */
    NAT_STATUS_SUCCESS,     /** NAT binding/allocation was successful */
    NAT_STATUS_FAILURE      /** NAT binding/allocation was unsuccessful (no 
                                response or negative response) */
} NAT_STATUS ;

/**
 * STURN_STATE holds state information for a STUN/TURN discovery binding/attempt
 */
typedef struct
{
    bool       bEnabled ;       /** Is STUN enabled? */
    NAT_STATUS status ;         /** What is the stun/turn status? */
  
    UtlString  address ;  /** Mapped or relay address */
    int        port ;     /** Mapped or relay port */
    
} STURN_STATE ;


// FORWARD DECLARATIONS
class OsNatKeepaliveListener ;
class OsNatAgentTask ;


/**
 * Base-class / default Nat implementation for both datagram and 
 * connection-oriented sockets.  This includeds all of the STUN, TURN, and
 * ICE APIs.
 */
class OsNatSocketBaseImpl 
    : public IOsNatSocket
    , public OsNatAgentContextListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    OsNatSocketBaseImpl() ;

    virtual ~OsNatSocketBaseImpl() ;

    virtual void destroy() ;

/* ============================ MANIPULATORS ============================== */

    virtual void setTransparentStunRead(bool bEnable) ;


    virtual UtlBoolean addCrLfKeepAlive(const char* szRemoteIp,
                                        const int   remotePort, 
                                        const int   keepAliveSecs,
                                        OsNatKeepaliveListener* pListener) ;

    virtual UtlBoolean removeCrLfKeepAlive(const char* szRemoteIp, 
                                           const int   remotePort) ;

    virtual UtlBoolean addStunKeepAlive(const char* szRemoteIp, 
                                        const int   remotePort, 
                                        const int   keepAliveSecs,
                                        OsNatKeepaliveListener* pListener) ;

    virtual UtlBoolean removeStunKeepAlive(const char* szRemoteIp, 
                                           const int   remotePort) ;

    /**
     * Sets as notification event that is signaled upon the next successful 
     * stun response or on failure (did not receive a stun response within 
     * (STUN_ABORT_THRESHOLD * STUN_TIMEOUT_RESPONSE_MS).  If a notification
     * event was previous set either by calling this method or via the 
     * constructor, it will be overridden.  If the initial STUN success/failure
     * state has already been determined, this method is undefined.
     *
     * @param pNotification Notification event signaled on success or failure.
     */ 
    virtual void setStunNotifier(OsNotification* pNotification) ;

    /** 
     * Enable STUN.  Enabling STUN will reset the the keep alive timer and 
     * will force a binding refresh.
     *
     * @param szStunServer
     * @param stunPort
     * @param iKeepAlive
     * @param stunOptions
     * @param bReadFromSocket
     */
    virtual void enableStun(const char* szStunServer, 
                            int stunPort,
                            int iKeepAlive, 
                            int stunOptions, 
                            bool bReadFromSocket,
                            int iNCPeriod = -1) ;

    /**
     * Disable STUN.  Disabling STUN will stop all keep alives and cause 
     * getMappedIp to fail.  
     */
    virtual void disableStun() ;


    /** 
     * Enable STUN.  Enabling STUN will reset the the keep alive timer and 
     * will force a binding refresh.
     *
     * @param szTurnSever
     * @param turnPort
     * @param iKeepAlive
     * @param username
     * @param password
     * @param bReadFromSocket
     */
    virtual void enableTurn(const char* szTurnSever,
                            int turnPort,
                            int iKeepAlive,
                            const char* username,
                            const char* password,  
                            bool bReadFromSocket) ;

    /**
     * TODO: DOCS
     */
    virtual UtlBoolean setTurnDestination(const char* szAddress, int iPort) ;

    /**
     * Prepares a destination under TURN usage.
     */
    virtual void primeTurnDestination(const char* szAddress, int iPort);


    /**
     * Disable TURN.  Disabling TURN will stop all keep alives and cause 
     * getTurnIp to fail.  
     */
    virtual void disableTurn(bool bForce = true) ;


    /**
     * Add an alternate destination to this OsNatDatagramSocket.  Alternate 
     * destinations are tested by sending stun packets.  If a stun response is
     * received and the priority is greater than what has already been selected
     * then that address is used.
     * 
     * @param szAddress IP address of the alternate destination
     * @param iPort port number of the alternate destination
     * @param priority priority of the alternate where a higher number 
     *        indicates a higher priority
     */
    virtual void addAlternateDestination(const char* szAddress, int iPort, int priority) ;


    virtual void OnClassificationComplete(NAT_CLASSIFICATION_TYPE type) ;

/* ============================ ACCESSORS ================================= */                           

    virtual OsSocket* getSocket();                                 

    /**
     * Get the timestamp of the last data indication
     */
    virtual bool getLastDI(OsDateTime& time) ;

    /**
     *
     */
    virtual bool getTransparentStunRead() const ;


   /**
    * Return the external mapped IP address for this socket.  This method will 
    * return false if stun is disabled, it was unable to retrieve a stun 
    * binding, or both the ip and port parameters are null.
    *
    * @param ip Buffer to place STUN-discovered IP address
    * @param port Buffer to place STUN-discovered port number
    */
   virtual UtlBoolean getMappedIp(UtlString* ip, int* port) ;


    /**
     * Return the external relay IP address for this socket.  This method will
     * return false if stun is disabled, it was unable to retrieve a turn 
     * allocation, or both the ip and port parameters are null.
     *
     * @param ip Buffer to place TURN-discovered IP address
     * @param port Buffer to place TURN-discovered port number
     */
   virtual UtlBoolean getTurnIp(UtlString* ip, int* port) ;

   /**
    * Wait for the best destination to become available
    */
   virtual bool waitForBestDestination(bool       bLongWait,
                                       UtlString& address,
                                       int&       iPort,
                                       bool&      bViaOurRelay,
                                       int&       priority) ;

   /**
    * TODO: DOCS
    */
   virtual UtlBoolean getBestDestinationAddress(UtlString& address, 
                                                int&       iPort, 
                                                bool&      bViaOurRelay, 
                                                int&       priority) ;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    /**
     * Handle/process an inbound STUN message.
     */
    virtual void handleStunMessage(char*      pBuf, 
                                   int        length, 
                                   UtlString& fromAddress, 
                                   int        fromPort,
                                   UtlString* pRelayIp = NULL,
                                   int*       pRelayPort = NULL) ; 

    /**
     * Handle/process an inbound TURN message.
     */
    virtual void handleTurnMessage(char*        pBuf, 
                                   int          length, 
                                   UtlString&   fromAddress, 
                                   int          fromPort) ;
    
    /**
     * Pull a data indication out of the buffer
     */
    virtual int handleTurnDataIndication(char*      buffer, 
                                         int        bufferLength,
                                         UtlString* pRecvFromIp,
                                         int*       pRecvFromPort) ;

    virtual bool handleSturnData(char*      buffer, 
                                 int&       bufferLength,
                                 UtlString& receivedIp,
                                 int&       receivedPort,
                                 UtlString* pRelayIp = NULL,
                                 int*       pRelayPort = NULL) ; 


    /**
     * Set the STUN-derived address for this socket.
     *
     * @param address STUN-derived hostname/IP address
     * @param iPort STUN-derived port address
     */ 
    void setStunAddress(const UtlString& address, const int iPort) ;


    /**
     * Set the TURN-dervied relay address for this socket. 
     *
     * @param address TURN-derived hostname/IP address
     * @param iPort TURN-derived port address
     */
    void setTurnAddress(const UtlString& address, const int iPort) ;


    /**
     * Report that a stun attempt failed.
     */
    void markStunFailure() ;


    /**
     * Report that a stun attempt succeeded.
     */
    void markStunSuccess(bool bAddressChanged) ;


    /**
     * Report that a stun attempt failed.
     */
    void markTurnFailure() ;


    /**
     * Report that a stun attempt succeeded.
     */
    void markTurnSuccess() ;


    /**
     * Reset the destination address for this OsNatDatagramSocket.  This
     * method is called by the OsStunAgentTask when a better address is 
     * found via STUN/ICE.
     *
     * @param address The new destination address
     * @param port The new destination port
     * @param priority Priority of the destination address
     */
    void evaluateDestinationAddress(const UtlString& address, int iPort, int priority, bool bViaOurRelay) ;       

    void checkDelayedShutdown() ;

protected:
    OsDateTime            mLastDI ;         // Last data indication
    bool                  mbReceivedDI ;
    bool                  mbDelayedTurnShutdown ;
        
    bool                  mbTransparentStunRead ;
    OsNatAgentTask*       mpNatAgent;

    /* ICE Settings */
    int miDestPriority ;        /**< Priority of destination address / port. */
    UtlString mDestAddress;     /**< Destination address */
    int miDestPort ;            /**< Destination port */
    bool mbDestViaOurRelay ;    /**< Is this dest accessible via our relay? */

    OsContact mFallbackContact ;
    int       miFallbackPriority ;

    STURN_STATE mStunState ; /**< STUN status/state */
    STURN_STATE mTurnState ; /**< TURN status/state */
   
    OsNotification* mpStunNotification ; /** Notify on initial stun success or failure */
    bool            mbStunNotified ;     /** Have we notified the requestor? */

};

/* ============================ INLINE METHODS ============================ */


/* ///////////////////////// HELPER CLASSES /////////////////////////////// */



#endif  // _OsNatSocketBaseImpl_h_

