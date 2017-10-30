//  
// Copyright (C) 2006-2017 SIPez LLC. All rights reserved.
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
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
#include <utl/UtlInit.h> // KEEP THIS ONE THE FIRST INCLUDE
#include <mp/MpCodecFactory.h>
#include <mp/MpEncoderBase.h>
#include <mp/MpDecoderBase.h>
#include <sdp/SdpDefaultCodecFactory.h>
#include <os/OsSysLog.h>
#include <os/OsSharedLibMgr.h>
#include <os/OsFS.h>
#include <utl/UtlHashBagIterator.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// DEFINES
// MACROS
// LOCAL TYPES DECLARATIONS
class MpCodecSubInfo : public UtlString
{
public:

   MpCodecSubInfo(const MpCodecCallInfoV1* pCodecCall,
                  const MppCodecInfoV1_1 *pCodecInfo)
   : UtlString(pCodecInfo->mimeSubtype)
   , mpCodecCall(pCodecCall)
   , mpCodecInfo(pCodecInfo)
   {
      // Store all MIME-subtypes in lower case to allow case insensitive
      // compare.
      toLower();
   }

   ~MpCodecSubInfo()
   {
      if (!mpCodecCall->isStatic())
         delete mpCodecCall;
      // Do not delete mpCodecInfo - we're just keeping pointer to it.
   }

   const MpCodecCallInfoV1* getCodecCall() const
   { return mpCodecCall; }

   const MppCodecInfoV1_1 *getCodecInfo() const
   { return mpCodecInfo; }

protected:
   const MpCodecCallInfoV1 *mpCodecCall;
   const MppCodecInfoV1_1    *mpCodecInfo;
};

// STATIC VARIABLE INITIALIZATIONS
MpCodecFactory* MpCodecFactory::spInstance = NULL;
OsBSem MpCodecFactory::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

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

void MpCodecFactory::freeSingletonHandle()
{
   sLock.acquire();
   if (spInstance != NULL)
   {
      delete spInstance;
      spInstance = NULL;
      freeStaticCodecs();
   }
   sLock.release();
}

MpCodecFactory::MpCodecFactory(void)
: mCodecInfoCacheValid(FALSE)
, mCachedCodecInfoNum(0)
, mpCodecInfoCache(NULL)
{

}

MpCodecFactory::~MpCodecFactory()
{
   freeAllLoadedLibsAndCodec();

   MpCodecSubInfo* pinfo;

   UtlHashBagIterator iter(mCodecsInfo);
   while ((pinfo = (MpCodecSubInfo*)iter()))
   { 
      if (!pinfo->getCodecCall()->isStatic()) {
         assert(!"Dynamically loaded codecs must be unloaded already");
      }
      delete pinfo;     
   }
   mCodecsInfo.removeAll();

   delete[] mpCodecInfoCache;
}

/* ============================= MANIPULATORS ============================= */

