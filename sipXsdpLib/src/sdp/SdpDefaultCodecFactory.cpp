//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <sdp/SdpDefaultCodecFactory.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */
SdpCodec SdpDefaultCodecFactory::getCodec(SdpCodec::SdpCodecTypes internalCodecId)
{
   switch(internalCodecId)
   {

   case SdpCodec::SDP_CODEC_L16_8K:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_L16_8K,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_AUDIO,
                         "pcm",
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW);
      }
      break;

   case SdpCodec::SDP_CODEC_TONES:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_TONES,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_DTMF_TONES,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;

   case SdpCodec::SDP_CODEC_G729A:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_G729A,
                         SdpCodec::SDP_CODEC_G729,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_G729A,
                         8000,
                         10000,
                         1,
                         "annexb=no",
                         SdpCodec::SDP_CODEC_CPU_HIGH,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;
   case SdpCodec::SDP_CODEC_G729AB:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_G729AB,
                         SdpCodec::SDP_CODEC_G729,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_G729AB,
                         8000,
                         10000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_HIGH,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;
   case SdpCodec::SDP_CODEC_G729ACISCO7960:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_G729A,
                         SdpCodec::SDP_CODEC_G729,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_G729ACISCO7960,
                         8000,
                         10000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_HIGH,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;

   case SdpCodec::SDP_CODEC_PCMA:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_PCMA,
                         SdpCodec::SDP_CODEC_PCMA,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_PCMA,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;

   case SdpCodec::SDP_CODEC_PCMU:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_PCMU,
                         SdpCodec::SDP_CODEC_PCMU,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_PCMU,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;

   case SdpCodec::SDP_CODEC_GIPS_IPCMA:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_GIPS_IPCMA,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_IPCMA,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;

   case SdpCodec::SDP_CODEC_GIPS_IPCMU:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_GIPS_IPCMU,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_IPCMU,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;

   case SdpCodec::SDP_CODEC_ILBC:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_ILBC,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_ILBC,
                         8000,
                         30000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_HIGH,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;

   case SdpCodec::SDP_CODEC_GIPS_ISAC:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_GIPS_ISAC,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_ISAC,
                         16000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_HIGH,
                         SDP_CODEC_BANDWIDTH_VARIABLE);
      }
      break;
   case SdpCodec::SDP_CODEC_GSM:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_GSM,
                         SdpCodec::SDP_CODEC_GSM,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_GSM,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_HIGH,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;
   case SdpCodec::SDP_CODEC_G723:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_G723,
                         SdpCodec::SDP_CODEC_G723,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_G723,
                         8000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_HIGH,
                         SDP_CODEC_BANDWIDTH_LOW); 
      }
      break;

   case SdpCodec::SDP_CODEC_GIPS_IPCMWB:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_GIPS_IPCMWB,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_IPCMWB,
                         16000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_HIGH);
      }
      break;

   case SdpCodec::SDP_CODEC_SPEEX:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_SPEEX,
                         SdpCodec::SDP_CODEC_SPEEX,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_SPEEX,
                         8000,
                         20000,
                         1,
                         "mode=3",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;
   case SdpCodec::SDP_CODEC_SPEEX_5:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_SPEEX_5,
                         SdpCodec::SDP_CODEC_SPEEX_5,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_SPEEX,
                         8000,
                         20000,
                         1,
                         "mode=2",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_LOW);
      }
      break;
   case SdpCodec::SDP_CODEC_SPEEX_15:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_SPEEX_15,
                         SdpCodec::SDP_CODEC_SPEEX_15,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_SPEEX,
                         8000,
                         20000,
                         1,
                         "mode=5",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;
   case SdpCodec::SDP_CODEC_SPEEX_24:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_SPEEX_24,
                         SdpCodec::SDP_CODEC_SPEEX_24,
                         MIME_TYPE_AUDIO,
                         MIME_SUBTYPE_SPEEX,
                         8000,
                         20000,
                         1,
                         "mode=7",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL);
      }
      break;

   case SdpCodec::SDP_CODEC_VP71_CIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_VP71_CIF,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_VP71,
                         90000,
                         20000,
                         1,
                         "size=CIF/QCIF/SQCIF",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL,
                         SDP_VIDEO_FORMAT_CIF);
      }
      break;

   case SdpCodec::SDP_CODEC_VP71_QCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_VP71_QCIF,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_VP71,
                         90000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL,
                         SDP_VIDEO_FORMAT_QCIF);
      }
      break;

   case SdpCodec::SDP_CODEC_VP71_SQCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_VP71_SQCIF,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_VP71,
                         90000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL,
                         SDP_VIDEO_FORMAT_SQCIF);
      }
      break;

   case SdpCodec::SDP_CODEC_VP71_QVGA:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_VP71_QVGA,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_VP71,
                         90000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL,
                         SDP_VIDEO_FORMAT_QVGA);
      }
      break;

   case SdpCodec::SDP_CODEC_IYUV_CIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_IYUV_CIF,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_IYUV,
                         90000,
                         20000,
                         1,
                         "size=CIF/QCIF/SQCIF",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_HIGH,
                         SDP_VIDEO_FORMAT_CIF);
      }
      break;

   case SdpCodec::SDP_CODEC_IYUV_QCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_IYUV_QCIF,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_IYUV,
                         90000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_HIGH,
                         SDP_VIDEO_FORMAT_QCIF);
      }
      break;

   case SdpCodec::SDP_CODEC_IYUV_SQCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_IYUV_SQCIF,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_IYUV,
                         90000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_HIGH,
                         SDP_VIDEO_FORMAT_SQCIF);
      }
      break;

   case SdpCodec::SDP_CODEC_IYUV_QVGA:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_IYUV_QVGA,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_IYUV,
                         90000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_HIGH,
                         SDP_VIDEO_FORMAT_QVGA);
      }
      break;

   case SdpCodec::SDP_CODEC_I420_CIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_I420_CIF,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_I420,
                         90000,
                         20000,
                         1,
                         "size=CIF/QCIF/SQCIF",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_HIGH,
                         SDP_VIDEO_FORMAT_CIF);
      }
      break;

   case SdpCodec::SDP_CODEC_I420_QCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_I420_QCIF,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_I420,
                         90000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_HIGH,
                         SDP_VIDEO_FORMAT_QCIF);
      }
      break;

   case SdpCodec::SDP_CODEC_I420_SQCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_I420_SQCIF,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_I420,
                         90000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_HIGH,
                         SDP_VIDEO_FORMAT_SQCIF);
      }
      break;

   case SdpCodec::SDP_CODEC_I420_QVGA:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_I420_QVGA,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_I420,
                         90000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_HIGH,
                         SDP_VIDEO_FORMAT_QVGA);
      }
      break;

   case SdpCodec::SDP_CODEC_RGB24_CIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_RGB24_CIF,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_RGB24,
                         90000,
                         20000,
                         1,
                         "size=CIF/QCIF/SQCIF",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_HIGH,
                         SDP_VIDEO_FORMAT_CIF);
      }
      break;

   case SdpCodec::SDP_CODEC_RGB24_QCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_RGB24_QCIF,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_RGB24,
                         90000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_HIGH,
                         SDP_VIDEO_FORMAT_QCIF);
      }
      break;

   case SdpCodec::SDP_CODEC_RGB24_SQCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_RGB24_SQCIF,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_RGB24,
                         90000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_HIGH,
                         SDP_VIDEO_FORMAT_SQCIF);
      }
      break;

   case SdpCodec::SDP_CODEC_H263_CIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_H263_CIF,
                         SdpCodec::SDP_CODEC_H263,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_H263,
                         90000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL,
                         SDP_VIDEO_FORMAT_CIF);
      }
      break;

   case SdpCodec::SDP_CODEC_H263_QCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_H263_QCIF,
                         SdpCodec::SDP_CODEC_H263,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_H263,
                         90000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL,
                         SDP_VIDEO_FORMAT_QCIF);
      }
      break;

   case SdpCodec::SDP_CODEC_H263_SQCIF:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_H263_SQCIF,
                         SdpCodec::SDP_CODEC_H263,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_H263,
                         90000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_NORMAL,
                         SDP_VIDEO_FORMAT_SQCIF);
      }
      break;

   case SdpCodec::SDP_CODEC_H263_QVGA:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_H263_QVGA,
                         SdpCodec::SDP_CODEC_H263,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_H263,
                         90000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_HIGH,
                         SDP_VIDEO_FORMAT_QVGA);
      }
      break;

   case SdpCodec::SDP_CODEC_RGB24_QVGA:
      {
         return SdpCodec(SdpCodec::SDP_CODEC_RGB24_QVGA,
                         SdpCodec::SDP_CODEC_UNKNOWN,
                         MIME_TYPE_VIDEO,
                         MIME_SUBTYPE_RGB24,
                         90000,
                         20000,
                         1,
                         "",
                         SdpCodec::SDP_CODEC_CPU_LOW,
                         SDP_CODEC_BANDWIDTH_HIGH,
                         SDP_VIDEO_FORMAT_QVGA);
      }
      break;

   default:
      osPrintf("\n\n  **** Unsupported codec: %d **** \n\n",
               internalCodecId);
      assert(FALSE);
      return SdpCodec();
   }
}


