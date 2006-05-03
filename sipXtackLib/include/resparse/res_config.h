//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDE_RESPARSE_RES_CONFIG_H
#define INCLUDE_RESPARSE_RES_CONFIG_H
#ifdef __cplusplus
extern "C" {
#endif

/* enable debugging code (needed for dig) */
/*#define       DEBUG   1 */


#define RESOLVSORT      /* allow sorting of addresses in gethostbyname */
#define RFC1535         /* comply with RFC1535 (STRONGLY reccomended by vixie)*/
#undef  USELOOPBACK     /* res_init() bind to localhost */
#undef  SUNSECURITY     /* verify gethostbyaddr() calls - WE DONT NEED IT  */
#define MULTI_PTRS_ARE_ALIASES 1 /* fold multiple PTR records into aliases */
#define CHECK_SRVR_ADDR 1 /* confirm that the server requested sent the reply */
#define BIND_UPDATE 1   /* update support */

#ifdef __cplusplus
}
#endif
#endif /* INCLUDE_RESPARSE_RES_CONFIG_H */
