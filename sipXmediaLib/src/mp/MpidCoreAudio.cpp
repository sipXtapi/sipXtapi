//
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 SIPfoundry Inc.
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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/MpidCoreAudio.h"
#include "mp/MpInputDeviceManager.h"
#include "os/OsTask.h"

#ifdef RTL_ENABLED // [
#  include "rtl_macro.h"
#else  // RTL_ENABLED ][
#  define RTL_WRITE(x)
#  define RTL_BLOCK(x)
#  define RTL_START(x)
#endif // RTL_ENABLED ]

//#define DUMP_AC
//#define DUMP_CALLS

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */
/* ============================ CREATORS ================================== */
// Default constructor
MpidCoreAudio::MpidCoreAudio(const UtlString& name,
                             MpInputDeviceManager& deviceManager)
: MpInputDeviceDriver(name, deviceManager)
, mDevice(CoreAudioHardware::invalid())
, mAudioFrame(NULL)
{

}

MpidCoreAudio::~MpidCoreAudio()
{
   if (isEnabled())
   {
      assert(!"Calling destructor while MpidCoreAudio is enabled!");
   }
}
/* ============================ MANIPULATORS ============================== */
OsStatus MpidCoreAudio::enableDevice(unsigned samplesPerFrame,
                                     unsigned samplesPerSec,
                                     MpFrameTime currentFrameTime)
{
   OSStatus err;
   OsStatus ret = OS_FAILED;
   if (isEnabled())
   {
      return OS_FAILED;
   }

   mAudioFrame = new MpAudioSample[samplesPerFrame];
   if (mAudioFrame == NULL)
   {
      return OS_LIMIT_REACHED;
   }
   memset(mAudioFrame, 0, samplesPerFrame * sizeof(MpAudioSample));


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
                    "MpidCoreAudio::enableDevice: Couldn't find any component\n");
      goto error_buffer;
   }
   
   //gains access to the services provided by the component
	OpenAComponent(comp, &mInputUnit);  
   
   //AUHAL needs to be initialized before anything is done to it
   err = AudioUnitInitialize(mInputUnit);
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::enableDevice: Couldn't initialize Audio Unit (%d)\n", (int)err);
      goto error_resempler;
   }
   
   //ENABLE IO (INPUT)
	//You must enable the Audio Unit (AUHAL) for input and disable output 
	//BEFORE setting the AUHAL's current device.
	
	//Enable input on the AUHAL
   UInt32 enableIO;
	enableIO = 1;
	err =  AudioUnitSetProperty(mInputUnit,
                               kAudioOutputUnitProperty_EnableIO,
                               kAudioUnitScope_Input,
                               1, // input element
                               &enableIO,
                               sizeof(enableIO));
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::enableDevice: Couldn't enable IO on input side (%d)\n", (int)err);
      goto error_buffer;
   }
	
	//disable Output on the AUHAL
	enableIO = 0;
	err = AudioUnitSetProperty(mInputUnit,
                              kAudioOutputUnitProperty_EnableIO,
                              kAudioUnitScope_Output,
                              0,   //output element
                              &enableIO,
                              sizeof(enableIO));
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::enableDevice: Couldn't disable IO on output side (%d)\n", (int)err);
      goto error_buffer;
   }
   
   // TODO ADD ABILITY TO OPEN ANY DEVICE
   mDevice = CoreAudioHardware::fromDefaultInput();
   if (!mDevice.IsValid())
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::enableDevice: Couldn't find any Output Device\n");
      goto error_buffer;
   }
   
   mInputSampleRate = mDevice.getSuitableSampleRateFor(samplesPerSec);
   
   AudioStreamBasicDescription ds;
   //   AudioStreamBasicDescription dsi;
   
   mDevice.getStreamFormat(ds);
   double prev = ds.mSampleRate;
   ds.mSampleRate = mInputSampleRate;
   mDevice.setStreamFormat(ds);
   mDevice.getStreamFormat(ds);
   
   OsSysLog::add(FAC_MP, PRI_INFO,
                 "MpidCoreAudio::enableDevice: Default samplerate %g, Optimal %g, Actual %g (Driver:%d)\n", prev, mInputSampleRate, ds.mSampleRate, (int)samplesPerSec);
   
   mInputSampleRate = ds.mSampleRate;
   
   // Set AHAUL device
   AudioDeviceID outputDeviceD = mDevice.getAudioDeviceID();
   err = AudioUnitSetProperty(mInputUnit,
                              kAudioOutputUnitProperty_CurrentDevice, 
                              kAudioUnitScope_Global, 
                              0, 
                              &outputDeviceD, 
                              sizeof(outputDeviceD));
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::enableDevice: Couldn't set Output Device to the Audio Unit (%d)\n", (int)err);
      goto error_component;
   }

   
   // Prepare AudioUnit
   if (!setAudioUnitFormat(ds))
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::enableDevice: Couldn't set format to the Audio Unit\n");
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
   
   err = AudioUnitAddPropertyListener(mInputUnit, kAudioUnitProperty_StreamFormat, propertyListener, this);
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::enableDevice: Couldn't set Audio Unit's property callback (%d)\n", (int)err);
      goto error_resempler;
   }   
   
   // Set callback
   AURenderCallbackStruct input;
	memset(&input, 0, sizeof(AURenderCallbackStruct));
	input.inputProc = renderProc;
	input.inputProcRefCon = this;
   err = AudioUnitSetProperty(mInputUnit, kAudioOutputUnitProperty_SetInputCallback, kAudioUnitScope_Global, 0, &input, sizeof(input));
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::enableDevice: Couldn't initialize callback for Audio Unit (%d)\n", (int)err);
      goto error_resempler;
   }
   
   // Ready to start Audio Unit
   err = AudioUnitInitialize(mInputUnit);
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::enableDevice: Couldn't initialize Audio Unit [2nd stage] (%d)\n", (int)err);
      goto error_resempler;
   }
   

   if (!setBuffers())
   {
      goto error_resempler;
   }
   
   err = AudioOutputUnitStart(mInputUnit);
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::enableDevice: Couldn't start Audio Unit (%d)\n", (int)err);
      goto error_set_buffers;
   }
   
   // Audio Unit has been successfuly started
   mIsEnabled = TRUE;
