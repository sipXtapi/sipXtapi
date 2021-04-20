//  
// Copyright (C) 2007-2021 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

#ifndef _MpodWinMM_h_
#define _MpodWinMM_h_

// SYSTEM INCLUDES
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <malloc.h>
#include <MMSystem.h>
#include <os/OsMutex.h>
#include <os/OsCallback.h>
#include <utl/UtlSList.h>
#include <utl/UtlVoidPtr.h>

// APPLICATION INCLUDES
#include "mp/MpOutputDeviceDriver.h"

// APPLICATION INCLUDES
#include "mp/MpTypes.h"

// DEFINES
#define DEFAULT_N_OUTPUT_BUFS 32

//#define DONTUSE_SLIST
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x0600)
#  define DONTUSE_SLIST
#endif

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsNotification;
class MpOutputDeviceManager;
class MpMMTimer;
struct IMMNotificationClient;

/**
*  @brief Container for Microsoft Windows device specific output driver.
*
*  The MpOutputDeviceDriver is the class for the Microsoft Windows output media 
*  driver wrapper implementation.  An instance of MpodWinMM is created for
*  every physical or logical output device (e.g. speaker).  
*
*  @note If stuttering occurs from this device driver, one can tweak the 
*        LOW_WAVEBUF_LVL value in MpodWinMM.cpp, but keep in mind increasing 
*        this directly increases latency in this device driver.
*
*  @todo With the current model of operation we should use events as
*        callback mechanism in waveOutOpen, because now we use callback function
*        from whic we set event by hands. This looks like a bit of overhead,
*        while we can use events directly.
*
*  @see MpOutputDeviceDriver
*/
class MpodWinMM : public MpOutputDeviceDriver, OsCallback
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor.
   explicit
   MpodWinMM(const UtlString& name,
             MpOutputDeviceManager* outputManger,
             unsigned nOutputBuffers = DEFAULT_N_OUTPUT_BUFS);
     /**<
     *
     *  @param[in] name - unique windows device driver name 
     *             (e.g. "YAMAHA AC-XG WDM Audio", etc.)
     *  @param[in] outputmanager - pounter to the output manager this 
     *             device will belong to.
     *  @param[in] nOutputBuffers - The number of frame-sized buffers to 
     *             have around for filling with data and passing to windows.
     *  @note \p nOutputBuffers does not directly determine the latency -- 
     *        this doesn't wait until all \p nOutputBuffers are full before
     *        passing on to windows, it just makes them available if we're
     *        passing frames faster than they're being output.
     */

     /// Destructor.
   ~MpodWinMM();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @copydoc MpOutputDeviceDriver::enableDevice(unsigned, unsigned, MpFrameTime)
   OsStatus enableDevice(unsigned samplesPerFrame, 
                         unsigned samplesPerSec,
                         MpFrameTime currentFrameTime,
                         OsCallback &frameTicker);

     /// @copydoc MpOutputDeviceDriver::disableDevice()
   OsStatus disableDevice();

     /// @copydoc MpOutputDeviceDriver::pushFrame(unsigned int, const MpAudioSample*, MpFrameTime)
   OsStatus pushFrame(unsigned int numSamples,
                      const MpAudioSample* samples,
                      MpFrameTime frameTime);

     /// @brief Switch to use MMTimer for Media subsystem ticks instead of output device
   OsStatus switchToMMTimer();

   /// @brief callback used by MpMMTimer when output device is not providing ticks
   virtual OsStatus signal(const intptr_t eventData);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// @brief get the windows name of the default wave output device.
   static UtlString getDefaultDeviceName();

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

     /// @brief Inquire if the windows device is valid
   inline UtlBoolean isDeviceValid();

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

     /// @brief Zero out a wave header, so it is ready to be filled in by us.
   WAVEHDR* initWaveHeader(int n);
     /**<
     *  Initialize all the values in the wave header indicated by \p n.
     *  @param[in] n - The index into mpWaveHeaders indicating 
     *             which wave header to initialize.
     *  @returns a pointer to the wave header that was initialized.
     */

     /// @brief Adds a header/buffer to the empty header list and notifies for a new frame.
   void finalizeProcessedHeader(WAVEHDR* pWaveHdr);
     /**<
     *  This method, called from ThreadMMProc thread, adds a header/buffer
     *  to a list of empty headers so pushFrame() knows there are free buffers
     *  to fill and send to WMM, pumps silence if we are critically low, and
     *  sends a ticker notification to keep processing data.
     *
     *  @param[in] pWaveHdr - Pointer to a wave header that windows is done with.
     */

     /// @brief callback used by wave output multimedia interface
   static void CALLBACK waveOutCallbackStatic(HWAVEOUT hwo, UINT uMsg, 
                                              void* dwInstance,
                                              void* dwParam1, void* dwParam2);
     /**< 
     *  This should only be called by the windows wave output multimedia.
     */