OsStatus MpCodecFactory::loadDynCodec(const char* name)
{
   OsSysLog::add(FAC_MP, PRI_INFO, "MpCodecFactory::loadDynCodec(\"%s\")", name);

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
   UtlString codecTokens;

   // 100 is a watchdog value, should be enough for everyone.
   for (i = 0; (i < 100); i++) 
   {
      r = getCodecsV1(i, &codecName);
      if ((r != RPLG_SUCCESS) || (codecName == NULL)) 
      {
         if (count == 0)
         {
            pShrMgr->unloadSharedLib(name);
            return OS_FAILED;
         }
         return OS_SUCCESS;
      }

      OsSysLog::add(FAC_MP, PRI_INFO, "found codec: %s", codecName);

      if(i > 0)
      {
         codecTokens.append(' ');
      }
      codecTokens.append(codecName);

      // Obtaining codecs functions
      UtlBoolean st;
      UtlBoolean stGetPacketSamples;
      UtlBoolean stSignaling;

      UtlString strCodecName = codecName;
      UtlString dlNameInit = strCodecName + MSK_INIT_V1_2;
      UtlString dlNameGetInfo = strCodecName + MSK_GET_INFO_V1_1;
      UtlString dlNameGetPacketSamples = strCodecName + MSK_GET_PACKET_SAMPLES_V1_2;
      UtlString dlNameDecode = strCodecName + MSK_DECODE_V1;
      UtlString dlNameEncdoe = strCodecName + MSK_ENCODE_V1;
      UtlString dlNameFree = strCodecName + MSK_FREE_V1;
      UtlString dlNameSignaling = strCodecName + MSK_SIGNALING_V1;
      
      dlPlgInitV1_2 plgInitAddr;
      dlPlgGetInfoV1_1 plgGetInfoAddr;
      dlPlgGetPacketSamplesV1_2 plgGetPacketSamples;
      dlPlgDecodeV1 plgDecodeAddr;
      dlPlgEncodeV1 plgEncodeAddr;
      dlPlgFreeV1 plgFreeAddr;
      dlPlgGetSignalingDataV1 plgSignaling;

      st = TRUE 
         && (pShrMgr->getSharedLibSymbol(name, dlNameInit,
                                         (void*&)plgInitAddr) == OS_SUCCESS)
         && (plgInitAddr != NULL)
         && (pShrMgr->getSharedLibSymbol(name, dlNameGetInfo,
                                         (void*&)plgGetInfoAddr) == OS_SUCCESS)
         && (plgGetInfoAddr != NULL)
         && (pShrMgr->getSharedLibSymbol(name, dlNameDecode,
                                         (void*&)plgDecodeAddr) == OS_SUCCESS)
         && (plgDecodeAddr != NULL)
         && (pShrMgr->getSharedLibSymbol(name, dlNameEncdoe,
                                         (void*&)plgEncodeAddr) == OS_SUCCESS)
         && (plgEncodeAddr != NULL)
         && (pShrMgr->getSharedLibSymbol(name, dlNameFree,
                                         (void*&)plgFreeAddr) == OS_SUCCESS)
         && (plgFreeAddr != NULL);

      if (st)
      {
         stGetPacketSamples = TRUE
            && (pShrMgr->getSharedLibSymbol(name, dlNameGetPacketSamples,
                                            (void*&)plgGetPacketSamples) == OS_SUCCESS)
            && (plgGetPacketSamples != NULL);

         stSignaling = TRUE
            && (pShrMgr->getSharedLibSymbol(name, dlNameSignaling,
                                            (void*&)plgSignaling) == OS_SUCCESS)
            && (plgSignaling != NULL);

         // Add codec to list if all basic (non-signaling) symbols are present.

         MpCodecCallInfoV1* pCallInfo = new MpCodecCallInfoV1(name, codecName, 
                                                              plgInitAddr,
                                                              plgGetInfoAddr,
                                                              plgGetPacketSamples,
                                                              plgDecodeAddr,
                                                              plgEncodeAddr,
                                                              plgFreeAddr,
                                                              plgSignaling,
                                                              FALSE);

         if (!pCallInfo)
            continue;         

         if (addCodecWrapperV1(pCallInfo) != OS_SUCCESS)
         {
            delete pCallInfo;
            continue;
         }

         //Plugin has been added successfully, need to rebuild cache list
         mCodecInfoCacheValid = FALSE;
         count ++;
      }
   }
   if (count == 0) {
      pShrMgr->unloadSharedLib(name);
      return OS_FAILED;
   }
   OsSysLog::add(FAC_MP, PRI_INFO, "Loaded %d codecs (%s) from %s", count, codecTokens.data(), name);

   return OS_SUCCESS;
}

OsStatus MpCodecFactory::loadAllDynCodecs(const char* path, const char* regexFilter)
{
   OsPath ospath = path;
   OsPath module;
   OsFileIterator fi(ospath);

   OsSysLog::add(FAC_MP, PRI_INFO, "MpCodecFactory::loadAllDynCodecs(\"%s\", \"%s\")",
                 path, regexFilter);

   OsStatus res;
   res = fi.findFirst(module, regexFilter);

   if (res != OS_SUCCESS) 
      return OS_NOT_FOUND;

   do {
      UtlString str = path;
      str += OsPathBase::separator;
      str += module.data();
      res = loadDynCodec(str.data());
      OsSysLog::add(FAC_MP, PRI_INFO, "MpCodecFactory::loadDynCodec(\"%s\") returned %d",
                    str.data(), res);
   } while (fi.findNext(module) == OS_SUCCESS);

   return OS_SUCCESS;
}

MpCodecCallInfoV1* MpCodecFactory::addStaticCodec(MpCodecCallInfoV1* sStaticCode)
{
    const struct MppCodecInfoV1_1 *codecInfo = NULL;
    sStaticCode->mPlgGetInfo(&codecInfo);
    if(codecInfo)
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG,
                      "binding static codec: %s from: %s",
                      codecInfo->mimeSubtype, codecInfo->codecManufacturer);
    }
    else
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG,
                      "binding static codec, could not get info");
    }
    sStaticCodecsV1 = (MpCodecCallInfoV1 *)sStaticCode->bound(MpCodecFactory::sStaticCodecsV1);
    return sStaticCodecsV1;
}

/* ============================== ACCESSORS =============================== */

