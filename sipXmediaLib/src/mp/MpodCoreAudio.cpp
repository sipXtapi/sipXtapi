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


// SYSTEM INCLUDES
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

// APPLICATION INCLUDES
#include "mp/MpodCoreAudio.h"
#include "mp/MpOutputDeviceManager.h"
#include "os/OsTask.h"
#include "os/OsNotification.h"

#ifdef RTL_ENABLED // [
#  include "rtl_macro.h"
#else  // RTL_ENABLED ][
#  define RTL_BLOCK(x)
#  define RTL_EVENT(x, y)
#endif // RTL_ENABLED ]

//#define DUMP_CALLS

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */
/* ============================ CREATORS ================================== */
// Default constructor
MpodCoreAudio::MpodCoreAudio(const UtlString& name)
: MpOutputDeviceDriver(name)
, mDevice(CoreAudioHardware::invalid())
, mAudioFrame(NULL)
, pNotificator(NULL)
{

}

MpodCoreAudio::~MpodCoreAudio()
{
   if (isEnabled())
   {
      assert(!"Calling destructor while MpodCoreAudio is enabled!");
   }
}
/* ============================ MANIPULATORS ============================== */

OsStatus MpodCoreAudio::enableDevice(unsigned samplesPerFrame,
                                     unsigned samplesPerSec,
                                     MpFrameTime currentFrameTime)
{
   OSStatus err;
   OsStatus ret = OS_FAILED;
   if (isEnabled())
   {
       return OS_FAILED;
   }
   // Get buffer for audio data and fill it with silence
   mAudioFrame = new MpAudioSample[2 * samplesPerFrame];
   if (mAudioFrame == NULL)
   {
      return OS_LIMIT_REACHED;
   }
   
   //Opening CoreAudio device
	Component comp;
	ComponentDescription desc;
	
	//There are several different types of Audio Units.
	//Some audio units serve as Outputs, Mixers, or DSP
	//units. See AUComponent.h for listing
	desc.componentType = kAudioUnitType_Output;
	
	//Every Component has a subType, which will give a clearer picture
	//of what this components function will be.
	desc.componentSubType = kAudioUnitSubType_HALOutput;
	
	//all Audio Units in AUComponent.h must use 
	//"kAudioUnitManufacturer_Apple" as the Manufacturer
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	
	//Finds a component that meets the desc spec's
	comp = FindNextComponent(NULL, &desc);
	if (comp == NULL)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::enableDevice: Couldn't find any component\n");
      goto error_buffer;
   }
	
	//gains access to the services provided by the component
	OpenAComponent(comp, &mOutputUnit);  

   // TODO ADD ABILITY TO OPEN ANY DEVICE
   mDevice = CoreAudioHardware::fromDefaultOutput();
   if (!mDevice.IsValid())
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::enableDevice: Couldn't find any Output Device\n");
      goto error_buffer;
   }
   

   mOutputSampleRate = mDevice.getSuitableSampleRateFor(samplesPerSec);
   
   AudioStreamBasicDescription ds;
   
   mDevice.getStreamFormat(ds);
   double prev = ds.mSampleRate;
   ds.mSampleRate = mOutputSampleRate;
   mDevice.setStreamFormat(ds);
   mDevice.getStreamFormat(ds);
   
   OsSysLog::add(FAC_MP, PRI_INFO,
                 "MpodCoreAudio::enableDevice: Default samplerate %g, Optimal %g, Actual %g (Driver:%d)\n", prev, mOutputSampleRate, ds.mSampleRate, (int)samplesPerSec);
   
   mOutputSampleRate = ds.mSampleRate;
   
   // Set AHAUL device
   AudioDeviceID outputDeviceD = mDevice.getAudioDeviceID();
   err = AudioUnitSetProperty(mOutputUnit,
                              kAudioOutputUnitProperty_CurrentDevice, 
                              kAudioUnitScope_Global, 
                              0, 
                              &outputDeviceD, 
                              sizeof(outputDeviceD));
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::enableDevice: Couldn't set Output Device to the Audio Unit (%d)\n", (int)err);
      goto error_component;
   }
   
   // Prepare AudioUnit
   if (!setAudioUnitFormat(ds))
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::enableDevice: Couldn't set format to the Audio Unit\n");
      goto error_component;
   }
   
   
   // Set some wave header stat information.
   mSamplesPerFrame = samplesPerFrame;
   mSamplesPerSec = samplesPerSec;
   mCurrentFrameTime = currentFrameTime;
   
   
   if (!initResempler(ds))
   {
      goto error_component;
   }
   
   err = AudioUnitAddPropertyListener(mOutputUnit, kAudioUnitProperty_StreamFormat, propertyListener, this);
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::enableDevice: Couldn't set Audio Unit's property callback (%d)\n", (int)err);
      goto error_resempler;
   }   
   
   // Set callback
   AURenderCallbackStruct input;
	memset(&input, 0, sizeof(AURenderCallbackStruct));
	input.inputProc = renderProc;
	input.inputProcRefCon = this;
   err = AudioUnitSetProperty(mOutputUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &input, sizeof(input));
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::enableDevice: Couldn't initialize callback for Audio Unit (%d)\n", (int)err);
      goto error_resempler;
   }
   
   if (!setBuffers())
   {
      goto error_resempler;
   }
   
   // Clear buffer and initialize pointer
   memset(mAudioFrame, 0, 2 * samplesPerFrame * sizeof(MpAudioSample));
   mAudioFrameData = &mAudioFrame[samplesPerFrame];

   err = AudioUnitInitialize(mOutputUnit);
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::enableDevice: Couldn't initialize Audio Unit (%d)\n", (int)err);
      goto error_resempler;
   }
   // Ready to start Audio Unit

   err = AudioOutputUnitStart(mOutputUnit);
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::enableDevice: Couldn't start Audio Unit (%d)\n", (int)err);
      goto error_resempler;
   }
   
   // Audio Unit has been successfuly started
   mIsEnabled = TRUE;
