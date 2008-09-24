//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include <mp/codecs/PlgDefsV1.h>

DECLARE_FUNCS_V1(sipxPcmu);
DECLARE_FUNCS_V1(sipxPcma);

PLG_ENUM_CODEC_START(sipXpcmapcmu)
  PLG_ENUM_CODEC(sipxPcmu)
  PLG_ENUM_CODEC_NO_SPECIAL_PACKING(sipxPcmu)
  PLG_ENUM_CODEC_NO_SIGNALING(sipxPcmu)

  PLG_ENUM_CODEC(sipxPcma)
  PLG_ENUM_CODEC_NO_SPECIAL_PACKING(sipxPcma)
  PLG_ENUM_CODEC_NO_SIGNALING(sipxPcma)
PLG_ENUM_CODEC_END 
