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
#include <assert.h>

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
// FORWARD DECLARATIONS
class MpAudioBufPtr;

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

    static MpBufPool *smpDefaultPool; ///< Default pool for this type of buffer

/* ============================ CREATORS ================================== */
///@name Creators
//@{


//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

    /// Set audio data type.
    void setSpeechType(MpSpeechType type) {mParams.mSpeechType = type;};

    /// Set current number of samples in audio data.
    /**
    * Set data size with #MpAudioSample size kept in mind.
    * 
    * @see MpArrayBuf::setDataSize() for more details
    */
    bool setSamplesNumber(unsigned samplesNum)
    {return mpData->setDataSize(samplesNum*sizeof(MpAudioSample));}

    /// Set time code for this frame
    void setTimecode(unsigned timecode) {assert(!"MpAudioBuf::setTimecode() is not implemented!");}

    /// Set amplitude of the data in the buffer.
    void setAmplitude(MpAudioSample amplitude) {mParams.mAmplitude = amplitude;}

    /// Is data in the buffer clipped?
    void setClipping(UtlBoolean clipping) {mParams.mIsClipped = clipping;}

    /// Set normalized frame energy level.
    void setEnergy(int energy) {mParams.mFrameEnergy = energy;}

    /// Set all speech parameters at once
    void setSpeechParams(const MpSpeechParams &params) {mParams = params;}

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Get audio data type.
    MpSpeechType getSpeechType() const {return mParams.mSpeechType;};

    /// Get pointer to audio data.
    const MpAudioSample *getSamplesPtr() const {return (const MpAudioSample*)getDataPtr();}

    /// Get writable pointer to audio data.
    MpAudioSample *getSamplesWritePtr() {return (MpAudioSample*)getDataWritePtr();}

    /// Get current number of samples in audio data.
    unsigned getSamplesNumber() const {return mpData->getDataSize()/sizeof(MpAudioSample);}

    /// Get time code for this frame
    unsigned getTimecode() const {assert(!"MpAudioBuf::getTimecode() is not implemented!"); return 0;}

    /// Get amplitude of the data in the buffer.
    MpAudioSample getAmplitude() const {return mParams.mAmplitude;}

    /// Is data in the buffer clipped?
    UtlBoolean getClipping() const {return mParams.mIsClipped;}

    /// Get normalized frame energy.
    int getEnergy() const {return mParams.mFrameEnergy;}

    /// Get speech parameters as a structure (const version)
    const MpSpeechParams &getSpeechParams() const {return mParams;}

    /// Get speech parameters as a structure
    MpSpeechParams &getSpeechParams() {return mParams;}

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

    /// compare two frames of audio to see if they are the same or similar
    int compareSamples(const MpAudioBufPtr& frame,
                       unsigned int tolerance = 0) const;
    /**<
    *  @param tolerance - the allowed difference between the corresponding
    *         samples in the two frames which are considered to still be
    *         the same.
    *  @returns 0, positive or negative value.  Zero means the samples
    *           are similar within the tolerance.
    */

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    MpSpeechParams mParams;  ///< Parameters of the frame data.

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

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Return pointer to MpAudioBuf.
    MPBUF_MEMBER_ACCESS_OPERATOR(MpAudioBuf)

    /// Return read-only pointer to MpAudioBuf.
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

/* ============================ INLINE METHODS ============================ */

#endif /* ] _INCLUDED_MPAUDIOBUF_H */
