// 
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.// $Id$
//
// Copyright (C) 2005 Pingtel Corp.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _OsNatSocketBaseImpl_h_
#define _OsNatSocketBaseImpl_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsMutex.h"
#include "os/IStunSocket.h"
#include "os/OsNotification.h"
#include "utl/UtlString.h"

// The follow defines are used to keep track of what has been recorded for
// various time-based metrics.
#define ONDS_MARK_NONE           0x00000000
#define ONDS_MARK_FIRST_READ     0x00000001
#define ONDS_MARK_LAST_READ      0x00000002
#define ONDS_MARK_FIRST_WRITE    0x00000004
#define ONDS_MARK_LAST_WRITE     0x00000008

class OsNatSocketBaseImpl : public IStunSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    OsNatSocketBaseImpl() ;
    virtual ~OsNatSocketBaseImpl() ;

    /**
     * Set a notification object to be signaled when the first the data 
     * packet is received from the socket.  Once this is signaled, the 
     * notification object is discarded.
     */
    virtual void setReadNotification(OsNotification* pNotification) ;

    /**
     * Configure ICE credentials for this socket. Once set, the STUN
     * responder in OsNatAgentTask will:
     *   - Validate USERNAME on incoming STUN Binding Requests
     *   - Validate MESSAGE-INTEGRITY using localPwd as the HMAC-SHA1 key
     *   - Include MESSAGE-INTEGRITY and FINGERPRINT in outgoing
     *     Binding Responses, signed with localPwd
     *
     * Without this call, the STUN responder operates in legacy mode
     * (no integrity validation, no signed responses) -- preserving the
     * historical sipXtapi behavior.
     *
     * remotePwd is stored but not currently used. It is reserved for
     * a future full-ICE implementation that initiates outbound STUN
     * Binding Requests; ice-lite mode (the current behavior) only
     * responds, so only local credentials are referenced on the wire.     
     *
     * Thread-safe: may be called from any thread. The OsNatAgentTask
     * thread reads the credentials under the same lock when servicing
     * STUN messages.
     */
    virtual void setIceCredentials(const UtlString& localUfrag,
                                   const UtlString& localPwd,
                                   const UtlString& remoteUfrag,
                                   const UtlString& remotePwd) ;

    /**
     * Retrieve a snapshot of the ICE credentials configured for this
     * socket. Returns true if credentials have been set (via
     * setIceCredentials), false otherwise.
     *
     * Outputs are copies, safe for the caller to retain regardless of
     * concurrent reconfiguration. The four output strings are populated
     * only when this function returns true.
     */
    virtual bool getIceCredentials(UtlString& localUfrag,
                                   UtlString& localPwd,
                                   UtlString& remoteUfrag,
                                   UtlString& remotePwd) ;

    /**
     * Cheap inquiry: are ICE credentials configured on this socket?
     * Equivalent to getIceCredentials() but skips the string copies.
     */
    virtual bool hasIceCredentials() ;

    /**
     * Register a callback to be invoked on the first STUN Binding Request
     * carrying USE-CANDIDATE. See IStunSocket::IceNominationCallback.
     * Thread-safe.
     */
    virtual void setIceNominationCallback(IStunSocket::IceNominationCallback callback,
                                          void* userData) ;

    /**
     * Fire the registered ICE nomination callback (one-shot).
     * Called by OsNatAgentTask. Thread-safe.
     */
    virtual void fireIceNomination(const UtlString& remoteIp, int remotePort) ;
    
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    void markReadTime() ;
    void markWriteTime() ;

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

    /**
     * Handle/process an inbound STUN message.
     */
    virtual void handleStunMessage(char* pBuf, int length, UtlString& fromAddress, int fromPort) ;


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

    /**
     *  Handle/process an inbound STUN or TURN message.
     */
    virtual bool handleSturnData(char*      buffer, 
                                 int&       bufferLength,
                                 UtlString& receivedIp,
                                 int&       receivedPort) ; 

    virtual OsSocket* getSocket() =0;

protected:
    unsigned int          miRecordTimes ;   // Bitmask populated w/ ONDS_MARK_*
    OsDateTime            mFirstRead ;
    OsDateTime            mLastRead ;
    OsDateTime            mFirstWrite ;
    OsDateTime            mLastWrite ;
    OsMutex               mReadNotificationLock ;
    OsNotification*       mpReadNotification ;

    // ICE credentials state. mIceCredsLock protects all five fields
    // below; held briefly across copies in get/set. mbIceCredsSet
    // doubles as the "ICE enabled?" flag for OsNatAgentTask.
    OsMutex               mIceCredsLock ;
    UtlString             mIceLocalUfrag ;
    UtlString             mIceLocalPwd ;
    UtlString             mIceRemoteUfrag ;
    UtlString             mIceRemotePwd ;
    bool                  mbIceCredsSet ;

    // ICE nomination callback. Protected by mIceCredsLock.
    // The higher layer passes 'this' as userData and casts it back in the callback.
    IStunSocket::IceNominationCallback mpIceNominationCallback ; ///< NULL until set
    void*                              mpIceNominationUserData ;  ///< opaque context
    bool                               mbIceNominationFired ;     ///< one-shot guard
    
};

/* ============================ INLINE METHODS ============================ */


/* ///////////////////////// HELPER CLASSES /////////////////////////////// */



#endif  // _OsNatSocketBaseImpl_h_

