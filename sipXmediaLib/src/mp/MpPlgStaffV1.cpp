//  
// Copyright (C) 2007-2020 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#include <ctype.h>

#include <utl/UtlInit.h>
#include <mp/MpPlgStaffV1.h>
#include <mp/MpCodecFactory.h>
#include <os/OsSysLog.h>

MpCodecCallInfoV1* MpCodecFactory::sStaticCodecsV1 = NULL;

extern "C" void callbackRegisterStaticCodec(const char* moduleName,
                                            const char* codecModuleName,
                                            dlPlgInitV1_2 plgInit,
                                            dlPlgGetInfoV1_1 plgGetInfo,
                                            dlPlgDecodeV1 plgDecode,
                                            dlPlgEncodeV1 plgEncode,
                                            dlPlgFreeV1 plgFree,
                                            dlPlgGetPacketSamplesV1_2 plgGetPacketSamples,
                                            dlPlgGetSignalingDataV1 plgSignaling)
{
   MpCodecCallInfoV1* pCodecInfo = new MpCodecCallInfoV1(moduleName,
                                                         codecModuleName,
                                                         plgInit,
                                                         plgGetInfo,
                                                         plgGetPacketSamples,
                                                         plgDecode,
                                                         plgEncode,
                                                         plgFree,
                                                         plgSignaling,
                                                         TRUE);
   assert(pCodecInfo);

   MpCodecFactory::addStaticCodec(pCodecInfo);
}

extern "C" void mppLogError(const char* format, ...)
{
    va_list varArgs;
    va_start(varArgs, format);

    OsSysLog::vadd(FAC_MP, PRI_ERR,
                  format, varArgs);

    va_end(varArgs);
}

