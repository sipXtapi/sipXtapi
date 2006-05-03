//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include "nspr.h"
#include "secutil.h"
#include "pk11func.h"
#include "pkcs12.h"
#include "p12plcy.h"
#include "nss.h"
#include "secport.h"
#include "certdb.h"

bool P12Wrapper_Init(char *dir, char *dbprefix);

SECStatus P12Wrapper_ImportPKCS12ObjectFromBuffer(char *pkcs12Buffer, int pkcs12Size, PK11SlotInfo *slot,
			SECItem *slotPw, SECItem *p12FilePw);
			