#ifdef DUMP_CALLS   
   printf("MpidCoreAudio::enableDevice ok\n");
#endif
   return OS_SUCCESS;
   
error_set_buffers:
   delete[] (char*)mTempBuffer;
   
error_resempler:
   //Destroy resempler
   AudioConverterDispose(mConverter);
   
error_component:   
   CloseComponent(mInputUnit);
   
error_buffer:
   delete[] mAudioFrame;
   mAudioFrame = NULL;
#ifdef DUMP_CALLS
   printf("MpidCoreAudio::enableDevice %d\n", ret);
#endif
   return ret;
   
}

OsStatus MpidCoreAudio::disableDevice()
{
   //OsStatus ret;
   if (!isEnabled())
   {
      return OS_FAILED;
   }
   
   // If the device is not valid, let the user know it's bad.
   OSStatus err = AudioOutputUnitStop(mInputUnit);
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::disableDevice: Couldn't stop Audio Unit (%d)\n", (int)err);
   }
#ifdef DUMP_CALLS     
   printf("MpidCoreAudio::disableDevice\n");
#endif
   CloseComponent(mInputUnit);
   
   AudioConverterDispose(mConverter);
   
   delete[] (char*)mTempBuffer;
   
   delete[] mAudioFrame;
   mAudioFrame = NULL;
   mIsEnabled = FALSE;
   
   return OS_SUCCESS;
}
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */
UtlBoolean MpidCoreAudio::isDeviceValid()
{
   // There is no way to inquery state before enabling device. So return TRUE.
   return TRUE;
}
/* //////////////////////////// PROTECTED ///////////////////////////////// */
void MpidCoreAudio::pushFrame()
{
   RTL_BLOCK("MpidCoreAudio::pushFrame");
   mpInputDeviceManager->pushFrame(mDeviceId,
                                   mSamplesPerFrame,
                                   mAudioFrame,
                                   mCurrentFrameTime);

   mCurrentFrameTime += getFramePeriod();
}

void MpidCoreAudio::skipFrame()
{
   mCurrentFrameTime += getFramePeriod();
}

