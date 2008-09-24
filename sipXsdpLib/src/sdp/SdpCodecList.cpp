// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
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

#define VERBOSE_CODEC_FACTORY
#undef VERBOSE_CODEC_FACTORY

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
   mCodecCPULimit = SdpCodec::SDP_CODEC_CPU_HIGH ;
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

       codecStringIndex++;
       UtlNameValueTokenizer::getSubField(codecList, codecStringIndex,
                                          ", \n\r\t", &oneCodec);
   }

   return numRejected;
}
#endif/* !_VXWORKS ] */

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
         osPrintf("Using %d codecs: %d %s/%s\n", mCodecs.entries(),
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
    UtlString prevSubmimeType = "none";
    UtlString actualSubmimeType;

    // Find a codec which does not have its payload type set
    // Cheat a little and make the codec writable
    while ((codecWithoutPayloadId = (SdpCodec*) getCodecByType(-1)))
    {
        codecWithoutPayloadId->getEncodingName(actualSubmimeType);

        // only increment payload id for different submime types
        if (prevSubmimeType.compareTo(actualSubmimeType, UtlString::ignoreCase) != 0)
        {
            // Find an unused dynamic payload type id
            while (getCodecByType(unusedDynamicPayloadId))
            {
                unusedDynamicPayloadId++;
            }
        }

        codecWithoutPayloadId->setCodecPayloadFormat(unusedDynamicPayloadId);
        prevSubmimeType = actualSubmimeType;
    }

#ifdef VERBOSE_CODEC_FACTORY /* [ */
    {
        int count, i;
        SdpCodec** codecs;
        UtlString codecData;
        getCodecs(count, codecs);
        for (i=0; i<count; i++) {
            codecs[i]->toString(codecData);
            osPrintf("\n  SDP Codec Factory[%d]:\n %s", i, codecData.data());
        }
    }
#endif /* VERBOSE_CODEC_FACTORY ] */
}

void SdpCodecList::copyPayloadType(SdpCodec& codec)
{
    SdpCodec* codecFound = NULL;
    OsWriteLock lock(mReadWriteMutex);
    UtlDListIterator iterator(mCodecs);
    int newPayloadType;

    while((codecFound = (SdpCodec*) iterator()) != NULL)
    {
        if(codecFound->isSameDefinition(codec))
        {
            newPayloadType = codec.getCodecPayloadFormat();
            codecFound->setCodecPayloadFormat(newPayloadType);
        }
    }
}

