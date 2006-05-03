//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsSocket_h_
#define _OsSocket_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "utl/UtlString.h"
#include "os/OsBSem.h"


// DEFINES
#define MAX_IP_ADDRESSES 32
#define MAX_ADAPTER_NAME_LENGTH 256

//: constant indentifier indicating the maximum number of IP addresses on this host.
#define OS_INVALID_SOCKET_DESCRIPTOR -1

#if defined(_WIN32)
#   include <os/wnt/getWindowsDNSServers.h>
#   include "os/wnt/WindowsAdapterInfo.h"
#  define OsSocketGetERRNO() (WSAGetLastError())
#  define OS_INVALID_INET_ADDRESS INADDR_NONE // 0xffffffff
#elif defined(_VXWORKS)
#  define OsSocketGetERRNO() (errno)
#  define OS_INVALID_INET_ADDRESS 0xffffffff
#elif defined(__pingtel_on_posix__)
#  include "os/linux/AdapterInfo.h"
#  define OsSocketGetERRNO() (h_errno)
#  define OS_INVALID_INET_ADDRESS 0xffffffff
#else
#  error Unsupported target platform.
#endif

// MACROS
// EXTERNAL FUNCTIONS

//used to access BindAddress from "C" code
extern "C" unsigned long osSocketGetDefaultBindAddress();

// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
/**
 * CONTACT_TYPE is an enumeration of possible address type for use with
 * SIP contacts and SDP connection information.
 */
typedef enum
{
    LOCAL,      /**< Local address for a particular interface */
    NAT_MAPPED, /**< NAT mapped address (e.g. STUN)           */
    RELAY,      /**< Relay address (e.g. TURN)                */
    CONFIG,     /**< Manually configured address              */

    AUTO = -1,  /**< Automatic contact selection; used for API 
                     parameters */
    ALL  = -2,  /**< Filter value for the SipContactDb, for looking
                     up records of all types. */
                     
} CONTACT_TYPE ;


/** Type for storing Contact Record identifiers */
typedef int CONTACT_ID; 



// TYPEDEFS

// FORWARD DECLARATIONS

//: Abstract Socket class
// This class encapsulates the Berkley socket in an object oriented,
// platform independent way.  The intention is also to be independent of
// the protocol over IP as much as is possible.  This should enable
// generic message transport with minimal knowledge of the underlying
// protocol.

class OsSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static UtlBoolean socketInitialized;

   enum SocketProtocolTypes
   {
      UNKNOWN = -1,
      TCP = 0,
      UDP = 1,
      MULTICAST = 2,
      SSL_SOCKET = 3
   };
   //: Protocol Types
   
/* ============================ CREATORS ================================== */
   OsSocket();
     //:Default constructor

   virtual
   ~OsSocket();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   static UtlBoolean socketInit();
   static unsigned long initDefaultAdapterID(UtlString &adapter_id);

   virtual int write(const char* buffer, int bufferLength);
   //:Blocking write to the socket
   // Write the characters in the given buffer to the socket.
   // This method will block until all of the bytes are written.
   //!param: buffer - The bytes to be written to the socket.
   //!param: bufferLength - The number of bytes contained in buffer.
   //!returns: The number of bytes actually written to the socket.
   //!returns: <br>Note: This does not necessarily mean that the bytes were
   //!returns: actually received on the other end.

   virtual int write(const char* buffer, int bufferLength, long waitMilliseconds);
   //:Non-blocking or limited blocking write to socket
   // Same as blocking version except that this write will block
   // for no more than the specified length of time.
   //!param: waitMilliseconds - The maximum number of milliseconds to block. This may be set to zero, in which case it does not block.

   virtual int read(char* buffer, int bufferLength);
   //:Blocking read from the socket
   // Read bytes into the buffer from the socket up to a maximum of
   // bufferLength bytes.  This method will block until there is
   // something to read from the socket.
   //!param: buffer - Place to put bytes read from the socket.
   //!param: bufferLength - The maximum number of bytes buffer will hold.
   //!returns: The number of bytes actually read.

   virtual int read(char* buffer, int bufferLength,
       UtlString* ipAddress, int* port);
   //:Blocking read from the socket
   // Read bytes into the buffer from the socket up to a maximum of
   // bufferLength bytes.  This method will block until there is
   // something to read from the socket.
   //!param: buffer - Place to put bytes read from the socket.
   //!param: bufferLength - The maximum number of bytes buffer will hold.
   //!param: ipAddress - The address of the socket that sent the bytes read.
   //!param: port - The port of the socket that sent the bytes read.
   //!returns: The number of bytes actually read.

   virtual int read(char* buffer, int bufferLength,
       struct in_addr* ipAddress, int* port);
   //:Blocking read from the socket
   // Read bytes into the buffer from the socket up to a maximum of
   // bufferLength bytes.  This method will block until there is
   // something to read from the socket.
   //!param: buffer - Place to put bytes read from the socket.
   //!param: bufferLength - The maximum number of bytes buffer will hold.
   //!param: ipAddress - The address of the socket that sent the bytes read.
   //!param: port - The port of the socket that sent the bytes read.
   //!returns: The number of bytes actually read.

   virtual int read(char* buffer, int bufferLength, long waitMilliseconds);
   //: Non-blocking or limited blocking read from socket
   // Same as blocking version except that this read will block
   // for no more than the specified length of time.
   //!param: waitMilliseconds - The maximum number of milliseconds to block. This may be set to zero in which case it does not block.

   virtual void close();
   //: Closes the socket

   virtual void makeNonblocking();
   virtual void makeBlocking();
   //: Make the connect and all subsequent operations blocking
   // By default the sockets are blocking.

   static void setDefaultBindAddress(const unsigned long bind_address);
   //set the default ipaddress the phone should bind to