bool MpidCoreAudio::setAudioUnitFormat(const AudioStreamBasicDescription& ds)
{
   UInt32 sz = sizeof(ds);
   AudioStreamBasicDescription dsi;
   OSStatus err;
   
   err = (AudioUnitSetProperty(mInputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 1, &ds, sz));
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::setAudioUnitFormat: Couldn't set Input stream info (%d)\n", (int)err);
      return false;
   }    
	err = (AudioUnitGetProperty(mInputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 1, &dsi, &sz));   
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::setAudioUnitFormat: Couldn't get Input stream info (%d)\n", (int)err);
      return false;
   }    
   
   if (!CoreAudioHardware::isEqualAudioStreamBasicDescription(ds, dsi))
   {
      CoreAudioHardware::dumpAudioStreamBasicDescription("   ds.", ds);
      CoreAudioHardware::dumpAudioStreamBasicDescription("in.ds.", dsi);
      
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::setAudioUnitFormat: Couldn't set format to the Input of AU's Device\n");
      return false;
   }
   
	err = (AudioUnitSetProperty(mInputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &ds, sz));   
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::setAudioUnitFormat: Couldn't set Output stream info (%d)\n", (int)err);
      return false;
   }    
	err = (AudioUnitGetProperty(mInputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &dsi, &sz));
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::setAudioUnitFormat: Couldn't get Output stream info (%d)\n", (int)err);
      return false;
   }    
   
   if (!CoreAudioHardware::isEqualAudioStreamBasicDescription(ds, dsi))
   {
      CoreAudioHardware::dumpAudioStreamBasicDescription("    ds.", ds);
      CoreAudioHardware::dumpAudioStreamBasicDescription("out.ds.", dsi);
      
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::setAudioUnitFormat: Couldn't set format to the Output of AU's Device\n");
      return false;
   }
   return true;
}

bool MpidCoreAudio::initResempler(const AudioStreamBasicDescription& ds)
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
   
   OSStatus err = AudioConverterNew(&ds, &source_format, &mConverter);
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::setAudioUnitFormat: Couldn't initialize resempler (%d)\n", (int)err);
      return false;
   }
   return true;
}

bool MpidCoreAudio::setBuffers()
{
   OSStatus err;
   // Set buffer size
   Float32 bSize = mInputSampleRate * (Float32)mSamplesPerFrame / (Float32)mSamplesPerSec; 
   UInt32 bufferSizeFrames = bSize;
   UInt32 propertySize = sizeof(bufferSizeFrames);
   
   err = AudioUnitSetProperty(mInputUnit, kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Global, 1, &bufferSizeFrames, propertySize);
	if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::setBuffers: Couldn't set buffer size (%d)\n", (int)err);
      return false;
   }
   
	err = AudioUnitGetProperty(mInputUnit, kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Global, 1, &bufferSizeFrames, &propertySize);
	if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::setBuffers: Couldn't get buffer size (%d)\n", (int)err);
      return false;
   }
   
   if ((Float32)bufferSizeFrames != bSize)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::setBuffers: Tried to set %g but got %d!\n", bSize, (int)bufferSizeFrames);
      return false;
   }
   
   // Allocate buffer to read data in
   mTempBufferHalfSize = mDevice.getBufferSize();
   mTempBuffer = new char[mTempBufferHalfSize * 2];
   mTempBufferHalf = &(((char*)mTempBuffer)[mTempBufferHalfSize]);
   memset(mTempBuffer, 0, mTempBufferHalfSize * 2);
   mInputFrameSize = bufferSizeFrames;
   OsSysLog::add(FAC_MP, PRI_INFO,
                 "MpidCoreAudio::setBuffers: Set buffer to %d (%d)\n", (int)bufferSizeFrames, (int)mTempBufferHalfSize);   
  
#ifdef DUMP_AC
   printf("MpidCoreAudio::setBuffers = %g (%g) (%d)\n", bSize, mInputSampleRate, (int)mTempBufferHalfSize);
#endif
   
   return true;
}

void MpidCoreAudio::propertyListener(void * inRefCon,
                                     AudioUnit ci,
                                     AudioUnitPropertyID inID,
                                     AudioUnitScope inScope,
                                     AudioUnitElement inElement)
{
   MpidCoreAudio* This = (MpidCoreAudio*)inRefCon;
	if ((inScope == kAudioUnitScope_Input) && (This->mIsEnabled))
	{
#ifdef DUMP_CALLS       
      printf("MpidCoreAudio::propertyListener\n");
#endif  
		OSStatus err = AudioOutputUnitStop(This->mInputUnit);
      if (err)
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
                       "MpidCoreAudio::propertyListener: Couldn't stop AudioUnit (%d)\n", (int)err);
         return;
      }  
      
		UInt32 size = sizeof(AudioStreamBasicDescription);
      AudioStreamBasicDescription dsi;
		err = AudioUnitGetProperty(This->mInputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &dsi, &size);
      if (err)
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
                       "MpidCoreAudio::propertyListener: Couldn't get stream info (%d)\n", (int)err);
         return;
      }      
		err = AudioUnitSetProperty(This->mInputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &dsi, size);
      if (err)
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
                       "MpidCoreAudio::propertyListener: Couldn't set stream info (%d)\n", (int)err);
         return;
      }
      
		This->mInputSampleRate = dsi.mSampleRate;
		
      OsSysLog::add(FAC_MP, PRI_INFO,
                    "MpidCoreAudio::propertyListener: New device sample rate %g Hz\n", This->mInputSampleRate);
      
      delete[] (char*)This->mTempBuffer;
      This->mTempBuffer = NULL;
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
		
		err = AudioOutputUnitStart(This->mInputUnit);
      if (err)
      {
         OsSysLog::add(FAC_MP, PRI_WARNING,
                       "MpidCoreAudio::propertyListener: Couldn't start AudioUnit (%d)\n", (int)err);
         return;
      }  
	}   
}

