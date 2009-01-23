//
// Copyright (C) 2008 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifndef _MpPlcSilence_h_
#define _MpPlcSilence_h_

#include <mp/MpPlcBase.h>

/**
*  Silence substitution PLC.
*
*  Simplest possible PLC - it just returns silence in place of missed frames.
*/
class MpPlcSilence : public MpPlcBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static const char *name;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// @copydoc MpPlcBase::init()
   OsStatus init(int samplesPerSec);

     /// Destructor
   ~MpPlcSilence();

     /// Reset
   void reset();

     /// @copydoc MpPlcBase::fullReset()
   void fullReset();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @copydoc MpPlcBase::insertToHistory()
   OsStatus insertToHistory(int frameNum,
                            const MpSpeechParams &speechParams,
                            MpAudioSample* pBuf,
                            unsigned inSamplesNum);

     /// @copydoc MpPlcBase::processFrame()
   OsStatus processFrame(MpSpeechParams &speechParams,
                         MpAudioSample* pBuf,
                         unsigned bufferSize,
                         unsigned inSamplesNum,
                         unsigned outSamplesNum,
                         int wantedAdjustment,
                         int &madeAdjustment);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// @copydoc MpPlcBase::getMaxDelayedFramesNum()
   int getMaxDelayedFramesNum() const;

     /// @copydoc MpPlcBase::getMaxFutureFramesNum()
   int getMaxFutureFramesNum() const;

     /// @copydoc MpPlcBase::getAlgorithmicDelay()
   int getAlgorithmicDelay() const;

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

#endif //_MpPlcSilence_h_
