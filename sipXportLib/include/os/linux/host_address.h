// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
////////////////////////////////////////////////////////////////////////////////

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

