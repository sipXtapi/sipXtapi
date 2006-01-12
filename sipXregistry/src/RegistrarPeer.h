// Copyright (C) 2005 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _REGISTRARPEER_H_
#define _REGISTRARPEER_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "net/Url.h"
#include "utl/UtlString.h"
#include "os/OsBSem.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * A peer registrar is another registry server in the same SIP domain
 * with which this registrar synchronizes all registration data.  This class
 * tracks the relationship with a particular peer.  Instances of this class
 * are constructed by SipRegistrar based on the configuration, and may not
 * be destroyed other than by SipRegistrar.
 *
 * The RegistrarPeer object for a given peer may be obtained using SipRegistrar::getPeer. 
 *
 * A UtlSListIterator over a list of all RegistrarPeer objects may be obtained
 * using SipRegistrar::getPeerList.  
 *
 * There is not a RegistrarPeer object for the local registrar
 * (we do not treat ourselves as a peer).
 *
 * @nosubgrouping
 */
class RegistrarPeer : public UtlString
{
  public:
         
// ================================================================
/** @name                  Addressing
 *
 * Note that the host name of the peer is stored in the parent UtlString,
 * so RegistrarPeer::data() returns a pointer to that name.
 */
///@{

   /// The full URL to be used to make an XML RPC request of this peer.
   void rpcURL(Url& url);

///@}

// ================================================================
/** @name                  Reachability
 */
///@{

   typedef enum
      {
         PeerSyncStateUnknown, ///< initial condition before startup sync is complete.
         PeerReachable,        ///< initial sync completed, and no failure has occured.
         PeerUnReachable       ///< most recent request to this peer failed.
      } SynchronizationState;

   /// Whether or not the most recent attempt to reach this peer succeeded.
   SynchronizationState synchronizationState();
   /**
    * No attempt should be made to push updates to or accept push updates from a peer
    * that is not PeerReachable; the RegisterTest thread is responsible for attempting
    * to re-establish contact.
    */
   
   /// Indicate that a request to this peer failed.
   void markUnReachable();
   /**<
    * This triggers the RegisterTest thread to begin polling
    * this peer; until that succeeds or a request is received from this
    * peer, the isReachable method will return false.
    */

   /// Indicate that a request to this peer succeeded or a request was received from it.
   void markReachable();
   /**<
    * If the peer was previously unreachable, this stops the RegisterTest polling.
    * Until the next time markUnReachable is called, isReachable returns true.
    */

///@}

// ================================================================
/** @name                  Synchronization
 */
///@{

   /// The oldest update successfully sent to this peer.
   intll sentTo();

   /// The last update received from this peer.
   intll receivedFrom();

   void setSentTo(intll updateNumber);

   void setReceivedFrom(intll updateNumber);
   

///@}
   
  protected:
   /// only SipRegistrar may construct and destroy RegistrarPeer objects
   friend class SipRegistrar; 

   OsBSem* mLock;       ///< must be held to access to other member variables.
   SynchronizationState mSyncState; 
   intll   mSentTo;
   intll   mReceivedFrom;
   Url     mUrl;        ///< XML RPC URL

   /// All RegistrarPeer objects are initially considered to be not reachable.
   RegistrarPeer( const UtlString& name
                 ,int              rpcPort
                 ,const char*      rpcPath = "/RPC2"
                 );

   ~RegistrarPeer();

  private:

   /// There is no copy constructor.
   RegistrarPeer(const RegistrarPeer&);

   /// There is no assignment operator.
   RegistrarPeer& operator=(const RegistrarPeer&);
    
};

#endif // _REGISTRARPEER_H_