OSStatus MpidCoreAudio::renderProc(void * inRefCon,
                                   AudioUnitRenderActionFlags* inActionFlags,
                                   const AudioTimeStamp * inTimeStamp,
                                   UInt32 inBusNumber,
                                   UInt32 inNumFrames,
                                   AudioBufferList * ioData)
{
   OSStatus err = noErr;
   MpidCoreAudio* This = (MpidCoreAudio*)inRefCon;
   RTL_BLOCK("MpidCoreAudio::renderProc");
   

#ifdef DUMP_AC
   printf("f=%d ", (int)inNumFrames);
#endif
   
   AudioBufferList lst;
   lst.mNumberBuffers = 1;
   lst.mBuffers[0].mNumberChannels = 1;
   lst.mBuffers[0].mData = This->mTempBufferHalf;
   lst.mBuffers[0].mDataByteSize = This->mTempBufferHalfSize;
   
   //Get the new audio data
	err = AudioUnitRender(This->mInputUnit,
                         inActionFlags,
                         inTimeStamp, 
                         inBusNumber,     
                         inNumFrames, //# of frames requested
                         &lst);// Audio Buffer List to hold data
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::renderProc: Couldn't get samples (%d)\n", (int)err);  
   }
   
   UInt32 frames = This->mSamplesPerFrame;
   AudioBufferList lst2;
   lst2.mNumberBuffers = 1;
   lst2.mBuffers[0].mNumberChannels = 1;
   lst2.mBuffers[0].mData = This->mAudioFrame;
   lst2.mBuffers[0].mDataByteSize = This->mSamplesPerFrame * 2;
   
   err = AudioConverterFillComplexBuffer(This->mConverter, ACproc, This, &frames, &lst2, NULL);
   if (err)
   {
      OsSysLog::add(FAC_MP, PRI_WARNING,
                    "MpidCoreAudio::renderProc: Couldn't convert sample rate! (%d)\n", (int)err);  
   }
   
   This->pushFrame();
   
   
   return noErr;
}

OSStatus MpidCoreAudio::ACproc(AudioConverterRef inAudioConverter, 
                               UInt32 * ioDataPackets,
                               AudioBufferList* ioData,
                               AudioStreamPacketDescription ** inUserData,
                               void *inUData)
{
   MpidCoreAudio* This = (MpidCoreAudio*)inUData;
   
#ifdef DUMP_AC
   printf("@%d(%d) [%d] \n", (int)*ioDataPackets, (int)This->mSamplesPerFrame, (int)ioData->mNumberBuffers);
#endif
   
   unsigned bytesPerFrame = This->mTempBufferHalfSize / This->mInputFrameSize;
#ifdef DUMP_AC   
   printf("Q = %d ",  This->mInputFrameSize);
#endif DUMP_AC
   
   char* ptr = (char*)This->mTempBufferHalf;
   if (*ioDataPackets > This->mInputFrameSize)
   {
      if (*ioDataPackets > 2 *  This->mInputFrameSize)
         *ioDataPackets = 2 *  This->mInputFrameSize;
      
      ptr -= (*ioDataPackets -  This->mInputFrameSize) * bytesPerFrame;
   }
   else if (*ioDataPackets <  This->mInputFrameSize)
   {
      printf("*ioDataPackets = %ld\n", *ioDataPackets);
      assert(!"Only full frame convertion is supported!");
   }
   
   ioData->mBuffers[0].mData = ptr;
   ioData->mBuffers[0].mDataByteSize = *ioDataPackets * bytesPerFrame;
   ioData->mBuffers[0].mNumberChannels = 1;
 
   return noErr;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

