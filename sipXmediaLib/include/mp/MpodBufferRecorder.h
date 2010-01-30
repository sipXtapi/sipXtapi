//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MpodBufferRecorder_h_
#define _MpodBufferRecorder_h_

// SYSTEM INCLUDES
//#include <utl/UtlDefs.h>
#include <os/OsStatus.h>
#include <utl/UtlString.h>

// APPLICATION INCLUDES
#include "mp/MpOutputDeviceDriver.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsNotification;
class OsTimer;

/**
*  @brief Container for device specific output driver.
*
*/
class MpodBufferRecorder : public MpOutputDeviceDriver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor.
   explicit
   MpodBufferRecorder(const UtlString& name, MpFrameTime bufferLength);
     /**<
     *  @param name - (in) unique device driver name (e.g. "/dev/dsp", 
     *         "YAMAHA AC-XG WDM Audio", etc.)
     *  @param bufferLength - (in) length of buffer for recorded data (in milliseconds).
     */

     /// Destructor.
   virtual
   ~MpodBufferRecorder();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @copydoc MpOutputDeviceDriver::enableDevice()
   virtual OsStatus enableDevice(unsigned samplesPerFrame, 
                                 unsigned samplesPerSec,
                                 MpFrameTime currentFrameTime,
                                 OsCallback &frameTicker);

     /// @copydoc MpOutputDeviceDriver::disableDevice()
   virtual OsStatus disableDevice();

     /// @copydoc MpOutputDeviceDriver::pushFrame()
   virtual
   OsStatus pushFrame(unsigned int numSamples,
                      const MpAudioSample* samples,
                      MpFrameTime frameTime);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   inline
   const MpAudioSample *getBufferData() const;

   inline
   unsigned getBufferLength() const;

   inline
   unsigned getBufferEnd() const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   MpFrameTime    mBufferLengthMS;
   unsigned       mBufferLength;
   MpAudioSample *mpBuffer;
   unsigned       mBufferEnd;

   OsTimer       *mpTickerTimer;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor (not implemented for this class)
   MpodBufferRecorder(const MpodBufferRecorder& rMpodBufferRecorder);

     /// Assignment operator (not implemented for this class)
   MpodBufferRecorder& operator=(const MpodBufferRecorder& rhs);
};


/* ============================ INLINE METHODS ============================ */

const MpAudioSample *MpodBufferRecorder::getBufferData() const
{
   return mpBuffer;
}

unsigned MpodBufferRecorder::getBufferLength() const
{
   return mBufferLength;
}

unsigned MpodBufferRecorder::getBufferEnd() const
{
   return mBufferEnd;
}

#endif  // _MpodBufferRecorder_h_
