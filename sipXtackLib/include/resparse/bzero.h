//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDE_RESPARSE_BZERO_H
#define INCLUDE_RESPARSE_BZERO_H
#ifdef __cplusplus
extern "C" {
#endif

/* _local added to function name to avoid name conflict --GAT */
void    bzero_local (void *buf, size_t len);
void* res_memset(void* dst0, int c0, size_t length);

#ifdef __cplusplus
}
#endif
#endif /* INCLUDE_RESPARSE_BZERO_H */
