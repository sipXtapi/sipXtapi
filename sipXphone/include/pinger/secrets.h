// $Id: //depot/OPENDEV/sipXphone/include/pinger/secrets.h#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

/*
 * This file contains the various shared secrets used within the 
 * C++ portion of the application layer.  They are only defined if not
 * previously defined as to allow for resetting during the build
 * process.
 */ 

#ifndef CONFIG_SECRET
// The config secret is by both myxpressa and for configuration retrieval.
#define CONFIG_SECRET  "CONFIG_1234"
#endif

#ifndef LICENSE_SECRET_BETA
// The beta secret was used for license retrieval during the beta stage
// of xpressa.  This value is maintained so that users are not required
// to obtain a new key.
#define LICENSE_SECRET_BETA  "LICENSE_BETA"
#endif

#ifndef LICENSE_SECRET_1_0
// The xpressa v1.0 secret.  This value is maintained so that users are
// not required to obtain a new key.
#define LICENSE_SECRET_1_0  "LICENSE_1"
#endif

#ifndef LICENSE_SECRET_2_0
// The xpressa v2.0 secret.  This value is maintained so that users are
// not requird to obtain a new key.
#define LICENSE_SECRET_2_0  "LICENSE_2"
#endif

#ifndef PASSWORD_SECRET
// This secret is used for the embedded WebUI.
#define PASSWORD_SECRET  "PASSWORD_1234"
#endif
