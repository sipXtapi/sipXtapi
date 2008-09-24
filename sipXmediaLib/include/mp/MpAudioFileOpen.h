//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MP_AUDIO_FILE_OPEN_H
#define MP_AUDIO_FILE_OPEN_H

#include "mp/MpAudioAbstract.h"
#include <os/istream>

MpAudioAbstract *MpOpenFormat(istream &file);

#endif
