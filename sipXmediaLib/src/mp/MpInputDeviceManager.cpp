//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <mp/MpInputDeviceManager.h>
#include <mp/MpInputDeviceDriver.h>
#include <mp/MpBuf.h>
#include <utl/UtlInt.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// PRIVATE CLASSES
/**
*  @brief Private class container for input device buffer and related info
*/
class MpInputDeviceFrameData
{
public:
    MpBufPtr mFrameBuffer;
    int mFrameTime;
    OsTime mFrameReceivedTime;

private:
      /// Copy constructor (not implemented for this class)
    MpInputDeviceFrameData(const MpInputDeviceFrameData& rMpInputDeviceFrameData);

      /// Assignment operator (not implemented for this class)
    MpInputDeviceFrameData& operator=(const MpInputDeviceFrameData& rhs);
};

/**
*  @brief Private class container for MpInputDeviceDriver pointer and window of buffers
*/
class MpAudioInputConnection : public UtlInt
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor
   MpAudioInputConnection(int deviceId,
                          MpInputDeviceDriver& deviceDriver,
                          int numFramesBuffered) :
   UtlInt(deviceId)
   {
       mpInputDeviceDriver = &deviceDriver;

       mppFrameBufferArray = NULL;
       // TODO: mppFrameBufferArray = new MpInputDeviceFrameData[numFramesBuffered];
   };

     /// Destructor
   virtual
   ~MpAudioInputConnection();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

    OsStatus pushFrame(MpBufPtr& bufferPtr, 
                      int frameTime);

    OsStatus getFrame(MpBufPtr& bufferPtr,
                      int frameTime,
                      int numFramesBefore,
                      int numFramesAfter); 

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    MpInputDeviceDriver* getDeviceDriver() const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    MpInputDeviceFrameData** mppFrameBufferArray;
    MpInputDeviceDriver* mpInputDeviceDriver;

      /// Copy constructor (not implemented for this class)
    MpAudioInputConnection(const MpAudioInputConnection& rMpAudioInputConnection);

      /// Assignment operator (not implemented for this class)
    MpAudioInputConnection& operator=(const MpAudioInputConnection& rhs);
};


//               MpInputDeviceManager implementation

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpInputDeviceManager::MpInputDeviceManager(int defaultSamplesPerFrame, 
                                           int defaultSamplesPerSec,
                                           int defaultNumBufferedFrames)
: mRwMutex(OsRWMutex::Q_PRIORITY)
, mLastDeviceId(0)
, mDefaultSamplesPerFrame(defaultSamplesPerFrame)
, mDefaultSamplesPerSecond(defaultSamplesPerSec)
, mDefaultNumBufferedFrames(defaultNumBufferedFrames)
{
    assert(defaultSamplesPerFrame > 0);
    assert(defaultSamplesPerSec > 0);
    assert(defaultNumBufferedFrames > 0);
}

// Destructor
MpInputDeviceManager::~MpInputDeviceManager()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

