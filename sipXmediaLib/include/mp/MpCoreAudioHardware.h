//
// Copyright (C) 2009 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifndef _AudioHardware_h_
#define _AudioHardware_h_

// SYSTEM INCLUDES
#include <stdlib.h>
#include <math.h>
#include <CoreAudio/AudioHardware.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreServices/CoreServices.h>
#include <AudioToolbox/DefaultAudioOutput.h>
#include <AudioToolbox/AudioConverter.h>

// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//#define DUMP_DEVICE

/**
 *  @brief Container for CoreAudio hardware device specific.
 */
class CoreAudioHardware
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   
   CoreAudioHardware(AudioDeviceID devid, bool isInput)
   : mDevId(devid)
   , mIsInput(isInput)
   {
#ifdef DUMP_DEVICE
      if (devid != kAudioDeviceUnknown)
      {
         double list[32];
         int i = getAvailableSampleRates(list, 32), j;
         for (j = 0; j < i; j++)
            printf("dev 0x%x: %g\n", (unsigned)mDevId, list[j]);
      }
#endif      
   }
   
   AudioDeviceID getAudioDeviceID() const
   {
      return mDevId;
   }
   
   bool IsInput() const
   {
      return mIsInput;
   }
   
   bool IsValid() const
   {
      return mDevId != kAudioDeviceUnknown;
   }
   
   
   static CoreAudioHardware invalid()
   {
      return CoreAudioHardware(kAudioDeviceUnknown, false);
   }
   
   static CoreAudioHardware fromDefaultOutput()
   {
      AudioDeviceID dev_id = kAudioDeviceUnknown;
      UInt32 parm_size = sizeof(dev_id);
      OSStatus st = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &parm_size, &dev_id);
      if (st != kAudioHardwareNoError)
         printf("AudioDeviceGetProperty->kAudioHardwarePropertyDefaultOutputDevice failed: %d\n", (int)st);
      
      return CoreAudioHardware(dev_id, false);
   }
   
   static CoreAudioHardware fromDefaultInput()
   {
      AudioDeviceID dev_id = kAudioDeviceUnknown;
      UInt32 parm_size = sizeof(dev_id);
      OSStatus st = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice, &parm_size, &dev_id);
      if (st != kAudioHardwareNoError)
         printf("AudioDeviceGetProperty->kAudioHardwarePropertyDefaultInputDevice failed: %d\n", (int)st);
      
      return CoreAudioHardware(dev_id, true);
   }
   
   double getActualSampleRate() const
   {
      Float64 actual = 0;
      UInt32 sz = sizeof(actual);
      OSStatus st = AudioDeviceGetProperty(mDevId, 0, mIsInput, kAudioDevicePropertyActualSampleRate, &sz, &actual);
      if (st != kAudioHardwareNoError)
         printf("[0x%x]AudioDeviceGetProperty->kAudioDevicePropertyActualSampleRate failed: %d\n", (unsigned)mDevId, (int)st);
      
      return actual;
   }
   
   int getLatency() const
   {
      UInt32 actual = 0;
      UInt32 sz = sizeof(actual);
      OSStatus st = AudioDeviceGetProperty(mDevId, 0, mIsInput, kAudioDevicePropertyLatency, &sz, &actual);
      if (st != kAudioHardwareNoError)
         printf("[0x%x]AudioDeviceGetProperty->kAudioDevicePropertyLatency failed: %d\n", (unsigned)mDevId, (int)st);
      
      return actual;
   }
   
   double getSuitableSampleRateFor(double sr)
   {
      const int sz = - getAvailableSampleRates(NULL, 0);
      double buffs[sz];
      int rating;
      getAvailableSampleRates(buffs, sz);
      
      int prev = 20000;
      int idx = 0;
      // This's a simple algorithm for finding more suitable frequncy.
      int j;
      for (j = 0; j < sz; j++)
      {
         double p = buffs[j] / sr;
         rating = (int)(10000 * (p - (int)p));
         rating += (int)p;
         
         if ((rating < prev) && (p >= 1))
         {
            prev = rating;
            idx = j;
         }
         
         printf("[0x%x]RT: %d (%g) %g\n", (unsigned)mDevId, rating, buffs[j], sr);
      }
      
      return buffs[idx];
   }
   
   int getAvailableSampleRates(double *pBuff, const int bufLen) const
   {
      AudioValueRange buffRanges[bufLen];
      UInt32 sz;
      OSStatus st = AudioDeviceGetPropertyInfo(mDevId, 0, mIsInput, kAudioDevicePropertyAvailableNominalSampleRates, &sz, NULL);
      if (st != kAudioHardwareNoError)
      {
         printf("[0x%x]AudioDeviceGetPropertyInfo->kAudioDevicePropertyAvailableNominalSampleRates failed: %d\n", (unsigned)mDevId, (int)st);
         return 0;
      }
      
      if (sz > sizeof(buffRanges))
         return -(int)(sz / sizeof(buffRanges[0]));
      
      st = AudioDeviceGetProperty(mDevId, 0, mIsInput, kAudioDevicePropertyAvailableNominalSampleRates, &sz, &buffRanges);      
      if (st != kAudioHardwareNoError)
      {
         printf("[0x%x]AudioDeviceGetProperty->kAudioDevicePropertyAvailableNominalSampleRates failed: %d\n", (unsigned)mDevId, (int)st);
         return 0;
      }
      
      unsigned i;
      for (i = 0; i < sz / sizeof(buffRanges[0]); i++)
      {
         pBuff[i] = buffRanges[i].mMinimum;
      }
      return i;
   }
   
   bool getStreamFormat(AudioStreamBasicDescription& desc) const
   {
      UInt32 sz = sizeof(desc);
      OSStatus st = AudioDeviceGetProperty(mDevId, 0, mIsInput, kAudioDevicePropertyStreamFormat, &sz, &desc);
      if (st != kAudioHardwareNoError)
      {
         printf("[0x%x]AudioDeviceGetProperty->kAudioDevicePropertyStreamFormat failed: %d\n", (unsigned)mDevId, (int)st);
         return false;
      }
      return true;
   }
   
   bool setStreamFormat(const AudioStreamBasicDescription& desc)
   {
      UInt32 sz = sizeof(desc);
      OSStatus st = AudioDeviceSetProperty(mDevId, NULL, 0, mIsInput, kAudioDevicePropertyStreamFormat, sz, &desc);
      if (st != kAudioHardwareNoError)
      {
         printf("[0x%x]AudioDeviceSetProperty->kAudioDevicePropertyStreamFormat failed: %d\n", (unsigned)mDevId, (int)st);
         return false;
      }
      return true;
   }
   
   static void dumpAudioStreamBasicDescription(const char* header, const AudioStreamBasicDescription& desc)
   {
      printf("%smSampleRate       = %g\n", header, desc.mSampleRate);                                                                                                  
      printf("%smFormatID         = %s\n", header, (char*)&desc.mFormatID);                                                                                                     
      printf("%smFormatFlags      = 0x%08lX\n", header, desc.mFormatFlags);                                                                                            
      printf("%smBytesPerPacket   = %ld\n", header, desc.mBytesPerPacket);                                                                                          
      printf("%smFramesPerPacket  = %ld\n", header, desc.mFramesPerPacket);                                                                                        
      printf("%smChannelsPerFrame = %ld\n", header, desc.mChannelsPerFrame);                                                                                      
      printf("%smBytesPerFrame    = %ld\n", header, desc.mBytesPerFrame);                                                                                            
      printf("%smBitsPerChannel   = %ld\n", header, desc.mBitsPerChannel);   
   }
   
   static bool isEqualAudioStreamBasicDescription(const AudioStreamBasicDescription& desc, const AudioStreamBasicDescription& desc2)
   {
      return (desc.mSampleRate      == desc2.mSampleRate &&
              desc.mFormatID        == desc2.mFormatID &&
              desc.mFormatFlags     == desc2.mFormatFlags &&
              desc.mBytesPerPacket  == desc2.mBytesPerPacket &&
              desc.mFramesPerPacket == desc2.mFramesPerPacket &&
              desc.mChannelsPerFrame == desc2.mChannelsPerFrame &&
              desc.mBytesPerFrame   == desc2.mBytesPerFrame &&
              desc.mBitsPerChannel  == desc2.mBitsPerChannel);
   }
   
   unsigned getBufferSize() const
   {
      UInt32 sr = 0;
      UInt32 sz = sizeof(sr);
      OSStatus st = AudioDeviceGetProperty(mDevId, 0, mIsInput, kAudioDevicePropertyBufferSize, &sz, &sr);
      if (st != kAudioHardwareNoError)
         printf("[0x%x]AudioDeviceGetProperty->kAudioDevicePropertyBufferSize failed: %d\n", (unsigned)mDevId, (int)st);
      
      return sr;
   }
   
   
   bool setBufferSize(unsigned sr)
   {
      UInt32 sz = sizeof(sr);
      OSStatus st = AudioDeviceSetProperty(mDevId, NULL, 0, mIsInput, kAudioDevicePropertyBufferSize, sz, &sr);
      if (st != kAudioHardwareNoError)
      {
         printf("[0x%x]AudioDeviceGetProperty->kAudioDevicePropertyBufferSize failed: %d\n", (unsigned)mDevId, (int)st);
         return false;
      }
      return true;
   }
   
protected:
   AudioDeviceID mDevId;  ///< Device handle
   bool mIsInput;         ///< Whether input or output operations
};


#endif

