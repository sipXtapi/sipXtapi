//  
// Copyright (C) 2010-2013 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MpAndroidAudioRecord_h_
#define _MpAndroidAudioRecord_h_

// SIPX INCLUDES
#include <os/OsStatus.h>
#include <mp/MpAndroidAudioBindingInterface.h>

// SYSTEM INCLUDES

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS

// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Audio input driver for Android OS.
*
*  @see MpInputDeviceDriver
*  @nosubgrouping
*/
class MpAndroidAudioRecord
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    static const int DEFAULT_SAMPLE_RATE = 8000;

    friend MpAndroidAudioRecord* MpAndroidAudioBindingInterface::createAudioRecord() const;

    enum event_type {
        EVENT_MORE_DATA = 0,
        EVENT_UNDERRUN = 1,
        EVENT_MARKER = 2,
        EVENT_NEW_POS = 3
    };

    class Buffer
    {
    public:
        enum {
            MUTE    = 0x00000001
        };
        uint32_t    flags;
        int         channelCount;
        int         format;
        size_t      frameCount;
        size_t      size;
        union {
            void*       raw;
            short*      i16;
            int8_t*     i8;
        };
    };

/* ============================ CREATORS ================================== */
///@name Creators
//@{

protected:
      /// Disallow direct invocation of constructor, must use factory method
    MpAndroidAudioRecord();

public:
      /// Destructor
    virtual ~MpAndroidAudioRecord();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators

    virtual int /*status_t*/ start();

    virtual void stop();

    virtual int /*status_t*/ set(int inputSource,
                                 int sampleRate,
                                 sipXcallback_t audioCallback,
                                 void* user,
                                 int notificationFrames); 

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

      /// Copy constructor (not implemented for this class)
    MpAndroidAudioRecord(const MpAndroidAudioRecord& rMpAndroidAudioRecord);

      /// Assignment operator (not implemented for this class)
    MpAndroidAudioRecord& operator=(const MpAndroidAudioRecord& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpAndroidAudioRecord_h_
