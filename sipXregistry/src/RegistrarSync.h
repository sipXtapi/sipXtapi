// 
// Copyright (C) 2006 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _REGISTRARSYNC_H_
#define _REGISTRARSYNC_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsTask.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipRegistrar;
class SipRegistrarServer;


/**
 * This thread is the XML-RPC client that sends updates to
 * each peer server.
 */
class RegistrarSync : public OsTask
{
public:
   /// constructor
   RegistrarSync(SipRegistrar& registrar);

   /// destructor
   virtual ~RegistrarSync();
  
   /// Signal that there may be updates ready to send.
   void sendUpdates();

   /// Task main loop.
   virtual int run(void* pArg);
   
protected:
   SipRegistrarServer& getRegistrarServer();

private:
   SipRegistrar& mRegistrar;

   /// mutex must be locked with OsLock to access any other member variable.
   OsBSem mMutex;

   // OsCSem mUpdatesPending;   :TODO: this may need to change to a queue
   
   /// There is no copy constructor.
   RegistrarSync(const RegistrarSync&);

   /// There is no assignment operator.
   RegistrarSync& operator=(const RegistrarSync&);
};

#endif // _REGISTRARSYNC_H_