//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

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