#ifdef DUMP_CALLS  
   printf("MpodCoreAudio::enableDevice ok\n");
#endif
   return OS_SUCCESS;
   
error_resempler:
   //Destroy resempler
   AudioConverterDispose(mConverter);
   
error_component:   
   CloseComponent(mOutputUnit);
   
error_buffer:
   delete[] mAudioFrame;
   mAudioFrame = mAudioFrameData = NULL;
#ifdef DUMP_CALLS  
   printf("MpodCoreAudio::enableDevice %d\n", ret);
#endif
   return ret;
}

OsStatus MpodCoreAudio::disableDevice()
{
   //OsStatus ret;
   if (!isEnabled())
   {
       return OS_FAILED;
   }

   // If the device is not valid, let the user know it's bad.
   OSStatus err = AudioOutputUnitStop(mOutputUnit);
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::disableDevice: Couldn't stop Audio Unit (%d)\n", (int)err);
   }
#ifdef DUMP_CALLS     
   printf("MpodCoreAudio::disableDevice\n");
#endif
   CloseComponent(mOutputUnit);
   
   AudioConverterDispose(mConverter);

   delete[] mAudioFrame;
   mAudioFrame = mAudioFrameData = NULL;
   mIsEnabled = FALSE;

   return OS_SUCCESS;
}

OsStatus MpodCoreAudio::pushFrame(unsigned int numSamples,
                                  const MpAudioSample* samples,
                                  MpFrameTime frameTime)
{
   if (!isEnabled())
      return OS_FAILED;

   // Currently only full frame supported
   assert(numSamples == mSamplesPerFrame);

   RTL_BLOCK("MpodCoreAudio::pushFrame");
   if (samples != NULL)
   {
      memcpy(mAudioFrameData, samples, numSamples * sizeof(MpAudioSample));
   }

   return OS_SUCCESS;
}

