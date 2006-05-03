//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifdef __linux__

#ifdef __cplusplus
extern "C" {
#endif

unsigned int getExternalHostAddressLinux(void);
void getEthernetHWAddrLinux(char * address, int length);

#ifdef __cplusplus
}
#endif

#endif /* __linux__ */