/* ============================ ACCESSORS ================================= */

   virtual int getIpProtocol() const = 0;
   //:Return the protocol type of this socket

   /// return the string representation of the SocketProtocolType 
   const char* ipProtocolString() const;
   
   virtual UtlBoolean reconnect() = 0;
   //:Set up the connection again, assuming the connection failed

   int getSocketDescriptor() const;
   //:Return the socket descriptor
   // Warning: Use of this method risks the creation of platform-dependent
   // code.


   static void OsSocket::getDomainName(UtlString &domain_name);
        //gets static member m_DomainName

   static unsigned long getDefaultBindAddress();
   // get default ip address in network byte order

   static void getHostName(UtlString* hostName);
   //:Get this host's name
   // Gets the host name independent of a socket.

   static void getHostIp(UtlString* hostAddress);
   //:Get this host's IP address

   void getLocalHostName(UtlString* localHostName) const;
   //:Return this host's name
   // Returns a string containing the name of the host on which this socket
   // resides.  This may be the local name, a fully qualified domain name or
   // anything in between. This name may vary on the same host if it is
   // multi-homed, depending upon which NIC the Socket is associated with.

   void getLocalHostIp(UtlString* localHostAddress) const;
   //:Return this host's ip address
   // Returns the ip address for this host on which this socket is communicating
   // On multi-homed machines, this is the address to the NIC over which the
   // socket communicates. The format is of the form: xx.x.xxx.x
   
   const UtlString& getLocalIp() const;
   //:Return this socket's Local Ip Address
   
   void setLocalIp(const UtlString& localIp) { mLocalIp = localIp; }
   
   virtual int getLocalHostPort() const;
   //:Return the local port number
   // Returns the port to which this socket is bound on this host.

   virtual void getRemoteHostName(UtlString* remoteHostName) const;
   //:Return remote host name
   // Returns a string containing the name of the host on which the socket
   // on the other end of this socket is bound. This may be the local
   // name, a fully qualified domain name or anything in between.


   virtual void getRemoteHostIp(struct in_addr* remoteHostAddress,
                        int* remotePort = NULL);
   //:Return remote host ip address
   // Returns the ip address for the host on which the socket on the
   // other end of this socket is bound.

   virtual void getRemoteHostIp(UtlString* remoteHostAddress,
                        int* remotePort = NULL);
   //:Return remote host ip address
   // Returns the ip address for the host on which the socket on the
   // other end of this socket is bound. The format is of the form:
   // xx.x.xxx.x

   virtual int getRemoteHostPort() const;
   //:Return the remote port number
   // Returns the port to which the socket on the other end of this socket
   // is bound.

