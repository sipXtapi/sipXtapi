//  
// Copyright (C) 2007-2017 SIPez LLC.  All rights reserved.
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

/* Parsing {param}={value} */
static int analizeParamEqValue(const char *parsingString, const char* paramName, int* value)
{
   int tmp;
   char c;
   int eqFound = FALSE;
   int digitFound = FALSE;  
   const char* res;
   res = strstr(parsingString, paramName);
   if (!res) {
      return -1;
   }
   res += strlen(paramName); //Skip name of param world

   for (; (c=*res) != 0; res++ )
   {
      if (isspace(c)) {
         if (digitFound) 
            break;
         continue;
      }
      if (c == '=') {
         if (eqFound) 
            goto end_of_analize;
         eqFound = TRUE;
         continue;            
      }
      if (isdigit(c)) {
         if (!eqFound) 
            goto end_of_analize;
         tmp = (c - '0');
         for (res++; isdigit(c=*res); res++) {
            tmp = tmp * 10 + (c - '0');
         }
         res--;
         digitFound = TRUE;
         continue;
      }

      /* Unexpected character */
      goto end_of_analize;
   }
   if (digitFound) {
      *value = tmp;
      return 0;
   }

end_of_analize:
   return -1;
}

extern "C" int getFmtpValueRange(const char* str, const char* param, int defValue, int minValue, int maxValue)
{
   int value;
   int res = (!str) ? (-1) : analizeParamEqValue(str, param, &value);
   if ((res == 0) && (value >= minValue) && (value <= maxValue))
      return value;
   return defValue;
}