OsStatus MpCodecFactory::createDecoder(const UtlString &mime,
                                       const UtlString &fmtp,
                                       int sampleRate,
                                       int numChannels,
                                       int payloadType,
                                       MpDecoderBase*& rpDecoder) const
{
   MpCodecSubInfo* codec = searchByMIME(mime, sampleRate, numChannels);
 
   if (codec)
   {      
      rpDecoder = new MpDecoderBase(payloadType,
                                    *codec->getCodecCall(),
                                    *codec->getCodecInfo(),
                                    fmtp);
   }
   else
   {
      OsSysLog::add(FAC_MP, PRI_WARNING, 
                    "MpCodecFactory::createDecoder unknown codec type "
                    "%s, fmtp=%s"
                    "payloadType = %d",
                    mime.data(), fmtp.data(), payloadType);
      OsSysLog::flush();
      assert(!"Could not find codec of given type!");
      rpDecoder=NULL;
   }

   if (NULL != rpDecoder) 
   {
      return OS_SUCCESS;
   }

   return OS_INVALID_ARGUMENT;
}

OsStatus MpCodecFactory::createEncoder(const UtlString &mime,
                                       const UtlString &fmtp,
                                       int sampleRate,
                                       int numChannels,
                                       int payloadType,
                                       MpEncoderBase*& rpEncoder) const
{
   MpCodecSubInfo* codec = searchByMIME(mime, sampleRate, numChannels);

   if (codec)
   {      
      rpEncoder = new MpEncoderBase(payloadType,
                                    *codec->getCodecCall(),
                                    *codec->getCodecInfo(),
                                    fmtp);
   }
   else
   {
      OsSysLog::add(FAC_MP, PRI_ERR, 
                    "MpCodecFactory::createEncoder unknown codec type "
                    "%s, fmtp=%s"
                    "payloadType = %d",
                    mime.data(), fmtp.data(), payloadType);
      rpEncoder=NULL;
   }

   if (NULL != rpEncoder) 
   {
      return OS_SUCCESS;
   }

   return OS_INVALID_ARGUMENT;
}

void MpCodecFactory::getCodecInfoArray(unsigned &count,
                                       const MppCodecInfoV1_1 **&codecInfoArray) const
{
   if (!mCodecInfoCacheValid)
   {
      updateCodecInfoCache();
   }

   count = mCachedCodecInfoNum;
   codecInfoArray = mpCodecInfoCache;
}

void MpCodecFactory::addCodecsToList(SdpCodecList &codecList) const
{
   UtlHashBagIterator iter(mCodecsInfo);

   MpCodecSubInfo* pCodec;
   while ((pCodec = (MpCodecSubInfo*)iter()))
   {
      const MppCodecInfoV1_1 *pCodecInfo = pCodec->getCodecInfo();
      if (pCodecInfo->fmtpsNum == 0)
      {
         SdpCodec::SdpCodecTypes codecType;
         OsStatus res = SdpDefaultCodecFactory::getCodecType(pCodecInfo->mimeSubtype,
                                                             pCodecInfo->sampleRate,
                                                             pCodecInfo->numChannels,
                                                             "",
                                                             codecType);
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                  "Codec added to list: [%3d]:%s/%d/%d fmtp=\"%s\"\n",
                  res==OS_SUCCESS?codecType:-1,
                  pCodecInfo->mimeSubtype, pCodecInfo->sampleRate, pCodecInfo->numChannels,
                  "");

         if (res == OS_SUCCESS)
         {
            codecList.addCodec(SdpDefaultCodecFactory::getCodec(codecType));
         }
      }
      else
      {
         for (unsigned fmtpIdx=0; fmtpIdx<pCodecInfo->fmtpsNum; fmtpIdx++)
         {
            SdpCodec::SdpCodecTypes codecType;
            OsStatus res = SdpDefaultCodecFactory::getCodecType(pCodecInfo->mimeSubtype,
                                                                pCodecInfo->sampleRate,
                                                                pCodecInfo->numChannels,
                                                                pCodecInfo->fmtps[fmtpIdx],
                                                                codecType);
            OsSysLog::add(FAC_MP, PRI_DEBUG,
                          "Codec added to list: [%3d]:%s/%d/%d fmtp=\"%s\"\n",
                          res==OS_SUCCESS?codecType:-1,
                          pCodecInfo->mimeSubtype, pCodecInfo->sampleRate, pCodecInfo->numChannels,
                          pCodecInfo->fmtps[fmtpIdx]);

            if (res == OS_SUCCESS)
            {
               codecList.addCodec(SdpDefaultCodecFactory::getCodec(codecType));
            }
         }
      }
   }
}

/* =============================== INQUIRY ================================ */


/* ////////////////////////////// PROTECTED /////////////////////////////// */


