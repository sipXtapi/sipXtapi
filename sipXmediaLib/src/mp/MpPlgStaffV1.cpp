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

#include <utl/UtlInit.h>
#include <mp/MpPlgStaffV1.h>
#include <mp/MpCodecFactory.h>

MpCodecCallInfoV1* MpCodecFactory::sStaticCodecsV1 = NULL;

void MpCodecCallInfoV1::registerStaticCodec(const char* moduleName, 
                                            const char* codecModuleName,
                                            dlPlgInitV1 plgInit,
                                            dlPlgDecodeV1 plgDecode,
                                            dlPlgEncodeV1 plgEncode,
                                            dlPlgFreeV1 plgFree,
                                            dlPlgEnumSDPAndModesV1 plgEnum,
                                            dlPlgGetSignalingDataV1 plgSignaling)
{
   MpCodecCallInfoV1* pCodecInfo = new MpCodecCallInfoV1(moduleName,
                                                         codecModuleName,
                                                         plgInit,
                                                         plgDecode,
                                                         plgEncode,
                                                         plgFree,
                                                         plgEnum,
                                                         plgSignaling,
                                                         TRUE);
   assert(pCodecInfo);

   MpCodecFactory::addStaticCodec(pCodecInfo);
}

int AutoRegistratorBase::nAutoRegistered = 0;

AutoRegistratorBase::AutoRegistratorBase()
{
   nAutoRegistered++;
}
