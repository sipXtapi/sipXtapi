//  
// Copyright (C) 2007-2017 SIPez LLC.  All rights reserved.
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
//#include <string.h>

// APPLICATION INCLUDES
#include <sdp/SdpCodecList.h>
#include <sdp/SdpDefaultCodecFactory.h>
#include <utl/UtlNameValueTokenizer.h>
#include <utl/UtlDListIterator.h>
#include <os/OsWriteLock.h>
#include <os/OsReadLock.h>
#include <os/OsSysLog.h>

#define VERBOSE_CODEC_FACTORY
#undef VERBOSE_CODEC_FACTORY

#if defined(VERBOSE_CODEC_FACTORY) || defined(TEST_PRINT)
#include <os/OsSysLog.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SdpCodecList::SdpCodecList(int numCodecs, SdpCodec* codecs[])
: mReadWriteMutex(OsRWMutex::Q_FIFO)
{
   mCodecCPULimit = SdpCodec::SDP_CODEC_CPU_VERY_HIGH ;
   addCodecs(numCodecs, codecs);
}

// Copy constructor
SdpCodecList::SdpCodecList(const SdpCodecList& rSdpCodecFactory)
: mReadWriteMutex(OsRWMutex::Q_FIFO)
{
    *this = rSdpCodecFactory;
}

