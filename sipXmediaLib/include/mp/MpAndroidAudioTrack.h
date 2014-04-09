//  
// Copyright (C) 2010-2011 SIPez LLC. 
// Licensed under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MpAndroidAudioTrack_h_
#define _MpAndroidAudioTrack_h_

// SIPX INCLUDES
#include <os/OsStatus.h>
#include <mp/MpAndroidAudioBindingInterface.h>

// SYSTEM INCLUDES

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS

class MpAndroidAudioTrack;

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
class MpAndroidAudioTrack
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    friend MpAndroidAudioTrack* MpAndroidAudioBindingInterface::createAudioTrack(int streamType,
                                                                                 uint32_t sampleRate,
                                                                                 int format,
                                                                                 int channels,
                                                                                 int frameCount,
                                                                                 uint32_t flags,
                                                                                 sipXcallback_t cbf,
                                                                                 void* user,
                                                                                 int notificationFrames) const;

    enum event_type {
        EVENT_MORE_DATA = 0,
        EVENT_UNDERRUN = 1,
        EVENT_LOOP_END = 2,
        EVENT_MARKER = 3,
        EVENT_NEW_POS = 4,
        EVENT_BUFFER_END = 5
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

      /// Destructor
    virtual ~MpAndroidAudioTrack();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators

    virtual void start();

    virtual void stop();

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    virtual int /*status_t*/ initCheck() const; 

    virtual uint32_t getSampleRate();

    virtual int frameSize() const;

    virtual uint32_t frameCount() const;

    virtual uint32_t latency() const;

    virtual void setVolume(float left, float right);

    virtual void dumpAudioTrack(const char* label);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    /// Disallow default constructor
    MpAndroidAudioTrack();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

      /// Copy constructor (not implemented for this class)
    MpAndroidAudioTrack(const MpAndroidAudioTrack& rMpAndroidAudioTrack);

      /// Assignment operator (not implemented for this class)
    MpAndroidAudioTrack& operator=(const MpAndroidAudioTrack& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpAndroidAudioTrack_h_
