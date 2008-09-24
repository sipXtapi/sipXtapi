//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef getDNSServers_h_
#define getDNSServers_h_

#ifdef __cplusplus
#include <utl/UtlString.h>

  /// Return this host's IP addresses.
extern "C" 
bool getAllLocalHostIps(const class HostAdapterAddress* localHostAddresses[],
                        int &numAddresses);
  /**<
  *  Does not include the "loopback" address.
  */

  /// Return a generated adapter name associated with the IP address.
extern "C"
bool getContactAdapterName(UtlString &adapterName, const UtlString &ipAddress,
                           bool unusedHere);

#endif

#endif // getDNSServers_h_
