//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MP_AUDIO_ABSTRACT_H
#define MP_AUDIO_ABSTRACT_H

#include <os/iostream>
#include <stdio.h>

typedef short AudioSample; 
typedef unsigned char AudioByte; 


//formats we support
#define AUDIO_FORMAT_UNKNOWN 0
#define AUDIO_FORMAT_WAV     1
#define AUDIO_FORMAT_AU      2

class MpAudioAbstract {

public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

    /// Default Constructor. 
    MpAudioAbstract(void);

    /// Copy Constructor 
    MpAudioAbstract(MpAudioAbstract *audio);

    /// Destructor 
    virtual ~MpAudioAbstract();

//@}

public:
///@name Non classfied
//@{

    /// Returns number of samples actually read, 0 on error.
    virtual size_t getSamples(AudioSample *, size_t) = 0;

    /// read length of bytes
    virtual size_t readBytes(AudioByte * buff, size_t length);

    /// get bytes size of the audio file 
    virtual size_t getBytesSize();

    /// get decompression type of the audio file 
    virtual int getDecompressionType(); 

//@}

public:
///@name MpAudioAbstract related operations
//@{

   /// get previous audio
   MpAudioAbstract *getPreviousAudio(void); 

   /// set previous audio to a
   void setPreviousAudio(MpAudioAbstract *a); 

   /// get next audio
   MpAudioAbstract *getNextAudio(void); 

   /// set next audio to a
   void setNextAudio(MpAudioAbstract *a);

//@}

public:
///@name Sample related functions 
//@{

    /// Set the sampling rate to s 
    virtual void setSamplingRate(long s);

    /// Set sampling rate recursively 
    virtual void setSamplingRateRecursive(long s);

    /// TODO: the meaning of this function, get the prefered Sampling rate
    virtual void minMaxSamplingRate(long *min, long *max, long *prefer);

    /// negotiate the sampling rate
    virtual void negotiateSamplingRate(void);

    /// Return the sampling rate
    virtual long getSamplingRate(void);

//@}

public:
///@name Channel related functions 
//@{

    ///Set channel to ch
    virtual void setChannels(int ch);

    ///Set channel recusively
    virtual void setChannelsRecursive(int s);

    ///Get prefered channel 
    virtual void minMaxChannels(int *min, int *max, int *preferred) ;

    /// negotiate channel 
    virtual void negotiateChannels(void);

    /// Return the channels
    virtual int getChannels(void);

    /// Set audio object format
    virtual void setAudioFormat(int type) { mDetectedFormat = type;}

    /// Get audio object format
    virtual int  getAudioFormat() { return mDetectedFormat;}

    /// Return true if file loaded ok.
    bool isOk() {return mbIsOk;}

//@}

/* ============================ PRIVATE ================================== */
private:
    // pointers to MpAudioAbstract itself then 
   MpAudioAbstract *mPrevious; ///< object to get data from
   MpAudioAbstract *mNext;     ///< object pulling data from us

private: // sampling Rate related stuff    
   long mSamplingRate;
   bool mSamplingRateFrozen;

private: // channel related stuff
   long mChannels;
   bool mChannelsFrozen;

   int mDetectedFormat;

protected:
   bool mbIsOk; ///< If file loaded ok.
};

#endif