OsStatus MpodCoreAudio::setTickerNotification(OsNotification *pFrameTicker)
{
   pNotificator = pFrameTicker;
   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */

bool MpodCoreAudio::setAudioUnitFormat(const AudioStreamBasicDescription& ds)
{
   UInt32 sz = sizeof(ds);
   AudioStreamBasicDescription dsi;
   OSStatus err;
   
   err = (AudioUnitSetProperty(mOutputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &ds, sz));
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::setAudioUnitFormat: Couldn't set Input stream info (%d)\n", (int)err);
      return false;
   }    
	err = (AudioUnitGetProperty(mOutputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &dsi, &sz));   
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::setAudioUnitFormat: Couldn't get Input stream info (%d)\n", (int)err);
      return false;
   }    
   
   if (!CoreAudioHardware::isEqualAudioStreamBasicDescription(ds, dsi))
   {
      CoreAudioHardware::dumpAudioStreamBasicDescription("   ds.", ds);
      CoreAudioHardware::dumpAudioStreamBasicDescription("in.ds.", dsi);
      
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::setAudioUnitFormat: Couldn't set format to the Input of AU's Device\n");
      return false;
   }
   
	err = (AudioUnitSetProperty(mOutputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &ds, sz));   
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::setAudioUnitFormat: Couldn't set Output stream info (%d)\n", (int)err);
      return false;
   }    
	err = (AudioUnitGetProperty(mOutputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &dsi, &sz));
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::setAudioUnitFormat: Couldn't get Output stream info (%d)\n", (int)err);
      return false;
   }    
   
   if (!CoreAudioHardware::isEqualAudioStreamBasicDescription(ds, dsi))
   {
      CoreAudioHardware::dumpAudioStreamBasicDescription("    ds.", ds);
      CoreAudioHardware::dumpAudioStreamBasicDescription("out.ds.", dsi);
      
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::setAudioUnitFormat: Couldn't set format to the Output of AU's Device\n");
      return false;
   }
   return true;
}

bool MpodCoreAudio::initResempler(const AudioStreamBasicDescription& ds)
{
   AudioStreamBasicDescription source_format;
   source_format.mSampleRate = mSamplesPerSec;
	source_format.mFormatID = kAudioFormatLinearPCM;
	source_format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
	source_format.mBytesPerPacket = 2;
	source_format.mFramesPerPacket = 1;
	source_format.mBytesPerFrame = 2;
	source_format.mChannelsPerFrame = 1;
	source_format.mBitsPerChannel = 16; 
   
   OSStatus err = AudioConverterNew(&source_format, &ds, &mConverter);
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::setAudioUnitFormat: Couldn't initialize resempler (%d)\n", (int)err);
      return false;
   }
   return true;
}

bool MpodCoreAudio::setBuffers()
{
   // Set buffer size
   Float32 bSize = mOutputSampleRate * (Float32)mSamplesPerFrame / (Float32)mSamplesPerSec; 
   UInt32 bufferSizeFrames = bSize;
   UInt32 propertySize = sizeof(bufferSizeFrames);
   
#ifdef DUMP_AC
   printf("MpodCoreAudio::setBuffers = %g\n", bSize);
#endif
   
   OSStatus err = AudioUnitSetProperty(mOutputUnit, kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Global, 0, &bufferSizeFrames, propertySize);
	if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::setBuffers: Couldn't set buffer size (%d)\n", (int)err);
      return false;
   }
   
	err = AudioUnitGetProperty(mOutputUnit, kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Global, 0, &bufferSizeFrames, &propertySize);
	if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::setBuffers: Couldn't get buffer size (%d)\n", (int)err);
      return false;
   }
   
   if ((Float32)bufferSizeFrames != bSize)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::setBuffers: Tried to set %g but got %d!\n", bSize, (int)bufferSizeFrames);
      return false;
   }
    
   OsSysLog::add(FAC_MP, PRI_INFO,
                 "MpodCoreAudio::setBuffers: Set buffer to %d\n", (int)bufferSizeFrames);   
   return true;
}


OsStatus MpodCoreAudio::signalForNextFrame()
{
   OsStatus ret = OS_FAILED;
   if (pNotificator != NULL)
      ret = pNotificator->signal(mCurrentFrameTime);
   return ret;
}

void MpodCoreAudio::skipFrame()
{
   mCurrentFrameTime += getFramePeriod();
}


