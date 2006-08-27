//
// Copyright (C) 2006 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsContact_h_
#define _OsContact_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlContainable.h"
#include "utl/UtlString.h"
#include "os/OsSocket.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ ENUMERATIONS ============================== */
/**
* Enumeration of IP address types.
*/
typedef enum IpAddressType
{
    IP4,    /** < Internet Protocol version 4 >*/
    IP6     /** < Internet Protocol version 6 >*/   
};

/**
 * OsContact is a containable object which represents
 * an inter-networking point of contact, 
 * which could also be described as a "transport endpoint".
 * It contains an address, a port value, a protocol, and the 
 * address type.
 */
class OsContact : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ FRIENDS =================================== */
    friend class OsContactTests;
    
/* ============================ ENUMERATIONS ============================== */

/* ============================ CREATORS ================================== */

    /**
     * Constructor.
     *
     * @param address String representation of the ip address. 
     *                For IPv4, it should be in "dotted quad" notation.
     *                For IPv6, it should use the text representation of
     *                addresses specified in RFC 3513.
     *
     *                Note that IP addresses which are sematically
     *                identical, but differ in string representation, will
     *                be considered as not being equal.  (e.g. - 
     *                a contact having an address "192.168.1.2" will not equal 
     *                a contact having an address "192.168.001.002".
     *                 
     *                Note that the current design doesn't allow for IPV4 tunneled over
     *                IPV6 to compare correctly with the original IPV4 address.
     *
     *                Note that IP addresses will be compared as strings, 
     *                so, use of wildcard addresses ("0.0.0.0") will not match any
     *                other address.
     *                 
     * @param port The port value for the contact. 
     * @param protocol The prefered protocol for this contact. 
     * @param type The type of ip address.
     */
    OsContact(UtlString address,
              int port,
              OsSocket::SocketProtocolTypes protocol,
              IpAddressType type);
                       
    /**
     * Copy constructor.
     */                       
    OsContact(const OsContact& ref);
         
    /**
     * Destructor
     */
    virtual ~OsContact();


/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */
    /**
     * Accessor for the address string.
     */
    const void getAddress(UtlString& address) const;
    
    /**
     * Accessor for the port value.
     */
    const int getPort() const;
    
    /**
     * Accessor for the protocol enum value.
     */
    const OsSocket::SocketProtocolTypes getProtocol() const;
    
    /**
     * Accessor for the address type enum value.
     */
    const IpAddressType getAddressType() const;

    /**
     * Calculate a unique hash code for this object.  If the equals
     * operator returns true for another object, then both of those
     * objects must return the same hashcode.
     */
    virtual unsigned hash() const ;

    /**
     * Get the ContainableType for a UtlContainable derived class.
     */
    virtual UtlContainableType getContainableType() const;
    
/* ============================ INQUIRY =================================== */

    /**
     * Compare the this object to another like-objects.  Results for 
     * designating a non-like object are undefined.
     *
     * @returns 0 if equal, < 0 if less then and >0 if greater.
     */
    virtual int compareTo(UtlContainable const *) const ;    

    /**
     * Test this object to another like-object for equality.  This method 
     * returns false if unlike-objects are specified.
     */
    virtual UtlBoolean isEqual(UtlContainable const *) const ; 

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    static UtlContainableType TYPE ;    /** < Class type used for runtime checking */ 

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    /**
     * Disallow assignment.
     */
    OsContact& operator=(const OsContact&) ;
    
    /**
     * Helper function for comparison of two ints.
     *
     * @param first First integer in the comparison pair.
     * @param second Second integer in the comparison pair.
     *
     * @returns 0 if equal, < 0 if first is less then and >0 if greater.
     */
    const int compareInt(const int first, const int second) const;
    
    UtlString mAddress;                      /** < String representation of the ip address. */
    int mPort ;                              /** < The port value for the contact. */ 
    OsSocket::SocketProtocolTypes mProtocol; /**< The prefered protocol for this contact. */
    IpAddressType mType;                     /**< The type of ip address. */

} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _OsContact_h_
