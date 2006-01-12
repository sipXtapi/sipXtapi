// 
// Copyright (C) 2005 SIPfoundry Inc.
// License by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _REGISTERINITIALSYNC_H_
#define _REGISTERINITIALSYNC_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
 * RegistrarInitialSync is an OsTask that implements the startup
 * phase of operation: recover the local registration
 * database (if possible), and resynchronize with all peer
 * registrars.  Any updates are pulled from peer registrars so that the
 * local registrar can tell when its database is up to date and that no
 * more updates are available.  During the startup phase the database is
 * not yet known to be up to date, so the Registry/Redirect service does
 * not accept either any SIP request or any request to push updates from
 * any peer registrar.
 */
class RegistrarInitialSync
{
  public:

   /// Create the startup phase thread.
   RegistrarInitialSync()
      {
      };

   virtual int run(void* pArg);

   /// destructor
   virtual ~RegistrarInitialSync()
      {
      };

  protected:

  private:

   /// There is no copy constructor.
   RegistrarInitialSync(const RegistrarInitialSync&);

   /// There is no assignment operator.
   RegistrarInitialSync& operator=(const RegistrarInitialSync&);
    
};

#endif // _REGISTERINITIALSYNC_H_