/* ============================ INQUIRY =================================== */

   virtual UtlBoolean isOk() const;
   //:Returns TRUE if this socket's descriptor is not the invalid descriptor

   virtual UtlBoolean isConnected() const;
   //:Returns TRUE if this socket is connected

   virtual UtlBoolean isReadyToReadEx(long waitMilliseconds, UtlBoolean &rSocketError) const;
   //:Poll if there are bytes to read
   // Returns TRUE if socket is read to read.
   // Returns FALSE if wait expires or socket error.
   // rSocketError returns TRUE is socket error occurred.

   virtual UtlBoolean isReadyToRead(long waitMilliseconds = 0) const;
   //:Poll if there are bytes to read
   // Returns TRUE if socket is read to read.
   // Returns FALSE if wait expires or socket error.

   virtual UtlBoolean isReadyToWrite(long waitMilliseconds = 0) const;
   //:Poll if socket is able to write without blocking

   static UtlBoolean isIp4Address(const char* address);
   //:Is the address a dotted IP4 address
   // (i.e., nnn.nnn.nnn.nnn where 0 <= nnn <= 255)

   static UtlBoolean isLocalHost(const char* hostAddress);
   //:Is the given host name this host

   static UtlBoolean isSameHost(const char* host1, const char* host2);
   //:Are these two host names/addresses equivalent

   static UtlBoolean getHostIpByName(const char* hostName, UtlString* hostAddress);
   //:Look up IP address for host name

   static void inet_ntoa_pt(struct in_addr input_address, UtlString& output_address);
   //:Convert in_addr input_address to dot ip address to avoid memory leak

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   static OsBSem mInitializeSem;
   int socketDescriptor;

   int localHostPort;
   int remoteHostPort;
   UtlString mLocalIp;
   UtlString localHostName;
   UtlString remoteHostName;
   UtlString mRemoteIpAddress;
   UtlBoolean mIsConnected;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static unsigned long m_DefaultBindAddress;
        //default ip address the phone should bind to. May be IPADDR_ANY

        static UtlString m_DomainName;
        //domain name for host machine

   OsSocket& operator=(const OsSocket& rhs);
     //:Disable assignment operator

   OsSocket(const OsSocket& rOsSocket);
     //:Disable copy constructor

   static UtlBoolean hasDefaultDnsDomain();
     //:Returns TRUE if this host has a default DNS domain
};

/* ============================ INLINE METHODS ============================ */


/**
 * The CONTACT_ADDRESS struct includes contact information (ip and port),
 * address source type, and interface.
 */
struct CONTACT_ADDRESS
{
    CONTACT_ADDRESS()
    {
        memset((void*)cInterface, 0, sizeof(cInterface));
        memset((void*)cIpAddress, 0, sizeof(cIpAddress));
        eContactType = AUTO;
        id = 0;
        iPort = -1;
        transportType = OsSocket::UNKNOWN;
    }
    
    // copy contstructor
    CONTACT_ADDRESS(const CONTACT_ADDRESS& ref)
    {
        strcpy(this->cInterface, ref.cInterface);
        strcpy(this->cIpAddress, ref.cIpAddress);
        this->eContactType = ref.eContactType;
        this->id = ref.id;
        this->iPort = ref.iPort;
        this->transportType = ref.transportType;
    }
    
    // assignment operator
    CONTACT_ADDRESS& operator=(const CONTACT_ADDRESS& ref)
    {
        // check for assignment to self
        if (this == &ref) return *this;

        strcpy(this->cInterface, ref.cInterface);
        strcpy(this->cIpAddress, ref.cIpAddress);
        this->eContactType = ref.eContactType;
        this->id = ref.id;
        this->iPort = ref.iPort;
        this->transportType = ref.transportType;
        
        return *this;
    }
        
    CONTACT_ID   id;              /**< Contact record Id */
    CONTACT_TYPE eContactType ;   /**< Address type/source */
    char              cInterface[MAX_ADAPTER_NAME_LENGTH + 4] ; /**< Source interface    */
    char              cIpAddress[MAX_IP_ADDRESSES] ; /**< IP Address          */
    int               iPort ;                         /**< Port                */
    OsSocket::SocketProtocolTypes   transportType;    /**< UDP, TCP, or TLS */
};
#endif  // _OsSocket_h_
