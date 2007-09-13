//  
// Copyright (C) 2006-2007 SIPez LLC. 
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

#include <assert.h>
#include <utl/UtlInit.h>
#include <mp/MpCodecFactory.h>
#include <os/OsSysLog.h>

#include <mp/MpePtAVT.h>
#include <mp/MpdPtAVT.h>

#include <os/OsSharedLibMgr.h>
#include <utl/UtlSListIterator.h>
#include <os/OsFS.h>

#include <mp/MpPlgEncoderWrap.h>
#include <mp/MpPlgDecoderWrap.h>

#include <sdp/SdpCodec.h>

class MpCodecSubInfo : public UtlVoidPtr
{
public:

   MpCodecSubInfo(MpCodecCallInfoV1* pCodecCall,
                  SdpCodec::SdpCodecTypes assignedSDPnum,
                  const char* pMimeSubtype)
      : mpCodecCall(pCodecCall)
      , mAssignedSDPnum(assignedSDPnum)
      , mpMimeSubtype(pMimeSubtype)
   {
   }

   ~MpCodecSubInfo()
   {
      delete mpCodecCall;
   }

   const char* getMIMEtype() const
   { return mpMimeSubtype; }

   SdpCodec::SdpCodecTypes getSDPtype() const
   { return mAssignedSDPnum; }

   MpCodecCallInfoV1* getCodecCall() const
   { return mpCodecCall; }

protected:
   MpCodecCallInfoV1* mpCodecCall;
   SdpCodec::SdpCodecTypes mAssignedSDPnum;
   const char* mpMimeSubtype;
};


MpCodecFactory* MpCodecFactory::spInstance = NULL;
OsBSem MpCodecFactory::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);


int MpCodecFactory::maxDynamicCodecTypeAssigned = 0;
UtlBoolean MpCodecFactory::fCacheListMustUpdate = FALSE;
SdpCodec::SdpCodecTypes* MpCodecFactory::pCodecs = NULL;

UtlSList MpCodecFactory::mCodecsInfo;

MpWorkaroundSDPNumList gWorkaroudForOldAPI[] =
{
   { SdpCodec::SDP_CODEC_PCMU,    "pcmu",   NULL  },
   { SdpCodec::SDP_CODEC_GSM,     "gsm",    NULL  },
   { SdpCodec::SDP_CODEC_G723,    "g723",   NULL  },
   { SdpCodec::SDP_CODEC_PCMA,    "pcma",   NULL  },
   { SdpCodec::SDP_CODEC_SPEEX,   "speex",  NULL  },
   { SdpCodec::SDP_CODEC_SPEEX_5, "speex",  "mode=2"  },
   { SdpCodec::SDP_CODEC_SPEEX_15,"speex",  "mode=5"  },
   { SdpCodec::SDP_CODEC_SPEEX_24,"speex",  "mode=7"  },
   { SdpCodec::SDP_CODEC_ILBC,    "ilbc",   NULL  }
};

#define SIZEOF_WORKAROUND_LIST     \
   (sizeof(gWorkaroudForOldAPI) / sizeof(gWorkaroudForOldAPI[0]))

static MpWorkaroundSDPNumList* searchWorkAroundSlot(SdpCodec::SdpCodecTypes num)
{
   int i;

   for (i = 0; i < SIZEOF_WORKAROUND_LIST; i++) {
      if (gWorkaroudForOldAPI[i].mPredefinedSDPnum == num)
         return &gWorkaroudForOldAPI[i];
   }
   return NULL;
}

/* ============================ CREATORS ================================== */

// Return a pointer to the MpCodecFactory singleton object, creating 
// it if necessary

MpCodecFactory* MpCodecFactory::getMpCodecFactory(void)
{
   // If the object already exists, then use it
   if (spInstance == NULL)
   {
      // If the object does not yet exist, then acquire
      // the lock to ensure that only one instance of the object is 
      // created
      sLock.acquire();
      if (spInstance == NULL)
         spInstance = new MpCodecFactory();
      sLock.release();

      spInstance->initializeStaticCodecs();
   }
   return spInstance;
}

MpCodecFactory::MpCodecFactory(void)
{
}

//:Destructor
MpCodecFactory::~MpCodecFactory()
{
}

