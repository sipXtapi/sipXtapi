// 
// Copyright (C) 2006 Pingtel Corp.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _IStunSocket_h_
#define _IStunSocket_h_

#include "os/OsDateTime.h"
#include "os/OsMsg.h"
#include "utl/UtlString.h"



// DEFINES
#define NAT_MSG_TYPE         (OsMsg::USER_START + 1) /**< Stun Msg type/Id */

// The follow defines are used to keep track of what has been recorded for
// various time-based metrics.
#define ONDS_MARK_NONE           0x00000000
#define ONDS_MARK_FIRST_READ     0x00000001
#define ONDS_MARK_LAST_READ      0x00000002
#define ONDS_MARK_FIRST_WRITE    0x00000004
#define ONDS_MARK_LAST_WRITE     0x00000008


class OsSocket;

/**
 * Generic interface representing a media transport object.
 * Implemented by VoiceEngineDatagramSocket, VoiceEngineConnectionSocket,
 * and any other class which provides RTP transport for VoiceEngine.
 */
class IStunSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ MANIPULATORS ============================== */

    /**
     * virtual destructor.
     */
    virtual ~IStunSocket() {};
    
    
    virtual OsSocket* getSocket() = 0;
    
    virtual UtlBoolean getRelayIp(UtlString* ip, int* port) = 0;
    virtual UtlBoolean getMappedIp(UtlString* ip, int* port) = 0;
    
    virtual void enableTransparentReads(bool bEnable) = 0;    
    
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
                            bool bReadFromSocket) = 0;



    /** 
     * Sets the STUN-resolved address and port for this socket.
     *
     * @param address String containing the stunned address.
     * @param iPort integer value of the stunned port.
     
     */
    virtual void setStunAddress(const UtlString& address, const int iPort) = 0;
    
    /**
     * Set the TURN-dervied relay address for this socket. 
     *
     * @param address TURN-derived hostname/IP address
     * @param iPort TURN-derived port address
     */
    virtual void setTurnAddress(const UtlString& address, const int iPort) = 0;


    /**
     * Report that a stun attempt failed.
     */
    virtual void markStunFailure() = 0;


    /**
     * Report that a stun attempt succeeded.
     */
    virtual void markStunSuccess(bool bAddressChanged) = 0;


    /**
     * Report that a stun attempt failed.
     */
    virtual void markTurnFailure() = 0;


    /**
     * Report that a stun attempt succeeded.
     */
    virtual void markTurnSuccess() = 0;


    /**
     * Reset the destination address for this OsNatDatagramSocket.  This
     * method is called by the OsStunAgentTask when a better address is 
     * found via STUN/ICE.
     *
     * @param address The new destination address
     * @param port The new destination port
     * @param priority Priority of the destination address
     */
    virtual void evaluateDestinationAddress(const UtlString& address, int iPort, int priority) = 0;    
    
    
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
                            bool bReadFromSocket) = 0;
                            
   /**
    * Prepares a destination under TURN usage.
    */
   virtual void readyDestination(const char* szAddress, int iPort) = 0;        
    
   /**
    * TODO: DOCS
    */
   virtual UtlBoolean applyDestinationAddress(const char* szAddress, int iPort) = 0;
   
   /**
    * TODO: DOCS
    */
   virtual UtlBoolean getBestDestinationAddress(UtlString& address, int& iPort) = 0;   
                           
    /**
     * Add an alternate destination to this OsNatDatagramSocket.  Alternate 
     * destinations are tested by sending stun packets.  If a stun response is
     * received and the priority is greater than what has already been selected
     * then that address is used.
     * 
     * @param szAddress IP address of the alternate destination
     * @param iPort port number of the alternate destination
     * @param priority priority of the alternate where a higher number 
     *        indicates a higher priority.
     */
    virtual void addAlternateDestination(const char* szAddress, int iPort, int priority) = 0;                           


/* ============================ ACCESSORS ================================= */

    /**
     * Get the timestamp of the first read data packet (excluding any 
     * STUN/TURN/NAT packets).
     */
    virtual bool getFirstReadTime(OsDateTime& time) = 0 ;

    /**
     * Get the timestamp of the last read data packet (excluding any 
     * STUN/TURN/NAT packets).
     */
    virtual bool getLastReadTime(OsDateTime& time) = 0 ;

    /**
     * Get the timestamp of the first written data packet (excluding any
     * STUN/TURN/NAT packets).
     */
    virtual bool getFirstWriteTime(OsDateTime& time) = 0 ;

    /**
     * Get the timestamp of the last written data packet (excluding any
     * STUN/TURN/NAT packets).
     */
    virtual bool getLastWriteTime(OsDateTime& time) = 0 ;
    
    /** 
     * Cleanup routine.
     */
    virtual void destroy() = 0;

};




#endif // #ifndef _IStunSocket_h_
