// $Id$
//
// Copyright (C) 2005 Pingtel Corp.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _OsNatConnectionSocket_h_
#define _OsNatConnectionSocket_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsConnectionSocket.h"
#include "os/OsNatDatagramSocket.h"
#include "os/OsMsgQ.h"
#include "os/OsTimer.h"
#include "os/OsQueuedEvent.h"
#include "tapi/sipXtapi.h"
#include "os/OsRWMutex.h"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
/**
 * Possible roles that a Media connection can have.
 */
typedef enum RtpTcpRoles
{
    ACTIVE,     /**< Connection is acting as a tcp client. */
    PASSIVE,    /**< Connection is acting as a tcp server. */
    ACTPASS     /**< Connection is acting as both a tcp client and server. */
};

// TYPEDEFS
// FORWARD DECLARATIONS
class NatMsg ;
class OsEvent ;
class OsNatAgentTask;
class OsNatKeepaliveListener;



/**
 * OsNatConnectionSocket extends an OsDatagramSocket by adding an integrated
 * STUN and TURN client.  If STUN or TURN is enabled, request will be send 
 * to the designated server every refresh period.  The external addresses 
 * obtained by these mechanisms are retrieved by invoking getMappedIp and
 * getRelayIp.
 *
 * For this mechanism to work, someone must pump inbound socket data by 
 * calling one of the ::read() methods.  Otherwise, the packets will not be 
 * received/processed.  Internally, the implemenation peeks at the read 
 * data and passes the message to the OsNatAgentTask for processing.
 */
class OsNatConnectionSocket : public OsConnectionSocket, public IStunSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    friend class OsNatAgentTask ;

/* ============================ CREATORS ================================== */
    OsNatConnectionSocket(int connectedSocketDescriptor,
                          const RtpTcpRoles role);

    OsNatConnectionSocket(const char* szLocalIp,
                          int connectedSocketDescriptor,
                          const RtpTcpRoles role);

    // Constructor
    OsNatConnectionSocket(int serverPort,
                          const char* serverName,
                          UtlBoolean blockingConnect,
                          const char* localIp,
                          const bool bConnect,
                          const RtpTcpRoles role);

    /**
     * Standard Destructor
     */
    virtual ~OsNatConnectionSocket();

/* ============================ MANIPULATORS ============================== */

    virtual void setRole(const RtpTcpRoles role);
    virtual const RtpTcpRoles getRole() const;
    
    /**
     * Standard read, see OsDatagramSocket for details.
     */
    virtual int read(char* buffer, int bufferLength) ;

    /**
     * Standard read, see OsDatagramSocket for details.
     */
    virtual int read(char* buffer, int bufferLength,
            UtlString* ipAddress, int* port);

    /**
     * Standard read, see OsDatagramSocket for details.
     */
    virtual int read(char* buffer, int bufferLength,
            struct in_addr* ipAddress, int* port);

    /**
     * Standard read, see OsDatagramSocket for details.
     */
    virtual int read(char* buffer, int bufferLength, long waitMilliseconds);


    /**
     * Standard write, see OsDatagramSocket for details.
     */
    virtual int write(const char* buffer, int bufferLength);


    /**
     * Standard write, see OsDatagramSocket for details.
     */
    virtual int write(const char* buffer, int bufferLength,
                      const char* ipAddress, int port);

    /**
     * Standard write, see OsDatagramSocket for details.
     */
    virtual int write(const char* buffer, int bufferLength, 
                      long waitMilliseconds);

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
                            bool bReadFromSocket) ;

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
     * Disable TURN.  Disabling TURN will stop all keep alives and cause 
     * getRelayIp to fail.  
     */
    virtual void disableTurn() ;


    /**
     * When a stun packet is received this socket can either call read again
     * to obtain the next packet or return with zero bytes read.  By default
     * the socket will transparently call Read again and will block until a
     * non-stun packet is read. Calling this method will effect the next 
     * read -- in other words it will not unblock an active read.
     *
     * @param bEnable True to enable transparent stun reads and block until
     *        a non-stun packet is received (default) or False to return 
     *        with zero bytes read if a stun packet is received.
     */ 
    virtual void enableTransparentReads(bool bEnable) ;


    /**
     * Add an alternate destination to this OsNatConnectionSocket.  Alternate 
     * destinations are tested by sending stun packets.  If a stun response is
     * received and the priority is greater than what has already been selected
     * then that address is used.
     * 
     * @param szAddress IP address of the alternate destination
     * @param iPort port number of the alternate destination
     * @param priority priority of the alternate where a higher number 
     *        indicates a higher priority.
     */
    virtual void addAlternateDestination(const char* szAddress, int iPort, int priority) ;


    /**
     * Prepares a destination under TURN usage.
     */
    virtual void readyDestination(const char* szAddress, int iPort) ;

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
    virtual void setNotifier(OsNotification* pNotification) ;

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