SdpCodec::SdpCodecTypes SdpDefaultCodecFactory::getCodecType(const char* pCodecName)
{
    SdpCodec::SdpCodecTypes retType = SdpCodec::SDP_CODEC_UNKNOWN;
    UtlString compareString(pCodecName);

    compareString.toUpper();

    if (strcmp(compareString,"TELEPHONE-EVENT") == 0 ||
       strcmp(compareString,"AUDIO/TELEPHONE-EVENT") == 0 || 
       strcmp(compareString,"AVT-TONES") == 0 ||
       strcmp(compareString,"AVT") == 0)
        retType = SdpCodec::SDP_CODEC_TONES;
    else
    if (strcmp(compareString,"PCMU") == 0 ||
       strcmp(compareString,"G711U") == 0)
        retType = SdpCodec::SDP_CODEC_PCMU;
    else
    if (strcmp(compareString,"PCMA") == 0 ||
       strcmp(compareString,"G711A") == 0)
        retType = SdpCodec::SDP_CODEC_PCMA;
    else
    if (strcmp(compareString,"EG711U") == 0)
        retType = SdpCodec::SDP_CODEC_GIPS_IPCMU;
    else
    if (strcmp(compareString,"EG711A") == 0)
        retType = SdpCodec::SDP_CODEC_GIPS_IPCMA;
    else
    if (strcmp(compareString,"IPCMWB") == 0)
        retType = SdpCodec::SDP_CODEC_GIPS_IPCMWB;
    else
    if (strcmp(compareString,"G729") == 0 ||
       strcmp(compareString,"G729A") == 0)
        retType = SdpCodec::SDP_CODEC_G729A;
    else
    if (strcmp(compareString,"G729AB") == 0 ||
       strcmp(compareString,"G729B") == 0)
        retType = SdpCodec::SDP_CODEC_G729AB;
    else
    if (strcmp(compareString,"G723") == 0)
        retType = SdpCodec::SDP_CODEC_G723;
    else
    if (strcmp(compareString,"G729A-FOR-CISCO-7960") == 0)
        retType = SdpCodec::SDP_CODEC_G729ACISCO7960;
    else
    if (strcmp(compareString,"ILBC") == 0)
        retType = SdpCodec::SDP_CODEC_ILBC;
    else
    if (strcmp(compareString,"GSM") == 0)
        retType = SdpCodec::SDP_CODEC_GSM;
   else
    if (strcmp(compareString,"ISAC") == 0)
        retType = SdpCodec::SDP_CODEC_GIPS_ISAC;
   else 
      if (strcmp(compareString,"SPEEX") == 0)
         retType = SdpCodec::SDP_CODEC_SPEEX;
   else 
      if (strcmp(compareString,"SPEEX_5") == 0)
         retType = SdpCodec::SDP_CODEC_SPEEX_5;
   else 
      if (strcmp(compareString,"SPEEX_15") == 0)
         retType = SdpCodec::SDP_CODEC_SPEEX_15;
   else 
      if (strcmp(compareString,"SPEEX_24") == 0)
         retType = SdpCodec::SDP_CODEC_SPEEX_24;
   else 
      if (strcmp(compareString,"GSM") == 0)
         retType = SdpCodec::SDP_CODEC_GSM;
   else
    if (strcmp(compareString,"VP71-CIF") == 0)
        retType = SdpCodec::SDP_CODEC_VP71_CIF;
   else
    if (strcmp(compareString,"VP71-QCIF") == 0)
        retType = SdpCodec::SDP_CODEC_VP71_QCIF;
   else
    if (strcmp(compareString,"VP71-SQCIF") == 0)
        retType = SdpCodec::SDP_CODEC_VP71_SQCIF;
   else
    if (strcmp(compareString,"VP71-QVGA") == 0)
        retType = SdpCodec::SDP_CODEC_VP71_QVGA;
   else
    if (strcmp(compareString,"IYUV-CIF") == 0)
        retType = SdpCodec::SDP_CODEC_IYUV_CIF;
   else
    if (strcmp(compareString,"IYUV-QCIF") == 0)
        retType = SdpCodec::SDP_CODEC_IYUV_QCIF;
   else
    if (strcmp(compareString,"IYUV-SQCIF") == 0)
        retType = SdpCodec::SDP_CODEC_IYUV_SQCIF;
   else
    if (strcmp(compareString,"IYUV-QVGA") == 0)
        retType = SdpCodec::SDP_CODEC_IYUV_QVGA;
   else
    if (strcmp(compareString,"I420-CIF") == 0)
        retType = SdpCodec::SDP_CODEC_I420_CIF;
   else
    if (strcmp(compareString,"I420-QCIF") == 0)
        retType = SdpCodec::SDP_CODEC_I420_QCIF;
   else
    if (strcmp(compareString,"I420-SQCIF") == 0)
        retType = SdpCodec::SDP_CODEC_I420_SQCIF;
   else
    if (strcmp(compareString,"I420-QVGA") == 0)
        retType = SdpCodec::SDP_CODEC_I420_QVGA;
   else
    if (strcmp(compareString,"H263-CIF") == 0)
        retType = SdpCodec::SDP_CODEC_H263_CIF;
   else
    if (strcmp(compareString,"H263-QCIF") == 0)
        retType = SdpCodec::SDP_CODEC_H263_QCIF;
   else
    if (strcmp(compareString,"H263-SQCIF") == 0)
        retType = SdpCodec::SDP_CODEC_H263_SQCIF;
   else
    if (strcmp(compareString,"H263-QVGA") == 0)
        retType = SdpCodec::SDP_CODEC_H263_QVGA;
   else
    if (strcmp(compareString,"RGB24-CIF") == 0)
        retType = SdpCodec::SDP_CODEC_RGB24_CIF;
   else
    if (strcmp(compareString,"RGB24-QCIF") == 0)
        retType = SdpCodec::SDP_CODEC_RGB24_QCIF;
   else
    if (strcmp(compareString,"RGB24-SQCIF") == 0)
        retType = SdpCodec::SDP_CODEC_RGB24_SQCIF;
   else
    if (strcmp(compareString,"RGB24-QVGA") == 0)
        retType = SdpCodec::SDP_CODEC_RGB24_QVGA;
    else
       retType = SdpCodec::SDP_CODEC_UNKNOWN;
    return retType;
}