// Returns a new instance of a decoder of the indicated type
// param: internalCodecId - (in) codec type identifier
// param: payloadType - (in) RTP payload type associated with this decoder
// param: rpDecoder - (out) Reference to a pointer to the new decoder object
OsStatus MpCodecFactory::createDecoder(SdpCodec::SdpCodecTypes internalCodecId,
                                       int payloadType, MpDecoderBase*& rpDecoder)
{
   rpDecoder=NULL;

   if (internalCodecId == SdpCodec::SDP_CODEC_TONES) {
      rpDecoder = new MpdPtAVT(payloadType);
      return OS_SUCCESS;
   }

   MpCodecSubInfo* codec = NULL;
   MpWorkaroundSDPNumList* slot = searchWorkAroundSlot(internalCodecId);
   if (slot) {
      UtlString str = slot->mimeSubtype;
      codec = searchByMIME(str);
   }
 
   if (codec) {      
      rpDecoder = new MpPlgDecoderWrapper(payloadType, *codec->getCodecCall(), slot->extraMode);
      ((MpPlgDecoderWrapper*)rpDecoder)->setAssignedSDPNum(internalCodecId);

   } else {
      OsSysLog::add(FAC_MP, PRI_WARNING, 
         "MpCodecFactory::createDecoder unknown codec type "
         "internalCodecId = (SdpCodec::SdpCodecTypes) %d, "
         "payloadType = %d",
         internalCodecId, payloadType);

      assert(FALSE);
   }

   if (NULL != rpDecoder) 
   {
      return OS_SUCCESS;
   }
   return OS_INVALID_ARGUMENT;
}

OsStatus MpCodecFactory::createEncoder(SdpCodec::SdpCodecTypes internalCodecId,
                                       int payloadType, MpEncoderBase*& rpEncoder)
{
   rpEncoder=NULL;
   if (internalCodecId == SdpCodec::SDP_CODEC_TONES) {
      rpEncoder = new MpePtAVT(payloadType);
      return OS_SUCCESS;
   }

   MpCodecSubInfo* codec = NULL;
   MpWorkaroundSDPNumList* slot = searchWorkAroundSlot(internalCodecId);
   if (slot) {
      UtlString str = slot->mimeSubtype;
      codec = searchByMIME(str);
   }

   if (codec) {      
      rpEncoder = new MpPlgEncoderWrapper(payloadType, *codec->getCodecCall(), slot->extraMode);
      ((MpPlgEncoderWrapper*)rpEncoder)->setAssignedSDPNum(internalCodecId);

   } else {
      OsSysLog::add(FAC_MP, PRI_WARNING, 
         "MpCodecFactory::createEncoder unknown codec type "
         "internalCodecId = (SdpCodec::SdpCodecTypes) %d, "
         "payloadType = %d",
         internalCodecId, payloadType);
         
      assert(FALSE);
   }

   if (NULL != rpEncoder) 
   {
      return OS_SUCCESS;
   }
   return OS_INVALID_ARGUMENT;
}

   
MpCodecSubInfo* MpCodecFactory::searchByMIME(UtlString& str)
{
   UtlSListIterator iter(mCodecsInfo);
   MpCodecSubInfo* pinfo;

   while ((pinfo = (MpCodecSubInfo*)iter()))
   { 
      if (str.compareTo(pinfo->getMIMEtype()) == 0)
         return pinfo;
   }
   return NULL;
}

MpCodecCallInfoV1* MpCodecFactory::addStaticCodec(MpCodecCallInfoV1* sStaticCode)
{
    sStaticCodecsV1 = (MpCodecCallInfoV1 *)sStaticCode->bound(MpCodecFactory::sStaticCodecsV1);
    return sStaticCodecsV1;
}