/* ============================ ACCESSORS ================================= */

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
   virtual UtlBoolean getRelayIp(UtlString* ip, int* port) ;

   /**
    * TODO: DOCS
    */
   virtual UtlBoolean getBestDestinationAddress(UtlString& address, int& iPort) ;

   /**
    * TODO: DOCS
    */
   virtual UtlBoolean applyDestinationAddress(const char* szAddress, int iPort) ;

   /**
    * Get the timestamp of the first read data packet (excluding any 
    * STUN/TURN/NAT packets).
    */
   virtual bool getFirstReadTime(OsDateTime& time) ;

   /**
    * Get the timestamp of the last read data packet (excluding any 
    * STUN/TURN/NAT packets).
    */
   virtual bool getLastReadTime(OsDateTime& time) ;

   /**
    * Get the timestamp of the first written data packet (excluding any
    * STUN/TURN/NAT packets).
    */
   virtual bool getFirstWriteTime(OsDateTime& time) ;

   /**
    * Get the timestamp of the last written data packet (excluding any
    * STUN/TURN/NAT packets).
    */
   virtual bool getLastWriteTime(OsDateTime& time) ;

   virtual void destroy();


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    /**
     * ACTIVE, PASSIVE, or ACTPASS
     */
    RtpTcpRoles mRole;
    
    mutable OsRWMutex mRoleMutex;

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
     * Reset the destination address for this OsNatConnectionSocket.  This
     * method is called by the OsStunAgentTask when a better address is 
     * found via STUN/ICE.
     *
     * @param address The new destination address
     * @param port The new destination port
     * @param priority Priority of the destination address
     */
    void evaluateDestinationAddress(const UtlString& address, int iPort, int priority) ;    
    

    /**
     * Handle/process an inbound STUN message.
     */
    void handleStunMessage(char* pBuf, int length, UtlString& fromAddress, int fromPort) ;


    /**
     * Handle/process an inbound TURN message.
     */
    void handleTurnMessage(char* pBuf, int length, UtlString& fromAddress, int fromPort) ;

    void markReadTime() ;

    void markWriteTime() ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    


    STUN_STATE mStunState ; /**< STUN status/state */
    TURN_STATE mTurnState ; /**< TURN status/state */


    /* Global Attributes */
    OsNatAgentTask* mpNatAgent;      /**< Pointer to Nat agent task (handles refreshes) */
    bool mbTransparentReads ;        /**< Block until a non-stun/turn packet is read */
    OsNotification* mpNotification ; /** Notify on initial stun success or failure */
    bool            mbNotified ;     /** Have we notified the requestor? */

    /* ICE Settings */
    int miDestPriority ;        /**< Priority of destination address / port. */
    UtlString mDestAddress;     /**< Destination address */
    int miDestPort ;            /**< Destination port */      

    unsigned int          miRecordTimes ;   // Bitmask populated w/ ONDS_MARK_*
    OsDateTime            mFirstRead ;
    OsDateTime            mLastRead ;
    OsDateTime            mFirstWrite ;
    OsDateTime            mLastWrite ;

};


#endif  // _OsNatConnectionSocket_h_