// Destructor
SdpCodecList::~SdpCodecList()
{
    mCodecs.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SdpCodecList& SdpCodecList::operator=(const SdpCodecList& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

    OsReadLock thatLock(((SdpCodecList&)rhs).mReadWriteMutex);
    OsWriteLock thisLock(mReadWriteMutex);
    mCodecs.destroyAll();
    UtlDListIterator iterator(((SdpCodecList&)rhs).mCodecs);
    const SdpCodec* codecFound;

    while((codecFound = (SdpCodec*) iterator()))
    {
        mCodecs.insert(new SdpCodec(*codecFound));
    }

    mCodecCPULimit = rhs.mCodecCPULimit;
    
   return *this;
}

void SdpCodecList::addCodec(const SdpCodec& newCodec)
{
    OsWriteLock lock(mReadWriteMutex);
    addCodecNoLock(newCodec);
}

void SdpCodecList::addCodecs(int numCodecs, SdpCodec* codecs[])
{
   OsWriteLock lock(mReadWriteMutex);
   for(int index = 0; index < numCodecs; index++)
   {
       addCodecNoLock(*(codecs[index]));
   }
}

#ifndef _VXWORKS /* [ */
//left this in to maintain backward compatibility

//////////////////////////////////////////////////////////////////////////////
//  WHAT?? WHY??
// While correcting compiler warnings I stumbled across something seriously
// stupid in src/net/SdpCodecFactory.cpp.  It is proof positive that too
// many cooks spoil the broth.
//
// This is NONSENSE.  It was left in due to laziness.  There is NOTHING
// platform-dependent in this code.  The original implementation was
// just plain broken, and at some point I fixed it.  Then, someone put
// it back in, rather than fixing the code that called it.
//
// The only reason it is "platform-dependent" is that this is called from
// the main program startup code, which is typically different for each
// APPLICATION; in this case, the phone (VxWorks) and the media server
// (Linus/posix) are the applications that each needed to be modified to
// call the corrected version of this routine.
//
// In fact, this routine was fixed to allow this file to be completely
// independent of the application (or "platform"), as the actual action
// of this code was now controlled by the caller, which is where any
// such variation should be isolated.
//
// Further complicating this whole thing is the fact that softphone needed
// to do something different that did the others to support the free vs.
// paid version, which differed by which codecs were supported (exactly
// the purpose of this entire class).
//
// But, instead of fixing the call to this routine to use the correct
// implementation, the decision was made to RESURRECT THE OLD BROKEN ONE.
//
// Anyway, this should be fixed in the main program of the softphone and
// the mediaserver, and then this bogus method must be deleted.
//////////////////////////////////////////////////////////////////////////////

int SdpCodecList::addCodecs(const UtlString &codecList)
{
   UtlString oneCodec;
   int numRejected = 0;
   int codecStringIndex = 0;
   SdpCodec::SdpCodecTypes codecs[1];
   SdpCodec::SdpCodecTypes internalCodecId;
   UtlNameValueTokenizer::getSubField(codecList,codecStringIndex ,
                                      ", \n\r\t", &oneCodec);
 
   while(!oneCodec.isNull())
   {
       internalCodecId = SdpDefaultCodecFactory::getCodecType(oneCodec.data());
       if (internalCodecId != SdpCodec::SDP_CODEC_UNKNOWN)
       {
           codecs[0] = internalCodecId;
           numRejected += addCodecs(1,codecs);
       }
       else
       {
          numRejected++;
#ifdef TEST_PRINT
          OsSysLog::add(FAC_SDP, PRI_ERROR, "Invalid codec token: %s",
              oneCodec.data());
#endif
       }

       codecStringIndex++;
       UtlNameValueTokenizer::getSubField(codecList, codecStringIndex,
                                          ", \n\r\t", &oneCodec);
   }

   return numRejected;
}

int SdpCodecList::addCodecs(UtlSList& codecNameList)
{
   UtlString* oneCodec = NULL;
   int numRejected = 0;
   SdpCodec::SdpCodecTypes codecs[1];
   SdpCodec::SdpCodecTypes internalCodecId;
   UtlSListIterator iterator(codecNameList);
 
   while((oneCodec = (UtlString*) iterator()))
   {
       internalCodecId = SdpDefaultCodecFactory::getCodecType(oneCodec->data());
       if (internalCodecId != SdpCodec::SDP_CODEC_UNKNOWN)
       {
           codecs[0] = internalCodecId;
           numRejected += addCodecs(1,codecs);
       }
       else
       {
          numRejected++;
#ifdef TEST_PRINT
          OsSysLog::add(FAC_SDP, PRI_ERROR, "Invalid codec token: %s",
              oneCodec->data());
#endif
       }
   }

   return numRejected;
}
#endif/* !_VXWORKS ] */

int SdpCodecList::addCodecs(int numTokens, const char* codecTokens[], int payloadIds[])
{
    UtlString oneToken;
    int numRejected = 0;
    SdpCodec::SdpCodecTypes codecs[1];
    SdpCodec::SdpCodecTypes internalCodecId;

    for(int tokenIndex = 0; tokenIndex < numTokens; tokenIndex++)
    {
        oneToken = codecTokens[tokenIndex];
        oneToken.strip(UtlString::both);

        if(! oneToken.isNull())
        {
            internalCodecId = SdpDefaultCodecFactory::getCodecType(oneToken.data());
            if (internalCodecId != SdpCodec::SDP_CODEC_UNKNOWN)
            {
                codecs[0] = internalCodecId;
                numRejected += addCodecs(1, codecs);

                // Force to be non-const so that we can set the payload ID
                SdpCodec* codecAdded = (SdpCodec*) getCodec(internalCodecId);
                if(codecAdded)
                {
                    codecAdded->setCodecPayloadFormat(payloadIds[tokenIndex]);
                }
            }
            else
            {
                numRejected++;
                OsSysLog::add(FAC_SDP, PRI_ERR, 
                              "SdpCodecList::addCodecs token: %d Invalid codec token: %s",
                              tokenIndex, oneToken.data());
            }
        }
    }

    return(numRejected);
}

int SdpCodecList::addCodecs(int codecCount, SdpCodec::SdpCodecTypes codecTypes[])
{
   int numRejected = 0;
   OsWriteLock lock(mReadWriteMutex);

   for (int loop = 0; loop < codecCount; loop++)
   {
      SdpCodec codecSdp = SdpDefaultCodecFactory::getCodec(codecTypes[loop]);
      if (codecSdp.getCodecType() != SdpCodec::SDP_CODEC_UNKNOWN)
      {
         // Add codec to set, if it is known.
         addCodecNoLock(codecSdp);

#ifdef VERBOSE_CODEC_FACTORY // [
         UtlString codecMediaType;
         UtlString codecEncodingName;
         codecSdp.getMediaType(codecMediaType);
         codecSdp.getEncodingName(codecEncodingName);
         osPrintf("Using %d codecs: %d %s/%s\n", (int)mCodecs.entries(),
                  codecTypes[loop],
                  codecMediaType.data(), codecEncodingName.data());
#endif // VERBOSE_CODEC_FACTORY ]
      } 
      else
      {
         numRejected++;
      }

   }

   return numRejected;
}

void SdpCodecList::bindPayloadTypes()
{
    int unusedDynamicPayloadId = SdpCodec::SDP_CODEC_MAXIMUM_STATIC_CODEC + 1;
    SdpCodec* codecWithoutPayloadId = NULL;

    // Find a codec which does not have its payload type set
    // Cheat a little and make the codec writable
    while ((codecWithoutPayloadId = (SdpCodec*) getCodecByType(-1)))
    {
        // Find an unused dynamic payload type id
        while (getCodecByType(unusedDynamicPayloadId))
        {
            unusedDynamicPayloadId++;
        }

        codecWithoutPayloadId->setCodecPayloadFormat(unusedDynamicPayloadId);
        unusedDynamicPayloadId++;
    }

#ifdef VERBOSE_CODEC_FACTORY /* [ */
    {
        int count, i;
        SdpCodec** codecs;
        UtlString codecData;
        getCodecs(count, codecs);
        for (i=0; i<count; i++) 
        {
            codecs[i]->toString(codecData);
            OsSysLog::add(FAC_SDP, PRI_DEBUG, "SdpCodecList::bind SDP Codec Factory[%d]:\n %s", i, codecData.data());
        }
    }
#endif /* VERBOSE_CODEC_FACTORY ] */
}

int SdpCodecList::unbindPayloadType(int payloadId)
{
    int unbindCount = 0;
    if(payloadId > SdpCodec::SDP_CODEC_MAXIMUM_STATIC_CODEC)
    {
        // Check if we have other codecs with the same payload ID
        SdpCodec* conflictedCodec = NULL;
        while((conflictedCodec = (SdpCodec*) getCodecByType(payloadId, FALSE /* don't lock */)))
        {
            unbindCount++;
            // Unbind this dynamic codec payload ID as it is in conflict
            conflictedCodec->setCodecPayloadFormat(SdpCodec::SDP_CODEC_UNKNOWN);
        }
    }

    return(unbindCount);
}

void SdpCodecList::copyPayloadType(const SdpCodec& codec)
{
    SdpCodec* codecFound = NULL;
    OsWriteLock lock(mReadWriteMutex);
    UtlDListIterator iterator(mCodecs);
    int newPayloadType;

    while((codecFound = (SdpCodec*) iterator()) != NULL)
    {
        if(codecFound->isSameDefinition(codec))
        {
            // This is all supposed to be done in isSameDefinition now
#if 0
            // If this is H.264 we need to have the exact same level_idc or we should not
            // use the same payload ID
            UtlString mimeSubtype;
            codec.getEncodingName(mimeSubtype);
            if(mimeSubtype.compareTo(MIME_SUBTYPE_H264, UtlString::ignoreCase) == 0)
            {
                UtlString codecFoundProfileId;
                UtlString codecProfileId;
                codecFound->getFmtpParameter("profile-level-id", codecFoundProfileId);
                codec.getFmtpParameter("profile-level-id", codecProfileId);
                if(codecFoundProfileId.compareTo(codecProfileId, UtlString::ignoreCase) == 0)
                {
                    newPayloadType = codec.getCodecPayloadFormat();
#ifdef TEST_PRINT
                    UtlString codecFoundString;
                    UtlString codecString;
                    codecFound->toString(codecFoundString);
                    codec.toString(codecString);
                    OsSysLog::add(FAC_SDP, PRI_DEBUG, "SdpCodecList::copyPayloadType payload ID was: %d, now: %d\n codec: \n%s\n codecFound: \n%s",
                        codecFound->getCodecPayloadFormat(), newPayloadType, codecString.data(), codecFoundString.data());
#endif

                    // Unbind any codecs already using this payload ID
                    unbindPayloadType(newPayloadType);
                    codecFound->setCodecPayloadFormat(newPayloadType);
                }
            }
            else
#endif
            {
                newPayloadType = codec.getCodecPayloadFormat();
                // Unbind any codecs already using this payload ID
                unbindPayloadType(newPayloadType);
                codecFound->setCodecPayloadFormat(newPayloadType);
            }
        }
    }
}

void SdpCodecList::copyPayloadTypes(int numCodecs, 
                                    const SdpCodec* codecArray[])
{
    int index;
    for(index = 0; index < numCodecs; index++)
    {
        copyPayloadType(*(codecArray[index]));
    }
}

void SdpCodecList::copyPayloadTypes(const SdpCodecList& codecList)
{
    const SdpCodec* aCodec = NULL;
    UtlDListIterator iterator(codecList.mCodecs);
    while((aCodec = (const SdpCodec*) iterator()))
    {
        copyPayloadType(*aCodec);
    }
}

void SdpCodecList::limitCodecs(const SdpCodecList& includeOnlyCodecList)
{
    // Loop through codecs in this list.
    SdpCodec* codecFound = NULL;
    OsWriteLock lock(mReadWriteMutex);
    UtlDListIterator iterator(mCodecs);

    while((codecFound = (SdpCodec*) iterator()))
    {
        // need to keep the H264 codec only if it has the same fmtp
        // codec set.  So we use exact matching.
        // If an allowed codec
        if(includeOnlyCodecList.containsCodec(*codecFound, TRUE /*exact matching */))
        {
        }

        // If codec is not in the given list, remove it
        else
        {
            mCodecs.destroy(codecFound);
        }
    }
}

void SdpCodecList::clearCodecs(void)
{
    OsWriteLock lock(mReadWriteMutex);
    mCodecs.destroyAll();
}

// Limits the advertised codec by CPU limit level.
void SdpCodecList::setCodecCPULimit(int iLimit)
{
   mCodecCPULimit = iLimit ;
}
     


/* ============================ ACCESSORS ================================= */

const SdpCodec* SdpCodecList::getCodec(SdpCodec::SdpCodecTypes internalCodecId) const
{
    UtlInt codecToMatch(internalCodecId);
    // Cheat to allow this to be const
    OsReadLock lock((OsRWMutex&)mReadWriteMutex);
    const SdpCodec* codecFound = (SdpCodec*) mCodecs.find(&codecToMatch);

#ifdef TEST_PRINT
    if(codecFound)
    {
        UtlString codecDump;
        codecFound->toString(codecDump);
        osPrintf("SdpCodecList::getCodec found:\n%s",
            codecDump.data());
    }
#endif

    // Filter the codec based on CPU limit
    if ((codecFound != NULL) && (codecFound->getCPUCost() > mCodecCPULimit))
    {
        codecFound = NULL ; 
    }

    return(codecFound);
}

const SdpCodec* SdpCodecList::getCodecByType(int payloadTypeId, UtlBoolean shouldLock) const
{
    const SdpCodec* codecFound = NULL;
    if(shouldLock)
    {
        // Cheat to allow this to be const
        ((OsRWMutex&)mReadWriteMutex).acquireRead();
    }

    UtlDListIterator iterator(mCodecs);

    while((codecFound = (SdpCodec*) iterator()))
    {
        // If the format type matches
        if((codecFound->getCodecPayloadFormat() == payloadTypeId) && 
           (codecFound->getCPUCost() <= mCodecCPULimit))
        {
            // we found a match
            break;
        }
    }

    if(shouldLock)
    {
        // Cheat to allow this to be const
        ((OsRWMutex&)mReadWriteMutex).releaseRead();
    }

    return(codecFound);
}

const SdpCodec* SdpCodecList::getCodec(const char* mimeType, 
                                       const char* mimeSubType,
                                       int sampleRate,
                                       int numChannels,
                                       const UtlString &fmtp) const
{
#ifdef TEST_PRINT
    OsSysLog::add(FAC_SDP, PRI_DEBUG,
            "SdpCodecList::getCodec(%s, %s, %d, %d, %s)",
            mimeType, mimeSubType, sampleRate, numChannels, fmtp.data());
#endif
    const SdpCodec* bestCodecFound = NULL;
    const SdpCodec* codecFound = NULL;
    UtlString foundMimeType;
    UtlString foundMimeSubType;
    UtlString foundFmtp;
    UtlString mimeTypeString(mimeType ? mimeType : "");
    mimeTypeString.SDP_MIME_TO_CASE();
    UtlString mimeSubTypeString(mimeSubType ? mimeSubType : "");
    mimeSubTypeString.SDP_MIME_SUBTYPE_TO_CASE();
    // Cheat to allow this to be const
    OsReadLock lock((OsRWMutex&)mReadWriteMutex);
    UtlDListIterator iterator(mCodecs);
    int compares;
    int bestCodecCompares;

    while((codecFound = (SdpCodec*) iterator()))
    {
        // If the mime type matches
        codecFound->getMediaType(foundMimeType);
#ifdef TEST_PRINT
        OsSysLog::add(FAC_SDP, PRI_DEBUG,
                "SdpCodecList::getCodec codecFound: %s", foundMimeType.data());
#endif
        if(foundMimeType.compareTo(mimeTypeString, UtlString::ignoreCase) == 0)
        {
#ifdef TEST_PRINT
            UtlString codecDump;
            codecFound->toString(codecDump);
            OsSysLog::add(FAC_SDP, PRI_DEBUG,
                    "SdpCodecList::getCodec codecFound matches mime type, codecFound:\n%s",
                    codecDump.data());
#endif
            // and if the mime subtype, sample rate, number of channels
            // and fmtp match.
            codecFound->getEncodingName(foundMimeSubType);
            if(  (foundMimeSubType.compareTo(mimeSubTypeString, UtlString::ignoreCase) == 0)
              && (sampleRate == -1 || codecFound->getSampleRate() == sampleRate)
              && (numChannels == -1 || codecFound->getNumChannels() == numChannels)
              && (codecFound->getCPUCost() <= mCodecCPULimit) 
              && codecFound->compareFmtp(fmtp, compares))
            {
#if 1
                if(bestCodecFound)
                {
                    // The prior match is an exact match
                    if(bestCodecCompares == 0)
                    {
                        // Keep the prior match
                    }
                    else
                    {
                        int newBestCompares;
                        bestCodecFound->compareFmtp(*codecFound, newBestCompares);
                        // Looking for the closest match
                        // Either:
                        // A)  fmtp > codecFound > bestCodecFound
                        // or
                        // B)  bestCodecFound < codecFound < fmtp
                        if(
                           (bestCodecCompares > 0 && newBestCompares > 0 && compares > 0) || // case A
                           (bestCodecCompares < 0 && newBestCompares < 0 && compares < 0)
                          )
                        {
                            bestCodecFound = codecFound;
                            bestCodecCompares = compares;
#ifdef TEST_PRINT
                            UtlString codecDump;
                            bestCodecFound->toString(codecDump);
                            OsSysLog::add(FAC_SDP, PRI_DEBUG,
                                          "SdpCodecList::getCodec new best codec:\n%s",
                                          codecDump.data());
#endif
                        }
                    }
                }
                else
                {
                    bestCodecFound = codecFound;
                    bestCodecCompares = compares;
#ifdef TEST_PRINT
                    UtlString codecDump;
                    bestCodecFound->toString(codecDump);
                    OsSysLog::add(FAC_SDP, PRI_DEBUG,
                                  "SdpCodecList::getCodec first best codec:\n%s",
                                   codecDump.data());
#endif
                }


#else
                // TODO:: checking for fmtp match must be made intelligent, e.g. by
                //        defining isCompatible(fmtp) method for SdpCodec. Checking
                //        by string comparison leads to errors when there are two
                //        or more parameters and they're presented in random order.
                codecFound->getSdpFmtpField(foundFmtp);
                if (fmtp.compareTo(foundFmtp, UtlString::ignoreCase) == 0)
                {
                    // we found a match
                    bestCodecFound = codecFound;
                    break;
                }
                else
                {
                    if (foundMimeSubType.compareTo(MIME_SUBTYPE_DTMF_TONES, UtlString::ignoreCase) == 0)
                    {
#ifdef SDP_RFC4733_STRICT_FMTP_CHECK // [
                        // Workaround for RFC4733. Refer to RFC4733 section 7.1.1.
                        // paragraph optional "Optional parameters" and
                        // section 2.4.1 for details.
                        if (  (fmtp.isNull() || fmtp == "0-15")
                           && (foundFmtp.isNull() || foundFmtp == "0-15"))
                        {
                            // we found a match
                            bestCodecFound = codecFound;
                            break;
                        }
#else // SDP_RFC4733_STRICT_FMTP_CHECK ][
                        // Match any fmtp for RFC4733 DTMFs.
                        // There are quite a few implementation which use
                        // different fmtp strings in their SDP and we should be
                        // interoperable with them. Simplest way is to accept
                        // everything and ignore unknown codes later.
                        // Examples of fmtp strings seen in the field:
                        // "0-16" (e.g. Snom phones), "0-11".
                        bestCodecFound = codecFound;
                        break;
#endif // !SDP_RFC4733_STRICT_FMTP_CHECK ]
                    }
                }
#endif
            }
        }
    }

#ifdef TEST_PRINT
    if(bestCodecFound)
    {
        UtlString codecDump;
        bestCodecFound->toString(codecDump);
        OsSysLog::add(FAC_SDP, PRI_DEBUG,
                      "SdpCodecList::getCodec found:\n%s",
                      codecDump.data());
    }
#endif

    return(bestCodecFound);
}

int SdpCodecList::getCodecCount() const
{
    // Cheat to allow this to be const
    OsReadLock lock((OsRWMutex&)mReadWriteMutex);
    SdpCodec* codecFound = NULL;
    
    // Find all codecs, where the CPU cost is tolerable.
    int iCount = 0;    
    UtlDListIterator iterator(mCodecs);
    while((codecFound = (SdpCodec*) iterator()))
    {
        if (codecFound->getCPUCost() <= mCodecCPULimit)
        {
            iCount++;
        }        
    }

    return iCount;
}

int SdpCodecList::getCodecCount(const char* mimetype) const
{
    // Cheat to allow this to be const
    OsReadLock lock((OsRWMutex&)mReadWriteMutex);
    SdpCodec* codecFound = NULL;
    UtlString foundMimeType;
    
    // Find all codecs, where the CPU cost is tolerable.
    int iCount = 0;    
    UtlDListIterator iterator(mCodecs);
    while((codecFound = (SdpCodec*) iterator()))
    {
        codecFound->getMediaType(foundMimeType);
        if (codecFound->getCPUCost() <= mCodecCPULimit && 
                foundMimeType.compareTo(mimetype, UtlString::ignoreCase) == 0)
        {
            iCount++;
        }        
    }

    return iCount;
}


void SdpCodecList::getCodecs(int& numCodecs, 
                             SdpCodec**& codecArray) const
{
    const SdpCodec* codecFound = NULL;
    // Cheat to allow this to be const
    OsReadLock lock((OsRWMutex&)mReadWriteMutex);
    int arrayMaximum = mCodecs.entries();
    codecArray = new SdpCodec*[arrayMaximum];
    UtlDListIterator iterator(mCodecs);
    int index = 0;

    while(index < arrayMaximum &&
          (codecFound = (SdpCodec*) iterator()) != NULL)
    {
        if (codecFound->getCPUCost() <= mCodecCPULimit)
        {
            codecArray[index] = new SdpCodec(*codecFound);
            index++;
        }
    }

    numCodecs = index;
}

void SdpCodecList::getCodecs(int& numCodecs, 
                             SdpCodec**& codecArray,
                             const char* mimeType) const
{
    const SdpCodec* codecFound = NULL;
    // Cheat to allow this to be const
    OsReadLock lock((OsRWMutex&)mReadWriteMutex);
    int arrayMaximum = mCodecs.entries();
    codecArray = new SdpCodec*[arrayMaximum];
    UtlDListIterator iterator(mCodecs);
    UtlString sMimeType;
    int index = 0;

    while(index < arrayMaximum &&
          (codecFound = (SdpCodec*) iterator()) != NULL)
    {
        codecFound->getMediaType(sMimeType);
        if (codecFound->getCPUCost() <= mCodecCPULimit && 
            sMimeType.compareTo(mimeType, UtlString::ignoreCase) == 0)
        {
            codecArray[index] = new SdpCodec(*codecFound);
            index++;
        }
    }

    numCodecs = index;
}

void SdpCodecList::getCodecs(int& numCodecs, 
                             SdpCodec**& codecArray,
                             const char* mimeType,
                             const char* subMimeType) const
{
    const SdpCodec* codecFound = NULL;
    // Cheat to allow this to be const
    OsReadLock lock((OsRWMutex&)mReadWriteMutex);
    int arrayMaximum = mCodecs.entries();
    codecArray = new SdpCodec*[arrayMaximum];
    UtlDListIterator iterator(mCodecs);
    UtlString sMimeType;
    UtlString sSubMimeType;
    int index = 0;

    while(index < arrayMaximum &&
          (codecFound = (SdpCodec*) iterator()) != NULL)
    {
        codecFound->getMediaType(sMimeType);
        codecFound->getEncodingName(sSubMimeType);
        if (    codecFound->getCPUCost() <= mCodecCPULimit && 
                sMimeType.compareTo(mimeType, UtlString::ignoreCase) == 0 && 
                sSubMimeType.compareTo(subMimeType, UtlString::ignoreCase) == 0)
        {
            codecArray[index] = new SdpCodec(*codecFound);
            index++;
        }
    }

    numCodecs = index;
}

void SdpCodecList::freeArray(int arraySize, SdpCodec**& codecArray)
{
    for(int codecIndex = 0; codecIndex < arraySize; codecIndex++)
    {
        delete codecArray[codecIndex];
        codecArray[codecIndex] = NULL;
    }
    delete[] codecArray;
    codecArray = NULL;
}


void SdpCodecList::toString(UtlString& serializedFactory) const
{
    serializedFactory.remove(0);
    const SdpCodec* codecFound = NULL;
    UtlDListIterator iterator(mCodecs);
    int index = 0;

    while((codecFound = (SdpCodec*) iterator()) != NULL)
    {
        UtlString codecString;
        char codecLabel[256];
        sprintf(codecLabel, "Codec[%d] cost=%d\n", index, codecFound->getCPUCost());
        serializedFactory.append(codecLabel);
        codecFound->toString(codecString);
        serializedFactory.append(codecString);
        serializedFactory.append("\n");
        index++;
    }
        
}

// Gets the codec CPU limit level
int SdpCodecList::getCodecCPULimit() const
{
   return mCodecCPULimit;
}


/* ============================ INQUIRY =================================== */

UtlBoolean SdpCodecList::containsCodec(const SdpCodec& codec, UtlBoolean exact) const
{
    SdpCodec* codecFound = NULL;
    OsReadLock lock((OsRWMutex&)mReadWriteMutex);
    UtlDListIterator iterator(mCodecs);

    while((codecFound = (SdpCodec*) iterator()))
    {
        if(codecFound->isSameDefinition(codec))
        {
            if(exact)
            {
                UtlString codecFoundFmtp;
                UtlString codecFmtp;
                codecFound->getSdpFmtpField(codecFoundFmtp);
                codec.getSdpFmtpField(codecFmtp);
                if(codecFoundFmtp.compareTo(codecFmtp, UtlString::ignoreCase) == 0)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }

    return(codecFound != NULL);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void SdpCodecList::addCodecNoLock(const SdpCodec& newCodec)
{
    mCodecs.insert(new SdpCodec(newCodec));

#ifdef TEST_PRINT
    UtlString codecDump;
    newCodec.toString(codecDump);
    OsSysLog::add(FAC_SDP, PRI_DEBUG,
                  "SdpCodecList::addCodec adding:\n%s",
                  codecDump.data());
#endif
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