void MpodCoreAudio::propertyListener(void * inRefCon,
                                     AudioUnit ci,
                                     AudioUnitPropertyID inID,
                                     AudioUnitScope inScope,
                                     AudioUnitElement inElement)
{
   MpodCoreAudio* This = (MpodCoreAudio*)inRefCon;
	if ((inScope == kAudioUnitScope_Output) && (This->mIsEnabled))
	{
		OSStatus err = AudioOutputUnitStop(This->mOutputUnit);
      if (err)
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
                       "MpodCoreAudio::propertyListener: Couldn't stop AudioUnit (%d)\n", (int)err);
         return;
      }  
      
		UInt32 size = sizeof(AudioStreamBasicDescription);
      AudioStreamBasicDescription dsi;
		err = AudioUnitGetProperty(This->mOutputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &dsi, &size);
      if (err)
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
                       "MpodCoreAudio::propertyListener: Couldn't get stream info (%d)\n", (int)err);
         return;
      }      
		err = AudioUnitSetProperty(This->mOutputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &dsi, size);
      if (err)
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
                       "MpodCoreAudio::propertyListener: Couldn't set stream info (%d)\n", (int)err);
         return;
      }
      
		This->mOutputSampleRate = dsi.mSampleRate;
		
      OsSysLog::add(FAC_MP, PRI_INFO,
                    "MpodCoreAudio::propertyListener: New device sample rate %g Hz\n", This->mOutputSampleRate);
      
      if (!This->setBuffers())
      {
         return;
      }
#ifdef DUMP_AC     
		printf("New device format: %g Hz\n", dsi.mSampleRate);
#endif
		
		AudioConverterDispose(This->mConverter);
		if (!This->initResempler(dsi))
      {
         return;
      }
		
		err = AudioOutputUnitStart(This->mOutputUnit);
      if (err)
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
                       "MpodCoreAudio::propertyListener: Couldn't start AudioUnit (%d)\n", (int)err);
         return;
      }  
	}   
}

OSStatus MpodCoreAudio::renderProc(void * inRefCon,
                                   AudioUnitRenderActionFlags* inActionFlags,
                                   const AudioTimeStamp * inTimeStamp,
                                   UInt32 inBusNumber,
                                   UInt32 inNumFrames,
                                   AudioBufferList * ioData)
{
   MpodCoreAudio* This = (MpodCoreAudio*)inRefCon;
   RTL_BLOCK("MpodCoreAudio::renderProc");
   
#ifdef DUMP_REND
   unsigned i;
   for (i = 0; i < ioData->mNumberBuffers; i++)
	{
      UInt32 size = ioData->mBuffers[i].mDataByteSize;	
      printf ("$%d ", (int)size);
      
      memset(ioData->mBuffers[i].mData, 0, size);
	}
#endif
   
   UInt32 frames = inNumFrames;
#ifdef DUMP_AC
   printf("f=%d ", frames);
#endif
   OSStatus err = AudioConverterFillComplexBuffer(This->mConverter, ACproc, This, &frames, ioData, NULL);
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpodCoreAudio::renderProc: Couldn't convert sample rate! (%d)\n", (int)err);  
   }
   
   This->signalForNextFrame();
   
   return noErr;
}

OSStatus MpodCoreAudio::ACproc(AudioConverterRef inAudioConverter, 
                               UInt32 * ioDataPackets,
                               AudioBufferList* ioData,
                               AudioStreamPacketDescription ** inUserData,
                               void *inUData)
{
   MpodCoreAudio* This = (MpodCoreAudio*)inUData;
   
#ifdef DUMP_AC
   printf("@%d(%d) [%d] \n", (int)*ioDataPackets, (int)This->mSamplesPerFrame, (int)ioData->mNumberBuffers);
#endif

#ifdef DUMP_REND
   unsigned i;
   for (i = 0; i < ioData->mNumberBuffers; i++)
	{
      UInt32 size = ioData->mBuffers[i].mDataByteSize;	
      printf ("^%d ", (int)size);
	}
#endif

   MpAudioSample* ptr = This->mAudioFrameData;
   if (*ioDataPackets > This->mSamplesPerFrame)
   {
      if (*ioDataPackets > 2 * This->mSamplesPerFrame)
         *ioDataPackets = 2 * This->mSamplesPerFrame;
      
      ptr -= *ioDataPackets - This->mSamplesPerFrame;
   }
   else if (*ioDataPackets < This->mSamplesPerFrame)
   {
      printf("*ioDataPackets = %ld\n", *ioDataPackets);
      assert(!"Only full frame convertion is supported!");
   }
   
   // Prepare buffer for resempling
   ioData->mBuffers[0].mData = ptr;
   ioData->mBuffers[0].mDataByteSize =*ioDataPackets  * 2;
   ioData->mBuffers[0].mNumberChannels = 1;
   
   return noErr;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