MpCodecSubInfo* MpCodecFactory::searchByMIME(const UtlString& mime,
                                             int sampleRate,
                                             int numChannels) const
{
   // Create a lower case copy of MIME-subtype string.
   UtlString mime_copy(mime);
   mime_copy.toLower();

   // Create iterator to list all codecs with given MIME subtype.
   UtlHashBagIterator iter(mCodecsInfo, &mime_copy);

   MpCodecSubInfo* pInfo;
   while ((pInfo = (MpCodecSubInfo*)iter()))
   { 
      if ((int)(pInfo->getCodecInfo()->sampleRate) == sampleRate &&
          (int)(pInfo->getCodecInfo()->numChannels) == numChannels)
      {
         return pInfo;
      }
   }

   return NULL;
/*
   // Create a lower case copy of MIME-subtype string.
   UtlString mime_copy(mime);
   mime_copy.toLower();

   // Perform search
   return (MpCodecSubInfo*)mCodecsInfo.find(&mime_copy);
*/
}

void MpCodecFactory::freeAllLoadedLibsAndCodec()
{
   OsSharedLibMgrBase* pShrMgr = OsSharedLibMgr::getOsSharedLibMgr();

   UtlHashBagIterator iter(mCodecsInfo);
   MpCodecSubInfo* pinfo;

   UtlHashBag libLoaded;
   UtlString* libName;

   while ((pinfo = (MpCodecSubInfo*)iter()))
   {  
      if ((!pinfo->getCodecCall()->isStatic()) && 
         (!libLoaded.find(&pinfo->getCodecCall()->getModuleName()))) {
         libLoaded.insert(const_cast<UtlString*>(&pinfo->getCodecCall()->getModuleName()));
      }    
   }

   UtlHashBagIterator iter2(libLoaded);
   while ((libName = (UtlString*)iter2()))
   {
      pShrMgr->unloadSharedLib(libName->data());
   }

   iter.reset();
   while ((pinfo = (MpCodecSubInfo*)iter()))
   {  
      if (!pinfo->getCodecCall()->isStatic()) {
         mCodecsInfo.remove(pinfo);
         delete pinfo;         
      }
   }

   mCodecInfoCacheValid = FALSE;
}

void MpCodecFactory::initializeStaticCodecs()
{
   MpCodecCallInfoV1* codecCallInfo;
   for (codecCallInfo = sStaticCodecsV1; codecCallInfo; codecCallInfo = codecCallInfo->getNext())
   {
      addCodecWrapperV1(codecCallInfo);
   }
}

void MpCodecFactory::freeStaticCodecs()
{
   MpCodecCallInfoV1* tmp = sStaticCodecsV1;
   MpCodecCallInfoV1* next;
   if (tmp) {
      for ( ;tmp; tmp = next)
      {
         next = tmp->getNext();
         delete tmp;
      }
      sStaticCodecsV1 = NULL;
   }
}

/* /////////////////////////////// PRIVATE //////////////////////////////// */

void MpCodecFactory::updateCodecInfoCache() const
{
   // First delete old data.
   delete[] mpCodecInfoCache;

   // Allocate memory for new array.
   mCachedCodecInfoNum = mCodecsInfo.entries();
   mpCodecInfoCache = new const MppCodecInfoV1_1*[mCachedCodecInfoNum];

   // Fill array with data.
   UtlHashBagIterator iter(mCodecsInfo);
   for (unsigned i=0; i<mCachedCodecInfoNum; i++)
   {
      MpCodecSubInfo* pInfo = (MpCodecSubInfo*)iter();
      mpCodecInfoCache[i] = pInfo->getCodecInfo();
   }

   // Cache successfully updated.
   mCodecInfoCacheValid = TRUE;
}

OsStatus MpCodecFactory::addCodecWrapperV1(MpCodecCallInfoV1* wrapper)
{
   MpCodecSubInfo* mpsi;

   const MppCodecInfoV1_1 *pCodecInfo;
   if (wrapper->mPlgGetInfo(&pCodecInfo) != RPLG_SUCCESS)
   {
      return OS_FAILED;
   }

   // If codec need special packing, it should provide GetPacketSamples() method
   if (  (pCodecInfo->framePacking == CODEC_FRAME_PACKING_SPECIAL)
      != (wrapper->mPlgGetPacketSamples != NULL))
   {
      return OS_FAILED;
   }

   mpsi = new MpCodecSubInfo(wrapper, pCodecInfo);
   if (mpsi == NULL)
   {
      return OS_NO_MEMORY;
   }

   sLock.acquire();
   mCodecsInfo.insert(mpsi);
   sLock.release();

   return OS_SUCCESS;
}


/* ============================== FUNCTIONS =============================== */
