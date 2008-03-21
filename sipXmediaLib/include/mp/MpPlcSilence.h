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

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// @copydoc MpPlcBase::init()
   virtual OsStatus init(int samplesPerSec, int samplesPerFrame);

     /// @copydoc MpPlcBase::~MpPlcSilence()
   virtual ~MpPlcSilence();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @copydoc MpPlcBase::processFrame()
   virtual OsStatus processFrame(int inFrameNum,
                                 int outFrameNum,
                                 const MpAudioSample* in,
                                 MpAudioSample* out,
                                 UtlBoolean* signalModified);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// @copydoc MpPlcBase::getAlgorithmicDelay()
   virtual int getAlgorithmicDelay() const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   int mSamplesPerFrame;

};

#endif //_MpPlcSilence_h_
