//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: 

#include <utl/UtlInit.h>
#include <mp/MpPlgStaffV1.h>
//#include "plg/PlgStaffV1.h"
#include <mp/MpCodecFactory.h>

//void* pPtrMagic::magic = NULL;

MpCodecCallInfoV1* MpCodecFactory::sStaticCodecsV1 = NULL;

void MpCodecCallInfoV1::registerStaticCodec(const char* moduleName, 
                                            const char* codecModuleName,
                                            dlPlgInitV1 plgInit,
                                            dlPlgDecodeV1 plgDecode,
                                            dlPlgEncodeV1 plgEncode,
                                            dlPlgFreeV1 plgFree,
                                            dlPlgEnumSDPAndModesV1 plgEnum,
                                            dlPlgGetSignalingDataV1 plgSignaling
                                            )
{
   MpCodecCallInfoV1* pCodecInfo = new MpCodecCallInfoV1;
   assert(pCodecInfo);

   pCodecInfo->mbStatic = TRUE;
   pCodecInfo->mModuleName = moduleName;
   pCodecInfo->mCodecModuleName = codecModuleName;
   pCodecInfo->mPlgInit = plgInit;
   pCodecInfo->mPlgDecode = plgDecode;
   pCodecInfo->mPlgEncode = plgEncode;
   pCodecInfo->mPlgFree = plgFree;
   pCodecInfo->mPlgEnum = plgEnum;
   pCodecInfo->mPlgSignaling = plgSignaling;

   MpCodecFactory::addStaticCodec(pCodecInfo);
}

const char** MpCodecCallInfoV1::getSDPModes(unsigned& modesCount)
{
   const char** sdpStrModes;
   int res = mPlgEnum(NULL, &modesCount, &sdpStrModes);
   assert (res == RPLG_SUCCESS);
   return sdpStrModes;
}

int AutoRegistratorBase::nAutoRegistered = 0;

AutoRegistratorBase::AutoRegistratorBase()
{
   nAutoRegistered++;
}