void MpCodecFactory::freeAllLoadedLibsAndCodec()
{
   OsSharedLibMgrBase* pShrMgr = OsSharedLibMgr::getOsSharedLibMgr();
   //UtlSListIterator iter(mDynCodecs);

   sLock.acquire();

   UtlSListIterator iter(mCodecsInfo);
   MpCodecSubInfo* pinfo;

   UtlSList libLoaded;
   UtlString* libName;

   while ((pinfo = (MpCodecSubInfo*)iter()))
   {  
      if ((!pinfo->getCodecCall()->mbStatic) && 
         (!libLoaded.find(&pinfo->getCodecCall()->mModuleName))) {
         libLoaded.insert(&pinfo->getCodecCall()->mModuleName);         
      }    

      if (!pinfo->getCodecCall()->mbStatic) {
         mCodecsInfo.remove(pinfo);
         delete pinfo;         
      }
   }

   sLock.release();

   UtlSListIterator iter2(libLoaded);
   while ((libName = (UtlString*)iter2()))
   {
      pShrMgr->unloadSharedLib(libName->data());
   }
}



OsStatus MpCodecFactory::loadAllDynCodecs(const char* path, const char* regexFilter)
{
   OsPath ospath = path;
   OsPath module;
   OsFileIterator fi(ospath);

   OsStatus res;
   res = fi.findFirst(module, regexFilter);

   if (res != OS_SUCCESS) 
      return OS_FAILED;

   do {
      UtlString str = path;
      str += "\\";
      str += module.data();
      loadDynCodec(str.data());
   } while (fi.findNext(module) == OS_SUCCESS);

   return OS_SUCCESS;
}

OsStatus MpCodecFactory::loadDynCodec(const char* name)
{
   OsStatus res;
   OsSharedLibMgrBase* pShrMgr = OsSharedLibMgr::getOsSharedLibMgr();
   
   res = pShrMgr->loadSharedLib(name);
   if (res != OS_SUCCESS)
   {
      return OS_FAILED;
   }
 
   void* address;   
   res = pShrMgr->getSharedLibSymbol(name, MSK_GET_CODEC_NAME_V1, address);
   if (res != OS_SUCCESS)
   {
      pShrMgr->unloadSharedLib(name);
      return OS_FAILED;
   }

   dlGetCodecsV1 getCodecsV1 = (dlGetCodecsV1)address;
   const char* codecName;
   int i, r, count = 0;

   // 100 is a watchdog value, should be enough for everyone.
   for (i = 0; (i < 100); i++) 
   {
      r = getCodecsV1(i, &codecName);
      if ((r != RPLG_SUCCESS) || (codecName == NULL)) 
      {
         if (count == 0) {
            pShrMgr->unloadSharedLib(name);
            return OS_FAILED;
         }
         return OS_SUCCESS;
      }

      // Obtaining codecs functions
      UtlBoolean st;
      UtlBoolean stSignaling;

      UtlString strCodecName = codecName;
      UtlString dlNameInit = strCodecName + MSK_INIT_V1;
      UtlString dlNameDecode = strCodecName + MSK_DECODE_V1;
      UtlString dlNameEncdoe = strCodecName + MSK_ENCODE_V1;
      UtlString dlNameFree = strCodecName + MSK_FREE_V1;
      UtlString dlNameEnum = strCodecName + MSK_ENUM_V1;
      UtlString dlNameSignaling = strCodecName + MSK_SIGNALING_V1;
      
      dlPlgInitV1 plgInitAddr;
      dlPlgDecodeV1 plgDecodeAddr;
      dlPlgEncodeV1 plgEncodeAddr;
      dlPlgFreeV1 plgFreeAddr;
      dlPlgEnumSDPAndModesV1 plgEnum;
      dlPlgGetSignalingDataV1 plgSignaling;

      st = TRUE 
         && (pShrMgr->getSharedLibSymbol(name, dlNameInit, address) == OS_SUCCESS)
         && ((plgInitAddr = (dlPlgInitV1)address) != NULL)
         && (pShrMgr->getSharedLibSymbol(name, dlNameDecode, address) == OS_SUCCESS)
         && ((plgDecodeAddr = (dlPlgDecodeV1)address) != NULL)
         && (pShrMgr->getSharedLibSymbol(name, dlNameEncdoe, address) == OS_SUCCESS)
         && ((plgEncodeAddr = (dlPlgEncodeV1)address) != NULL)
         && (pShrMgr->getSharedLibSymbol(name, dlNameFree, address) == OS_SUCCESS)
         && ((plgFreeAddr = (dlPlgFreeV1)address) != NULL)
         && (pShrMgr->getSharedLibSymbol(name, dlNameEnum, address) == OS_SUCCESS)
         && ((plgEnum = (dlPlgEnumSDPAndModesV1)address) != NULL);

      stSignaling = TRUE 
         && (pShrMgr->getSharedLibSymbol(name, dlNameSignaling, address) == OS_SUCCESS)  
         && ((plgSignaling = (dlPlgGetSignalingDataV1)address) != NULL);

      if (!stSignaling)
            plgSignaling = NULL;

      // Test if the codec could enumerate SDP
      unsigned enumCount;
      const char *mime;
      const char **tmp;
      UtlBoolean bPlgCouldEnum = (plgEnum(&mime, &enumCount, &tmp) == RPLG_SUCCESS);

      if (st &&  bPlgCouldEnum)
      {
         //Add codec to list
         MpCodecCallInfoV1* pci = new MpCodecCallInfoV1(name, codecName, 
            plgInitAddr, plgDecodeAddr, plgEncodeAddr, 
            plgFreeAddr, plgEnum, plgSignaling, FALSE);

         if (!pci)
            continue;         

         if (addCodecWrapperV1(pci) != OS_SUCCESS)
         {
            delete pci;
            continue;
         }

         //Plugin has been added successfully, need to rebuild cache list
         fCacheListMustUpdate = TRUE;
         count ++;
      }
   }
   if (count == 0) {
      pShrMgr->unloadSharedLib(name);
      return OS_FAILED;
   }
   return OS_SUCCESS;
}

