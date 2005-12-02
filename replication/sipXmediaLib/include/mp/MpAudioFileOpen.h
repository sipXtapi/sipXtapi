//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef MP_AUDIO_FILE_OPEN_H
#define MP_AUDIO_FILE_OPEN_H

#include "mp/MpAudioAbstract.h"
#include <os/istream>

MpAudioAbstract *MpOpenFormat(istream &file);

#endif
