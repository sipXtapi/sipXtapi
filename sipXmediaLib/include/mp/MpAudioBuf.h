//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _INCLUDED_MPAUDIOBUF_H /* [ */
#define _INCLUDED_MPAUDIOBUF_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpDataBuf.h"
#include "mp/MpTypes.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

///  Buffer for raw audio data.
/**
*  This is only the header for audio data. It contain some speech-related
*  parameters and pointer to external data (cause it is based on MpDataBuf).
*/
struct MpAudioBuf : public MpDataBuf
{
    friend class MpAudioBufPtr;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    /// Sort of audio data.
    typedef enum {
        MP_SPEECH_UNKNOWN,        ///< is yet undetermined
        MP_SPEECH_SILENT,         ///< found to contain no speech
        MP_SPEECH_COMFORT_NOISE,  ///< to be replaced by comfort noise
        MP_SPEECH_ACTIVE,         ///< found to contain speech
        MP_SPEECH_MUTED,          ///< may contain speech, but must be muted
        MP_SPEECH_TONE            ///< filled with active (not silent) tone data
    } SpeechType;

    static MpBufPool *smpDefaultPool; ///< Default pool for this type of buffer

/* ============================ CREATORS ================================== */
///@name Creators
//@{


//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

    /// Set audio data type.
    void setSpeechType(SpeechType type) {mSpeechType = type;};

    /// Set current number of samples in audio data.
    /**
    * Set data size with #MpAudioSample size kept in mind.
    * 
    * @see MpArrayBuf::setDataSize() for more details
    */
    bool setSamplesNumber(unsigned samplesNum)
    {return mpData->setDataSize(samplesNum*sizeof(MpAudioSample));}

    /// Set attenuation applied at speakerphone speaker.
    void setAttenDb(short attenDb) {mAttenDb = attenDb;}

    /// Set time code for this frame
    void setTimecode(unsigned timecode) {mTimecode=timecode;}

    /// compare two frames of audio to see if they are the same or similar
    /** 
    *  @param tolerance - the allowed difference between the corresponding
    *         samples in the two frames which are considered to still be
    *         the same.
    *  @returns 0, positive or negative value.  Zero means the samples
    *           are similar within the tolerance.
    */
    static int compareSamples(const MpAudioBuf& frame1, 
                              const MpAudioBuf& frame2, 
                              unsigned int tolerance = 0);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Get audio data type.
    SpeechType getSpeechType() const {return mSpeechType;};

    /// Get pointer to audio data.
    const MpAudioSample *getSamplesPtr() const {return (const MpAudioSample*)getDataPtr();}

    /// Get writable pointer to audio data.
    MpAudioSample *getSamplesWritePtr() {return (MpAudioSample*)getDataWritePtr();}

    /// Get current number of samples in audio data.
    unsigned getSamplesNumber() const {return mpData->getDataSize()/sizeof(MpAudioSample);}

    /// Get attenuation applied at speakerphone speaker.
    short getAttenDb() const {return mAttenDb;}

    /// Get time code for this frame
    unsigned getTimecode() const {return mTimecode;}

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

    /// Does buffer contain active voice or silence data
    bool isActiveAudio() const;

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    short      mAttenDb;     ///< attenuation applied at speakerphone speaker
    SpeechType mSpeechType;  ///< if we know, whether buffer contains speech
    unsigned   mTimecode;    ///< time when this data is generated

    /// This is called in place of constructor.
    void init();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /// Disable copy (and other) constructor.
    MpAudioBuf(const MpBuf &);
    /**<
    * This struct will be initialized by init() member.
    */

    /// Disable assignment operator.
    MpAudioBuf &operator=(const MpBuf &);
    /**<
    * Buffers may be copied. But do we need this?
    */
};

///  Smart pointer to MpAudioBuf.
/**
*  You should only use this smart pointer, not #MpAudioBuf* itself.
*  The goal of this smart pointer is to care about reference counter and
*  buffer deallocation.
*/
class MpAudioBufPtr : public MpDataBufPtr {

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

    /// Default constructor - construct invalid pointer.
    MPBUF_DEFAULT_CONSTRUCTOR(MpAudioBuf)

    /// This constructor owns MpBuf object.
    MPBUFDATA_FROM_BASE_CONSTRUCTOR(MpAudioBuf, MP_BUF_AUDIO, MpDataBuf)

    /// Copy object from base type with type check.
    MPBUF_TYPECHECKED_COPY(MpAudioBuf, MP_BUF_AUDIO, MpDataBuf)

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

    /// compare two frames of audio to see if they are the same or similar
    /** 
    *  @param tolerance - the allowed difference between the corresponding
    *         samples in the two frames which are considered to still be
    *         the same.
    *  @returns 0, positive or negative value.  Zero means the samples
    *           are similar within the tolerance.
    */
    int compareSamples(const MpAudioBufPtr& frame2, 
                       unsigned int tolerance = 0) const
    {
        int compareValue = 0;
        if(!isValid())
        {
            compareValue = -1;
        }
        else if(!frame2.isValid())
        {
            compareValue = 1;
        }
        else
        {
            // Need to downcast before dereferencing to avoid implicit
            // construction of MpAudioBuf from MpBuf
            compareValue =
                MpAudioBuf::compareSamples(*((MpAudioBuf*)mpBuffer), 
                                           *((MpAudioBuf*)frame2.mpBuffer), 
                                           tolerance);
        }
        return(compareValue);
    };

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Return pointer to MpAudioBuf.
    MPBUF_MEMBER_ACCESS_OPERATOR(MpAudioBuf)

    /// Return readonly pointer to MpAudioBuf.
    MPBUF_CONST_MEMBER_ACCESS_OPERATOR(MpAudioBuf)

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif /* ] _INCLUDED_MPAUDIOBUF_H */
