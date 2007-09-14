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

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <utl/UtlInit.h> // KEEP THIS ONE THE FIRST INCLUDE

#include <mp/MpCodecFactory.h>
#include <mp/MpPlgEncoderWrap.h>
#include <mp/MpPlgDecoderWrap.h>
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

   MpCodecSubInfo(MpCodecCallInfoV1* pCodecCall,
                  const char* pMimeSubtype,
                  const char** pDefaultFmtpArray,
                  unsigned int defaultFmtpArrayLength)
      : UtlString(pMimeSubtype)
      , mpCodecCall(pCodecCall)
      , mpDefaultFmtpArray(pDefaultFmtpArray)
      , mDefaultFmtpArrayLength(defaultFmtpArrayLength)
   {
      // Store all MIME-subtypes in lower case to allow case insensitive
      // compare.
      toLower();
   }

   ~MpCodecSubInfo()
   {
      if (!mpCodecCall->isStatic())
         delete mpCodecCall;
   }

   const char* getMIMEtype() const
   { return data(); }

   MpCodecCallInfoV1* getCodecCall() const
   { return mpCodecCall; }

   const char** getDefaultFmtpArray() const
   { return mpDefaultFmtpArray; }

   unsigned int getDefaultFmtpArrayLength() const
   { return mDefaultFmtpArrayLength; }

protected:
   MpCodecCallInfoV1* mpCodecCall;
   const char**       mpDefaultFmtpArray;
   unsigned int       mDefaultFmtpArrayLength;
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
   }
   return spInstance;
}

MpCodecFactory::MpCodecFactory(void)
: mIsMimeTypesCacheValid(FALSE)
, mCachedMimeTypesNum(0)
, mpMimeTypesCache(NULL)
{
   initializeStaticCodecs();
}

MpCodecFactory::~MpCodecFactory()
{
   freeAllLoadedLibsAndCodec();

   MpCodecSubInfo* pinfo;

   UtlHashBagIterator iter(mCodecsInfo);
   while ((pinfo = (MpCodecSubInfo*)iter()))
   { 
      if (!pinfo->getCodecCall()->mbStatic) {
         assert(!"Dynamically loaded codecs must be unloaded already");
      }
      delete pinfo;     
   }
   mCodecsInfo.removeAll();

   delete[] mpMimeTypesCache;
}

/* ============================= MANIPULATORS ============================= */