protected:
     /// @brief internal method to handle final writing of audio to the output device
   OsStatus internalPushFrame(unsigned int numSamples, 
                              const MpAudioSample* samples, 
                              MpFrameTime frameTime);

protected:
   MpOutputDeviceManager* mpOutputManger; ///< output manager this device will belong to.
   OsMutex  mEmptyHdrVPtrListsMutex; ///< Mutex to serialize access to vptr and empty header lists.
   int      mWinMMDeviceId;    ///< The underlying windows Device ID (not the 
                               ///< logical Mp device ID)
   HWAVEOUT mDevHandle;        ///< The Microsoft handle for this audio input device
   MpFrameTime mCurFrameTime;  ///< The current frame time for this device.
   unsigned mNumOutBuffers;    ///< The number of buffers to supply to windows
                               ///< for audio processing.
   unsigned mWaveBufSize;      ///< The size, in bytes, of mpWaveBuffer 
                               ///< after allocation.
   WAVEHDR* mpWaveHeaders;     ///< Array of nNumInBuffers wave headers.
   LPSTR*   mpWaveBuffers;     ///< Array of nNumInBuffers wave buffers.
   UtlSList mEmptyHeaderList;  ///< List of pointers to the mpWaveHeaders that 
                               ///< are empty, waiting to be filled.
   UtlSList mUnusedVPtrList;   ///< List of unused UtlVoidPtrs.  Used ones are
                               ///< in mEmptyHeaderList.
   int      mUnderrunLength;   ///< Length of underrun, taking place now (in frames).
   DWORD    mTotSampleCount;   ///< A count of the samples coming in via pushFrame.
   IMMNotificationClient* mWinAudioDeviceChangeCallback; ///< Callback interface for audio
                               ///< device state changes.
   HANDLE   mCallbackThread;   ///< Handle of thread which processes WMM messages.
   HANDLE   mCallbackEvent;    ///< Event to signal that WMM message is available for processing.
   OsAtomicLightBool mExitFlag; ///< Should processing thread finish its execution?
   MpMMTimer* mpTickerTimer;   ///< Optional timer to provide media subsystem ticks when output
                               ///< device is not working properly.

#ifndef DONTUSE_SLIST
     /// Structure used to pass WMM message data to processing thread.
   struct WinAudioDataChain
   {
      SLIST_ENTRY ItemEntry;
      UINT mCbParamMsg;
      WAVEHDR* mCbParamHdr;
   };

   SLIST_HEADER mPoolSignaled; ///< Queue of WMM messages signaled for processing.
   SLIST_HEADER mPoolFree;     ///< Pool of WMM messages.
#else
     /// Structure used to pass WMM message data to processing thread.
   class WinAudioData : public UtlVoidPtr
   {
   public:
      WinAudioData() { setValue(this); }

      UINT mCbParamMsg;
      WAVEHDR* mCbParamHdr;
   };

   UtlSList mListSignaled; ///< Queue of WMM messages signaled for processing.
   UtlSList mListFree;     ///< Pool of WMM messages.
#endif

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor (not implemented for this class)
   MpodWinMM(const MpodWinMM& rMpodWinMM);

     /// Assignment operator (not implemented for this class)
   MpodWinMM& operator=(const MpodWinMM& rhs);

   class MpWinOutputAudioDeviceNotifier;

     /// WMM messages processing thread.
   static DWORD WINAPI ThreadMMProc(LPVOID lpMessage);
     /**<
     *  All WMM messages are forwarded from static callback function to
     *  this thread and are actually processed here. This is done to avoid
     *  deadlocks, when other WMM functions are called from static callback
     *  functions.
     */

   OsStatus resetDevice();
};


/* ============================ INLINE METHODS ============================ */

UtlBoolean MpodWinMM::isDeviceValid()
{
   return mWinMMDeviceId >= 0;
}

#endif  // _MpodWinMM_h_
