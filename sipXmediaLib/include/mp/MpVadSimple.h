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

#ifndef _MpVadSimple_h_
#define _MpVadSimple_h_

#include <mp/MpVadBase.h>

/**
*  Simple Energy-basede VAD.
*
*  This VAD is simplest possible - it just compare mean frame energy with given
*  threshold.
*/
class MpVadSimple : public MpVadBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static const char *name; ///< Name of this VAD algorithm for use in MpVadBase::createVad().

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpVadSimple();

     /// @copydoc MpVadBase::init()
   OsStatus init(int samplesPerSec);

     /// Destructor
   ~MpVadSimple();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @copydoc MpVadBase::processFrame()
   MpSpeechType processFrame(uint32_t packetTimeStamp,
                             const MpAudioSample* pBuf,
                             unsigned inSamplesNum,
                             const MpSpeechParams &speechParams,
                             UtlBoolean calcEnergyOnly = FALSE);

     /// @copydoc MpVadBase::setParam()
   OsStatus setParam(const char* paramName, void* value);

     /// Set threshold energy
   void setMinimumEnergy(int minEnergy);

     /// @copydoc MpVadBase::reset()
   void reset();

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{
     /// @copydoc MpVadBase::getEnergy()
   int getEnergy() const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int mLastFrameSize;
   int mLastEnergy;
   int mSamplesPerSecond;

   int mMinEnergy;
};

#endif // _MpVadSimple_h_