OsStatus MpCodecFactory::createDecoder(const UtlString &mime,
                                       const UtlString &fmtp,
                                       int payloadType,
                                       MpDecoderBase*& rpDecoder)
{
   MpCodecSubInfo* codec = searchByMIME(mime);
 
   if (codec)
   {      
      rpDecoder = new MpPlgDecoderWrapper(payloadType, *codec->getCodecCall(), fmtp);
   }
   else
   {
      OsSysLog::add(FAC_MP, PRI_WARNING, 
                    "MpCodecFactory::createDecoder unknown codec type "
                    "%s, fmtp=%s"
                    "payloadType = %d",
                    mime.data(), fmtp.data(), payloadType);

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
                                       int payloadType,
                                       MpEncoderBase*& rpEncoder)
{
   MpCodecSubInfo* codec = searchByMIME(mime);

   if (codec)
   {      
      rpEncoder = new MpPlgEncoderWrapper(payloadType, *codec->getCodecCall(), fmtp);
   }
   else
   {
      OsSysLog::add(FAC_MP, PRI_WARNING, 
                    "MpCodecFactory::createEncoder unknown codec type "
                    "internalCodecId = (SdpCodec::SdpCodecTypes) %d, "
                    "%s, fmtp=%s"
                    "payloadType = %d",
                    mime.data(), fmtp.data(), payloadType);
      assert(!"Could not find codec of given type!");
      rpEncoder=NULL;
   }

   if (NULL != rpEncoder) 
   {
      return OS_SUCCESS;
   }

   return OS_INVALID_ARGUMENT;
}

MpCodecCallInfoV1* MpCodecFactory::addStaticCodec(MpCodecCallInfoV1* sStaticCode)
{
    sStaticCodecsV1 = (MpCodecCallInfoV1 *)sStaticCode->bound(MpCodecFactory::sStaticCodecsV1);
    return sStaticCodecsV1;
}

void MpCodecFactory::freeSingletonHandle()
{
   sLock.acquire();
   if (spInstance != NULL)
   {
      delete spInstance;
      spInstance = NULL;
   }
   sLock.release();
}

void MpCodecFactory::globalCleanUp()
{
   sLock.acquire();
   MpCodecCallInfoV1* tmp = sStaticCodecsV1;
   MpCodecCallInfoV1* next;
   if (tmp) {
      for ( ;tmp; tmp = next)
      {
         next = tmp->next();
         delete tmp;
      }
      sStaticCodecsV1 = NULL;
   }
   sLock.release();
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
      if ((!pinfo->getCodecCall()->mbStatic) && 
         (!libLoaded.find(&pinfo->getCodecCall()->mModuleName))) {
         libLoaded.insert(&pinfo->getCodecCall()->mModuleName);         
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
      if (!pinfo->getCodecCall()->mbStatic) {
         mCodecsInfo.remove(pinfo);
         delete pinfo;         
      }
   }

   mIsMimeTypesCacheValid = FALSE;
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

      stSignaling = st 
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
         mIsMimeTypesCacheValid = FALSE;
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
   const char* mimeSubtype;
   const char** defaultFtmps;
   unsigned int defaultFtmpsNum;

   // Get codec's MIME-subtype and recommended modes.
   int res = wrapper->mPlgEnum(&mimeSubtype, &defaultFtmpsNum, &defaultFtmps);
   if (res != RPLG_SUCCESS)
   {
      return OS_FAILED;
   }

   mpsi = new MpCodecSubInfo(wrapper, mimeSubtype, defaultFtmps, defaultFtmpsNum);
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

/* ============================== ACCESSORS =============================== */

void MpCodecFactory::getMimeTypes(unsigned& count, const UtlString*& mimeTypes) const
{
   if (!mIsMimeTypesCacheValid)
   {
      updateMimeTypesCache();
   }

   count = mCachedMimeTypesNum;
   mimeTypes = mpMimeTypesCache;
}

OsStatus MpCodecFactory::getCodecFmtps(const UtlString &mime,
                                       unsigned& fmtpCount,
                                       const char**& fmtps) const
{
   MpCodecSubInfo* pInfo = searchByMIME(mime);

   if (!pInfo)
   {
      return OS_NOT_FOUND;
   }

   fmtpCount = pInfo->getDefaultFmtpArrayLength();
   fmtps = pInfo->getDefaultFmtpArray();

   return OS_SUCCESS;
}

/* =============================== INQUIRY ================================ */


/* ////////////////////////////// PROTECTED /////////////////////////////// */


MpCodecSubInfo* MpCodecFactory::searchByMIME(const UtlString& mime) const
{
/*   UtlHashBagIterator iter(mCodecsInfo);
   MpCodecSubInfo* pInfo;

   while ((pInfo = (MpCodecSubInfo*)iter()))
   { 
      if (mime.compareTo(pInfo->getMIMEtype(), UtlString::ignoreCase) == 0)
      {
         return pInfo;
      }
   }
*/
   // Create a lower case copy of MIME-subtype string.
   UtlString mime_copy(mime);
   mime_copy.toLower();

   // Perform search
   return (MpCodecSubInfo*)mCodecsInfo.find(&mime_copy);
}

void MpCodecFactory::updateMimeTypesCache() const
{
   // First delete old data.
   delete[] mpMimeTypesCache;

   // Allocate memory for new array.
   mCachedMimeTypesNum = mCodecsInfo.entries();
   mpMimeTypesCache = new UtlString[mCachedMimeTypesNum];

   // Fill array with data.
   UtlHashBagIterator iter(mCodecsInfo);
   for (unsigned i=0; i<mCachedMimeTypesNum; i++)
   {
      MpCodecSubInfo* pInfo = (MpCodecSubInfo*)iter();
      mpMimeTypesCache[i] = pInfo->getMIMEtype();
   }

   // Cache successfully updated.
   mIsMimeTypesCacheValid = TRUE;
}

/* /////////////////////////////// PRIVATE //////////////////////////////// */


/* ============================== FUNCTIONS =============================== */