OsStatus MpCodecFactory::addCodecWrapperV1(MpCodecCallInfoV1* wrapper)
{
   MpCodecSubInfo* mpsi;
   UtlString str;
   const char* tmpstr;
   int sdpNum;
   int res = wrapper->mPlgEnum(&tmpstr, NULL, NULL);
   if (res != RPLG_SUCCESS) {
      return OS_FAILED;
   }
   str = tmpstr;
   str.toLower();
   sdpNum = assignAudioSDPnumber(str);

   mpsi = new MpCodecSubInfo(wrapper, (SdpCodec::SdpCodecTypes)sdpNum, tmpstr);
   if (mpsi == NULL) {
      return OS_NO_MEMORY;
   }

   sLock.acquire();
   mCodecsInfo.insert(mpsi);
   sLock.release();

   return OS_SUCCESS;
}

void MpCodecFactory::initializeStaticCodecs() //Should be called from mpStartup()
{
   MpCodecCallInfoV1* tmp = sStaticCodecsV1;
   for ( ;tmp; tmp = tmp->next())
   {
      addCodecWrapperV1(tmp);
   }
}

SdpCodec::SdpCodecTypes* MpCodecFactory::getAllCodecTypes(unsigned& count)
{
   // NOT implemented yet
   return NULL;
}

const char** MpCodecFactory::getAllCodecModes(SdpCodec::SdpCodecTypes codecId, unsigned& count)
{
   // NOT implemented yet
   return NULL;
}

int MpCodecFactory::assignAudioSDPnumber(const UtlString& mimeSubtypeInLowerCase)
{
   struct knownSDPnums {
      int sdpNum;
      const char* mimeSubtype;
   };

   const knownSDPnums statics[] = {
      { SdpCodec::SDP_CODEC_PCMU, "pcmu" },
      { SdpCodec::SDP_CODEC_GSM,  "gsm" },
      { SdpCodec::SDP_CODEC_G723, "g723" },
      { SdpCodec::SDP_CODEC_PCMA, "pcma" },
      { SdpCodec::SDP_CODEC_G729, "g729" }
   };

   int i;

   for (i = 0; i < (sizeof(statics) / sizeof(statics[0])); i++ )
   {
      if (mimeSubtypeInLowerCase.compareTo(statics[i].mimeSubtype) == 0) 
      {
         return statics[i].sdpNum;
      }
   }

   //Not found in statics, add number for this mimeSubtype
   maxDynamicCodecTypeAssigned++;
   assert (maxDynamicCodecTypeAssigned > 0);
   
   return SdpCodec::SDP_CODEC_MAXIMUM_STATIC_CODEC + maxDynamicCodecTypeAssigned;
}


void MpCodecFactory::updateCodecArray(void)
{
   // NOT implemented yet
}