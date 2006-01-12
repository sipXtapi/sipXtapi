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

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
 * This thread is the XML-RPC client that sends updates to
 * each peer server.
 */
class RegistrarSync : public OsTask
{
  public:

   /// constructor
   RegistrarSync();

   /// destructor
   virtual ~RegistrarSync()
      {
      };

   /// Signal that there may be updates ready to send.
   void sendUpdates();

   /// Task main loop.
   virtual int run(void* pArg);
   
  protected:

  private:

   /// mutex must be locked with OsLock to access any other member variable.
   OsBSem mutex;

   OsCSem mUpdatesPending;
   
   /// There is no copy constructor.
   RegistrarSync(const RegistrarSync&);

   /// There is no assignment operator.
   RegistrarSync& operator=(const RegistrarSync&);
    
};

#endif // _REGISTRARSYNC_H_