OsStatus SdpDefaultCodecFactory::getCodecNameByType(SdpCodec::SdpCodecTypes type, UtlString& codecName)
{
   OsStatus rc = OS_SUCCESS;

   switch (type)
   {
   case SdpCodec::SDP_CODEC_TONES:
      codecName = SIPX_CODEC_ID_TELEPHONE;
      break;
   case SdpCodec::SDP_CODEC_G729A:
      codecName = SIPX_CODEC_ID_G729;
      break;
   case SdpCodec::SDP_CODEC_PCMA:
      codecName = SIPX_CODEC_ID_PCMA;
      break;
   case SdpCodec::SDP_CODEC_PCMU:
      codecName = SIPX_CODEC_ID_PCMU;
      break;
   case SdpCodec::SDP_CODEC_GIPS_IPCMA:
      codecName = SIPX_CODEC_ID_EG711A;
      break;
   case SdpCodec::SDP_CODEC_GIPS_IPCMU:
      codecName = SIPX_CODEC_ID_EG711U;
      break;
   case SdpCodec::SDP_CODEC_GIPS_IPCMWB:
      codecName = SIPX_CODEC_ID_IPCMWB;
      break;
   case SdpCodec::SDP_CODEC_ILBC:
      codecName = SIPX_CODEC_ID_ILBC;
      break;
   case SdpCodec::SDP_CODEC_GIPS_ISAC:
      codecName = SIPX_CODEC_ID_ISAC;
      break;
   case SdpCodec::SDP_CODEC_SPEEX:
      codecName = SIPX_CODEC_ID_SPEEX;
      break;
   case SdpCodec::SDP_CODEC_SPEEX_5:
      codecName = SIPX_CODEC_ID_SPEEX_5;
      break;
   case SdpCodec::SDP_CODEC_SPEEX_15:
      codecName = SIPX_CODEC_ID_SPEEX_15;
      break;
   case SdpCodec::SDP_CODEC_SPEEX_24:
      codecName = SIPX_CODEC_ID_SPEEX_24;
      break;
   case SdpCodec::SDP_CODEC_GSM:
      codecName = SIPX_CODEC_ID_GSM;
      break;
   default:
      assert(!"Unsupported codec type");
      codecName = "";
      rc = OS_FAILED;
      osPrintf("sipXmediaFactoryImpl::getCodecNameByType(): "
               "Unsupported codec type %d.",
               type);

   }

   return rc;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