void SdpCodecList::copyPayloadTypes(int numCodecs, 
                                       SdpCodec* codecArray[])
{
    int index;
    for(index = 0; index < numCodecs; index++)
    {
        copyPayloadType(*(codecArray[index]));
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

void SdpCodecList::removeCodecByType(const SdpCodec::SdpCodecTypes type)
{
    SdpCodec* codecFound = NULL;
    OsWriteLock lock(mReadWriteMutex);
    UtlDListIterator iterator(mCodecs);

    while((codecFound = (SdpCodec*) iterator()) != NULL)
    {
        if(codecFound->getCodecType() == type)
        {
            mCodecs.remove(codecFound);
        }
    }
}


/* ============================ ACCESSORS ================================= */

const SdpCodec* SdpCodecList::getCodec(SdpCodec::SdpCodecTypes internalCodecId)
{
    UtlInt codecToMatch(internalCodecId);
    OsReadLock lock(mReadWriteMutex);
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

const SdpCodec* SdpCodecList::getCodecByIndex(size_t index)
{
    const SdpCodec* pCodec = NULL ;
    OsReadLock lock(mReadWriteMutex);

    if (index < mCodecs.entries())
    {
        pCodec = (const SdpCodec*) mCodecs.at(index) ;
    }

    return pCodec ;
}

const SdpCodec* SdpCodecList::getCodecByType(int payloadTypeId)
{
    const SdpCodec* codecFound = NULL;

    OsReadLock lock(mReadWriteMutex);
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

    return(codecFound);
}

const SdpCodec* SdpCodecList::getCodec(const char* mimeType, 
                                       const char* mimeSubType,
                                       unsigned sampleRate,
                                       unsigned numChannels)
{
    const SdpCodec* codecFound = NULL;
    UtlString foundMimeType;
    UtlString foundMimeSubType;
    UtlString mimeTypeString(mimeType ? mimeType : "");
    mimeTypeString.toLower();
    UtlString mimeSubTypeString(mimeSubType ? mimeSubType : "");
    mimeSubTypeString.toLower();
    OsReadLock lock(mReadWriteMutex);
    UtlDListIterator iterator(mCodecs);

    while((codecFound = (SdpCodec*) iterator()))
    {
        // If the mime type matches
        codecFound->getMediaType(foundMimeType);
        if(foundMimeType.compareTo(mimeTypeString, UtlString::ignoreCase) == 0)
        {
            // and if the mime subtype matches
            codecFound->getEncodingName(foundMimeSubType);
            if(  (foundMimeSubType.compareTo(mimeSubTypeString, UtlString::ignoreCase) == 0)
              && (sampleRate == -1 || codecFound->getSampleRate() == sampleRate)
              && (numChannels == -1 || codecFound->getNumChannels() == numChannels)
              && (codecFound->getCPUCost() <= mCodecCPULimit))
            {
                // we found a match
                break;
            }
        }
    }

#ifdef TEST_PRINT
    if(codecFound)
    {
        UtlString codecDump;
        codecFound->toString(codecDump);
        osPrintf("SdpCodecList::getCodec found:\n%s",
            codecDump.data());
    }
#endif

    return(codecFound);
}

int SdpCodecList::getCodecCount()
{
    OsReadLock lock(mReadWriteMutex);
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

int SdpCodecList::getCodecCount(const char* mimetype)
{
    OsReadLock lock(mReadWriteMutex);
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

void SdpCodecList::getCodecList(const char* szMimeType, UtlString& list)
{
    const SdpCodec* pCodec = NULL;

    OsReadLock lock(mReadWriteMutex);
    UtlDListIterator iterator(mCodecs);

    while((pCodec = (SdpCodec*) iterator()))
    {
        UtlString mimeType ;
        pCodec->getMediaType(mimeType);
        if (mimeType.compareTo(szMimeType, UtlString::ignoreCase) == 0)
        {
            UtlString name ;
            UtlString videoFormat ;
            pCodec->getEncodingName(name) ;
            if (pCodec->getVideoFormatString(videoFormat) && !videoFormat.isNull())
            {
                name.append("-") ;
                name.append(videoFormat) ;
            }
            name.insert(0, " ") ;
            name.append(" ") ;
            if (list.index(name, 0, UtlString::ignoreCase) == UtlString::UTLSTRING_NOT_FOUND)
            {
                list.append(name) ;
            }
        }            
    }
    list.strip(UtlString::both, ' ') ;
}

void SdpCodecList::applyCodecListOrdering(const UtlString& desiredPriority, 
                                             UtlString& list)
{
    UtlString newList ;
    /*
     * Walk through the priority list and populate based on that order.  If
     * elements are found in the list which were not on the priority list, 
     * include them at the end (in order)
     */

    int index = 0 ;
    UtlString codec ;
    while (UtlNameValueTokenizer::getSubField(desiredPriority, index++," \t\r\n", &codec))
    {
        int pos = list.index(codec, 0, UtlString::ignoreCase) ;
        if (pos != UtlString::UTLSTRING_NOT_FOUND)
        {
            newList.append(codec) ;
            newList.append(" ") ;
            list.remove(pos, codec.length()) ;
        }
    }

    newList.append(list) ;
    list = newList ;   
    list.strip(UtlString::both, ' ') ; 
}

void SdpCodecList::removeKeywordFromCodecList(const UtlString& keyword, UtlString& list) 
{
    UtlString newList ;

    int index = 0 ;
    UtlString codec ;
    while (UtlNameValueTokenizer::getSubField(list, index++," \t\r\n", &codec))
    {
        int pos = codec.index(keyword, 0, UtlString::ignoreCase) ;
        if (pos == UtlString::UTLSTRING_NOT_FOUND)
        {
            newList.append(codec) ;
            newList.append(" ") ;
        }
    }

    list = newList ;   
    list.strip(UtlString::both, ' ') ; 
}

void SdpCodecList::getCodecs(int& numCodecs, 
                             SdpCodec**& codecArray)
{
    const SdpCodec* codecFound = NULL;
    OsReadLock lock(mReadWriteMutex);
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
                             const char* mimeType)
{
    const SdpCodec* codecFound = NULL;
    OsReadLock lock(mReadWriteMutex);
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
                                const char* subMimeType)
{
    const SdpCodec* codecFound = NULL;
    OsReadLock lock(mReadWriteMutex);
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

void SdpCodecList::toString(UtlString& serializedFactory)
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
int SdpCodecList::getCodecCPULimit()
{
   return mCodecCPULimit;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void SdpCodecList::addCodecNoLock(const SdpCodec& newCodec)
{
    mCodecs.insert(new SdpCodec(newCodec));

#ifdef TEST_PRINT
    UtlString codecDump;
    newCodec.toString(codecDump);
    osPrintf("SdpCodecList::addCodec adding:\n%s",
        codecDump.data());
#endif
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
