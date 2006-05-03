//
// Copyright (C) 2004-2006 SIPfoundry Inc.
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

extern "C" 
bool getAllLocalHostIps(const class HostAdapterAddress* localHostAddresses[],
                        int &numAddresses);
//: Return this host's ip addresses, as an array of UtlString references
//  Does not include the "loopback" address.

extern "C" bool getContactAdapterName(char* szAdapter, const char* szIp, bool unusedHere);
//: Returns a generated adapter name associated with the IP address

#endif

#endif // getDNSServers_h_
