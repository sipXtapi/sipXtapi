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

#define MAX_RTP_BYTES 4096

typedef enum
{
    UNKNOWN_PACKET,
    MEDIA_PACKET,
    STUN_PROBE_PACKET,
    STUN_DISCOVERY_PACKET,
    TURN_PACKET,
    CRLF_KEEPALIVE_PACKET,
    STUN_KEEPALIVE_PACKET,
    OTHER_PACKET
} PacketType ;

/**
 * Possible roles that a Media connection can have.
 */
 typedef enum 
 {
   RTP_TRANSPORT_UNKNOWN = 0x00000000,
   RTP_TRANSPORT_UDP = 0x00000001,
   RTP_TRANSPORT_TCP = 0x00000002,
   RTP_TCP_ROLE_ACTIVE = 0x00000004,
   RTP_TCP_ROLE_PASSIVE = 0x00000008,
   RTP_TCP_ROLE_ACTPASS = 0x00000010,
   RTP_TCP_ROLE_CONNECTION = 0x00000020,
} RTP_TRANSPORT ;

typedef int RtpTransportOptions;
typedef int RtpTcpRoles;

class OsSocket;
class OsNotification; 

typedef enum 
{
    STUN = 0x02,
    DATA = 0x03
} TURN_FRAMING_TYPE;

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
    
    /**
     * Standard write - should be used to invoke the base class's write method
     */
    virtual int socketWrite(const char* buffer, int bufferLength,
                      const char* ipAddress, int port, PacketType packetType=UNKNOWN_PACKET) = 0;    
    
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


    /**
     * Set a notification object to be signaled when the first the data 
     * packet is received from the socket.  Once this is signaled, the 
     * notification object is discarded.
     */
    virtual void setReadNotification(OsNotification* pNotification) = 0 ;

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
