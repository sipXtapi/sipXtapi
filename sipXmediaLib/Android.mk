#
# Copyright (C) 2009-2013 SIPez LLC.  All rights reserved.
#
#
#//////////////////////////////////////////////////////////////////////////
#
# Author: Dan Petrie (dpetrie AT SIPez DOT com)
#
#
# This Makefile is for building sipXmediaLib as a part of Android NDK.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Set up the target identity.
# LOCAL_MODULE/_CLASS are required for local-intermediates-dir to work.
LOCAL_MODULE := libsipXmedia
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
#intermediates := $(call local-intermediates-dir)

# Not immediately needed on Android
FOO_DONT_BUILD := \

fail_to_compile := \
    src/mp/mpau.cpp \
    src/mp/MpAudioFileOpen.cpp \
    src/mp/MpAudioFileUtils.cpp \
    src/mp/MpAudioWaveFileRead.cpp \
    src/mp/MprFromStream.cpp \

ANDROID_DEBUG_SRC_FILES := \
    contrib/android/android_2_0_src/system/core/debuggerd/debuggerd.c

LOCAL_SRC_FILES := \
    src/mp/dft.cpp \
    src/mp/dmaTaskPosix.cpp \
    src/mp/DSPlib.cpp \
    src/mp/dtmflib.cpp \
    src/mp/FilterBank.cpp \
    src/mp/HandsetFilterBank.cpp \
    src/mp/MpAgcBase.cpp \
    src/mp/MpAgcSimple.cpp \
    src/mp/MpAndroidAudioBindingInterface.cpp \
    src/mp/MpAndroidAudioRecord.cpp \
    src/mp/MpAndroidAudioTrack.cpp \
    src/mp/MpArrayBuf.cpp \
    src/mp/MpAudioAbstract.cpp \
    src/mp/MpAudioBuf.cpp \
    src/mp/MpAudioFileDecompress.cpp \
    src/mp/MpAudioOutputConnection.cpp \
    src/mp/MpAudioResource.cpp \
    src/mp/MpAudioUtils.cpp \
    src/mp/MpBridgeAlgLinear.cpp \
    src/mp/MpBridgeAlgSimple.cpp \
    src/mp/MpBuf.cpp \
    src/mp/MpBufPool.cpp \
    src/mp/MpBufferMsg.cpp \
    src/mp/MpCallFlowGraph.cpp \
    src/mp/MpCodec.cpp \
    src/mp/MpCodecFactory.cpp \
    src/mp/MpDataBuf.cpp \
    src/mp/MpDecoderBase.cpp \
    src/mp/MpDecoderPayloadMap.cpp \
    src/mp/MpDspUtils.cpp \
    src/mp/MpDTMFDetector.cpp \
    src/mp/MpEncoderBase.cpp \
    src/mp/MpFlowGraphBase.cpp \
    src/mp/MpFlowGraphMsg.cpp \
    src/mp/mpG711.cpp \
    src/mp/MpidAndroid.cpp \
    src/mp/MpInputDeviceDriver.cpp \
    src/mp/MpInputDeviceManager.cpp \
    src/mp/MpJbeFixed.cpp \
    src/mp/MpJitterBuffer.cpp \
    src/mp/MpJitterBufferEstimation.cpp \
    src/mp/MprHook.cpp \
    src/mp/MpMediaTask.cpp \
    src/mp/MpMediaTaskMsg.cpp \
    src/mp/MpMisc.cpp \
    src/mp/MpMMTimer.cpp \
    src/mp/MpMMTimerPosix.cpp \
    src/mp/MpodAndroid.cpp \
    src/mp/MpodBufferRecorder.cpp \
    src/mp/MpOutputDeviceDriver.cpp \
    src/mp/MpOutputDeviceManager.cpp \
    src/mp/MpPlayer.cpp \
    src/mp/MpPlayerEvent.cpp \
    src/mp/MprBridge.cpp \
    src/mp/MprDecode.cpp \
    src/mp/MprDejitter.cpp \
    src/mp/MprDelay.cpp \
    src/mp/MprEchoSuppress.cpp \
    src/mp/MprEncode.cpp \
    src/mp/MpResampler.cpp \
    src/mp/MpResamplerSpeex.cpp \
    src/mp/MpResource.cpp \
    src/mp/MpResourceFactory.cpp \
    src/mp/MpResourceMsg.cpp \
    src/mp/MpResourceSortAlg.cpp \
    src/mp/MpResourceTopology.cpp \
    src/mp/MpResNotificationMsg.cpp \
    src/mp/MpRtpBuf.cpp \
    src/mp/MpUdpBuf.cpp \
    src/mp/MprAudioFrameBuffer.cpp \
    src/mp/MprFromInputDevice.cpp \
    src/mp/MprFromFile.cpp \
    src/mp/MprFromMic.cpp \
    src/mp/MprFromNet.cpp \
    src/mp/MprMixer.cpp \
    src/mp/MprnDTMFMsg.cpp \
    src/mp/MprnIntMsg.cpp \
    src/mp/MprnProgressMsg.cpp \
    src/mp/MprnRtpStreamActivityMsg.cpp \
    src/mp/MprnStringMsg.cpp \
    src/mp/MprNull.cpp \
    src/mp/MprNullAec.cpp \
    src/mp/MprRecorder.cpp \
    src/mp/MprRtpDispatcher.cpp \
    src/mp/MprRtpDispatcherActiveSsrcs.cpp \
    src/mp/MprRtpDispatcherIpAffinity.cpp \
    src/mp/MprSpeakerSelector.cpp \
    src/mp/MprToSpkr.cpp \
    src/mp/MpSineWaveGeneratorDeviceDriver.cpp \
    src/mp/MpStaticCodecInit.cpp \
    src/mp/MprSpeexEchoCancel.cpp \
    src/mp/MprSpeexPreProcess.cpp \
    src/mp/MprSplitter.cpp \
    src/mp/MprToneGen.cpp \
    src/mp/MprToNet.cpp \
    src/mp/MprToOutputDevice.cpp \
    src/mp/MpRtpInputConnection.cpp \
    src/mp/MpRtpOutputConnection.cpp \
    src/mp/MprVad.cpp \
    src/mp/MprVoiceActivityNotifier.cpp \
    src/mp/MpStreamFeeder.cpp \
    src/mp/MpStreamMsg.cpp \
    src/mp/MpStreamPlayer.cpp \
    src/mp/MpStreamPlaylistPlayer.cpp \
    src/mp/MpStreamQueuePlayer.cpp \
    src/mp/MpSpeakerSelectBase.cpp \
    src/mp/MpPlcBase.cpp \
    src/mp/MpPlcSilence.cpp \
    src/mp/MpPlgStaffV1.cpp \
    src/mp/MpTopologyGraph.cpp \
    src/mp/MpTypes.cpp \
    src/mp/MpVadBase.cpp \
    src/mp/MpVadSimple.cpp \
    src/mp/NetInTask.cpp \
    src/mp/StreamBufferDataSource.cpp \
    src/mp/StreamDataSource.cpp \
    src/mp/StreamDataSourceListener.cpp \
    src/mp/StreamDecoderListener.cpp \
    src/mp/StreamFileDataSource.cpp \
    src/mp/StreamFormatDecoder.cpp \
    src/mp/StreamHttpDataSource.cpp \
    src/mp/StreamQueueingFormatDecoder.cpp \
    src/mp/StreamQueueMsg.cpp \
    src/mp/StreamRAWFormatDecoder.cpp \
    src/mp/StreamWAVFormatDecoder.cpp \
    src/rtcp/BaseClass.cpp \
    src/rtcp/ByeReport.cpp \
    src/rtcp/Message.cpp \
    src/rtcp/MsgQueue.cpp \
    src/rtcp/NetworkChannel.cpp \
    src/rtcp/ReceiverReport.cpp \
    src/rtcp/RTCManager.cpp \
    src/rtcp/RTCPConnection.cpp \
    src/rtcp/RTCPHeader.cpp \
    src/rtcp/RTCPRender.cpp \
    src/rtcp/RTCPSession.cpp \
    src/rtcp/RTCPSource.cpp \
    src/rtcp/RTCPTimer.cpp \
    src/rtcp/RTPHeader.cpp \
    src/rtcp/SenderReport.cpp \
    src/rtcp/SourceDescription.cpp


LOCAL_CFLAGS += -DCODEC_PCMA_PCMU_STATIC=1 \
                -DCODEC_TONES_STATIC=1 \
                -DCODEC_G722_STATIC=1 \
                -DCODEC_SPEEX_STATIC=1 \
                -DCODEC_OPUS_STATIC=1 \
                -DDISABLE_STREAM_PLAYER

LOCAL_CFLAGS += -DHAVE_SPEEX
LOCAL_CFLAGS += -DHAVE_SPEEX_RESAMPLER

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXtackLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \


LOCAL_SHARED_LIBRARIES := libpcre
LOCAL_STATIC_LIBRARIES := libsipXtack libsipXsdp libsipXport $(SIPX_CODEC_LIBS)

LOCAL_LDLIBS += -lstdc++ -ldl

# Android audio related stuff
#SIPX_MEDIA_SHARED_LIBS += libmedia libutils libcutils 
SIPX_MEDIA_STATIC_LIBS += libspeex libspeexdsp
SIPX_MEDIA_LDLIBS += -llog -Wl,--allow-shlib-undefined -L$(SIPX_HOME)/sipXmediaLib/contrib/android/android_2_0_libs -lmedia -lutils -lcutils
# Android Dalvik lib used for catching stack dump in native code signal catcher
SIPX_MEDIA_CFLAGS += -include AndroidConfig.h -DANDROID_2_0
SIPX_MEDIA_C_INCLUDES += \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_2_0_headers/frameworks/base/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_2_0_headers/system/core/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_2_0_headers/system/core/include/arch/linux-arm \
    $(SIPX_HOME)/sipXmediaLib/contrib/libspeex/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android

LOCAL_SHARED_LIBRARIES += $(SIPX_MEDIA_SHARED_LIBS)
LOCAL_STATIC_LIBRARIES += $(SIPX_MEDIA_STATIC_LIBS)
LOCAL_LDLIBS += $(SIPX_MEDIA_LDLIBS)
LOCAL_CFLAGS += $(SIPX_MEDIA_CFLAGS)
LOCAL_C_INCLUDES += $(SIPX_MEDIA_C_INCLUDES)

#LOCAL_CFLAGS += -DEXTERNAL_VAD
#LOCAL_CFLAGS += -DEXTERNAL_PLC
#LOCAL_CFLAGS += -DEXTERNAL_JB_ESTIMATION
#LOCAL_CFLAGS += -DEXTERNAL_AGC
#LOCAL_CFLAGS += -DEXTERNAL_SS

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)

# =======================

# Shared lib for audio drivers on Android 2.0-2.2
include $(CLEAR_VARS)

# Set up the target identity.
LOCAL_MODULE := libsipXandroid2_0

LOCAL_SRC_FILES := \
    src/mp/MpAndroidX_XAudioBinding.cpp \
    src/mp/MpAndroidX_XAudioRecord.cpp \
    src/mp/MpAndroidX_XAudioTrack.cpp \

LOCAL_LDLIBS += -lstdc++ -ldl

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \

LOCAL_SHARED_LIBRARIES += $(SIPX_MEDIA_SHARED_LIBS) libsipXtapi
LOCAL_LDLIBS += $(SIPX_MEDIA_LDLIBS)
LOCAL_CFLAGS += $(SIPX_MEDIA_CFLAGS)
LOCAL_C_INCLUDES += $(SIPX_MEDIA_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)

# =======================

# Shared lib for audio drivers on Android 4.2.1+
include $(CLEAR_VARS)

# Set up the target identity.
LOCAL_MODULE := libsipXandroid4_2_1

LOCAL_SRC_FILES := \
    src/mp/MpAndroidX_XAudioBinding.cpp \
    src/mp/MpAndroidX_XAudioRecord.cpp \
    src/mp/MpAndroidX_XAudioTrack.cpp \

LOCAL_LDLIBS += -lstdc++ -ldl

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \

LOCAL_SHARED_LIBRARIES += libsipXtapi
LOCAL_LDLIBS += -llog -Wl,--allow-shlib-undefined -L$(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_2_1_libs -lmedia -lutils -lcutils
LOCAL_CFLAGS += -include AndroidConfig.h -DANDROID_4_2_1
LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_2_1_headers/frameworks/base/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_2_1_headers/frameworks/native/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_2_1_headers/frameworks/av/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_2_1_headers/system/core/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_2_1_headers/build/core/combo/include/arch/linux-arm \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_2_1_headers/hardware/libhardware/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_2_1_headers \
    $(SIPX_HOME)/sipXmediaLib/contrib/libspeex/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android


include $(BUILD_SHARED_LIBRARY)

# =======================

# Shared lib for audio drivers on Android 4.1.1+
include $(CLEAR_VARS)

# Set up the target identity.
LOCAL_MODULE := libsipXandroid4_1_1

LOCAL_SRC_FILES := \
    src/mp/MpAndroidX_XAudioBinding.cpp \
    src/mp/MpAndroidX_XAudioRecord.cpp \
    src/mp/MpAndroidX_XAudioTrack.cpp \

LOCAL_LDLIBS += -lstdc++ -ldl

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \

LOCAL_SHARED_LIBRARIES += libsipXtapi
LOCAL_LDLIBS += -llog -Wl,--allow-shlib-undefined -L$(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_1_1_libs -lmedia -lutils -lcutils
LOCAL_CFLAGS += -include AndroidConfig.h -DANDROID_4_1_1
LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_1_1_headers/frameworks/base/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_1_1_headers/frameworks/native/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_1_1_headers/frameworks/av/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_1_1_headers/system/core/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_1_1_headers/system/core/include/arch/linux-arm \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_1_1_headers/hardware/libhardware/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_1_1_headers \
    $(SIPX_HOME)/sipXmediaLib/contrib/libspeex/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android


include $(BUILD_SHARED_LIBRARY)

# =======================

# Shared lib for audio drivers on Android 4.0.1+
include $(CLEAR_VARS)

# Set up the target identity.
LOCAL_MODULE := libsipXandroid4_0_1

LOCAL_SRC_FILES := \
    src/mp/MpAndroidX_XAudioBinding.cpp \
    src/mp/MpAndroidX_XAudioRecord.cpp \
    src/mp/MpAndroidX_XAudioTrack.cpp \

LOCAL_LDLIBS += -lstdc++ -ldl

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \

LOCAL_SHARED_LIBRARIES += libsipXtapi
LOCAL_LDLIBS += -llog -Wl,--allow-shlib-undefined -L$(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_0_1_libs -lmedia -lutils -lcutils
LOCAL_CFLAGS += -include AndroidConfig.h -DANDROID_4_0_1
LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_0_1_headers/frameworks/base/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_0_1_headers/system/core/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_0_1_headers/system/core/include/arch/linux-arm \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_0_1_headers/hardware/libhardware/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_4_0_1_headers \
    $(SIPX_HOME)/sipXmediaLib/contrib/libspeex/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android


include $(BUILD_SHARED_LIBRARY)

# =======================

# Shared lib for audio drivers on Android 2.3_4+
include $(CLEAR_VARS)

# Set up the target identity.
LOCAL_MODULE := libsipXandroid2_3_4

LOCAL_SRC_FILES := \
    src/mp/MpAndroidX_XAudioBinding.cpp \
    src/mp/MpAndroidX_XAudioRecord.cpp \
    src/mp/MpAndroidX_XAudioTrack.cpp \

LOCAL_LDLIBS += -lstdc++ -ldl

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \

LOCAL_SHARED_LIBRARIES += libsipXtapi
LOCAL_LDLIBS += -llog -Wl,--allow-shlib-undefined -L$(SIPX_HOME)/sipXmediaLib/contrib/android/android_2_3_4_libs -lmedia -lutils -lcutils
LOCAL_CFLAGS += -include AndroidConfig.h -DANDROID_2_3_4
LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_2_3_4_headers/frameworks/base/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_2_3_4_headers/system/core/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_2_3_4_headers/system/core/include/arch/linux-arm \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_2_3_4_headers \
    $(SIPX_HOME)/sipXmediaLib/contrib/libspeex/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android


include $(BUILD_SHARED_LIBRARY)

# =======================

# Shared lib for audio drivers on Android 2.3+
include $(CLEAR_VARS)

# Set up the target identity.
LOCAL_MODULE := libsipXandroid2_3

LOCAL_SRC_FILES := \
    src/mp/MpAndroidX_XAudioBinding.cpp \
    src/mp/MpAndroidX_XAudioRecord.cpp \
    src/mp/MpAndroidX_XAudioTrack.cpp \

LOCAL_LDLIBS += -lstdc++ -ldl

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \

LOCAL_SHARED_LIBRARIES += libsipXtapi
LOCAL_LDLIBS += -llog -Wl,--allow-shlib-undefined -L$(SIPX_HOME)/sipXmediaLib/contrib/android/android_2_3_libs -lmedia -lutils -lcutils
LOCAL_CFLAGS += -include AndroidConfig.h -DANDROID_2_3
LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_2_3_headers/frameworks/base/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_2_3_headers/system/core/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_2_3_headers/system/core/include/arch/linux-arm \
    $(SIPX_HOME)/sipXmediaLib/contrib/android/android_2_3_headers \
    $(SIPX_HOME)/sipXmediaLib/contrib/libspeex/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android


include $(BUILD_SHARED_LIBRARY)

# =======================

# Unit test framework library

include $(CLEAR_VARS)

# Set up the target identity.
LOCAL_MODULE := sipxmediaunit

# Need to be re-written to NOT use exceptions
fails_to_compile_use_exceptions := \
    src/test/mp/MpInputDeviceManagerTest.cpp \
    src/test/mp/MpInputOutputFrameworkTest.cpp \
    src/test/mp/MprDelayTest.cpp \

# Has Oss sys/soundcard.h dependencies
fails_to_compile_uses_oss := \
    src/test/mp/MpWBInputOutputDeviceTest.cpp \

# MprFromFIle does not compile yet, so we cannot run this test yet
fails_to_compile_istream := \
    src/test/mp/MprFromFileTest.cpp \

# Not immediately needed
notNeededSource := \


LOCAL_SRC_FILES := \
    src/test/mp/MpAudioBufTest.cpp \
    src/test/mp/MpBufTest.cpp \
    src/test/mp/MpCodecsPerformanceTest.cpp \
    src/test/mp/MpDspUtilsTest.cpp \
    src/test/mp/MpFlowGraphTest.cpp \
    src/test/mp/MpGenericResourceTest.cpp \
    src/test/mp/MpInputDeviceDriverTest.cpp \
    src/test/mp/MpMMTimerTest.cpp \
    src/test/mp/MpMediaTaskTest.cpp \
    src/test/mp/MpOutputDriverTest.cpp \
    src/test/mp/MpOutputFrameworkTest.cpp \
    src/test/mp/MpOutputManagerTest.cpp \
    src/test/mp/MpResourceTest.cpp \
    src/test/mp/MpResourceTopologyTest.cpp \
    src/test/mp/MpTestResource.cpp \
    src/test/mp/MprBridgeTest.cpp \
    src/test/mp/MprBridgeTestWB.cpp \
    src/test/mp/MprFromMicTest.cpp \
    src/test/mp/MprMixerTest.cpp \
    src/test/mp/MprRecorderTest.cpp \
    src/test/mp/MprSpeakerSelectorTest.cpp \
    src/test/mp/MprSplitterTest.cpp \
    src/test/mp/MprToSpkrTest.cpp \
    src/test/mp/MprToneGenTest.cpp \


LOCAL_CFLAGS += -DDISABLE_STREAM_PLAYER

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXportLib/src/test \
    $(SIPX_HOME)/sipXportLib/src/test/sipxportunit \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXtackLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \
    $(SIPX_HOME)/sipXmediaLib/src/test

LOCAL_SHARED_LIBRARIES := libpcre 
LOCAL_STATIC_LIBRARIES := libsipxUnit libsipXsdp libsipXtack libsipXmedia libsipXport $(SIPX_CODEC_LIBS)

LOCAL_LDLIBS += -lstdc++ -ldl

# Add sipXmediaLib dependencies
LOCAL_SHARED_LIBRARIES += $(SIPX_MEDIA_SHARED_LIBS)
LOCAL_STATIC_LIBRARIES += $(SIPX_MEDIA_STATIC_LIBS)
LOCAL_LDLIBS += $(SIPX_MEDIA_LDLIBS)
LOCAL_CFLAGS += $(SIPX_MEDIA_CFLAGS)
LOCAL_C_INCLUDES += $(SIPX_MEDIA_C_INCLUDES)

include $(BUILD_EXECUTABLE)


# =======================

# Unit sandbox to test a single test in the test framework library

include $(CLEAR_VARS)

# Set up the target identity.
LOCAL_MODULE := mediasandbox

LOCAL_SRC_FILES := \
    src/test/mp/MpInputOutputFrameworkTest.cpp
#    src/test/mp/MpOutputDriverTest.cpp \
#    src/test/mp/MpOutputFrameworkTest.cpp
#    src/test/mp/MpInputDeviceDriverTest.cpp


#    src/test/mp/MpMMTimerTest.cpp \

LOCAL_CFLAGS += -DDISABLE_STREAM_PLAYER

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXportLib/src/test \
    $(SIPX_HOME)/sipXportLib/src/test/sipxportunit \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXtackLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \
    $(SIPX_HOME)/sipXmediaLib/src/test

LOCAL_SHARED_LIBRARIES := libpcre 
LOCAL_STATIC_LIBRARIES := libsipxUnit libsipXsdp libsipXtack libsipXmedia libsipXport $(SIPX_CODEC_LIBS)
LOCAL_LDLIBS += -lstdc++ -ldl

# Add sipXmediaLib dependencies
LOCAL_SHARED_LIBRARIES += $(SIPX_MEDIA_SHARED_LIBS)
LOCAL_STATIC_LIBRARIES += $(SIPX_MEDIA_STATIC_LIBS)
LOCAL_LDLIBS += $(SIPX_MEDIA_LDLIBS)
LOCAL_CFLAGS += $(SIPX_MEDIA_CFLAGS)
LOCAL_C_INCLUDES += $(SIPX_MEDIA_C_INCLUDES)

include $(BUILD_EXECUTABLE)

########################################################

#codecs

# =======================

include $(CLEAR_VARS)

# Set up the target identity.
LOCAL_MODULE := libcodec_pcmapcmu


LOCAL_SRC_FILES := \
    src/mp/codecs/plgpcmapcmu/CodecPcmaWrapper.c \
    src/mp/codecs/plgpcmapcmu/CodecPcmuWrapper.c \
    src/mp/codecs/plgpcmapcmu/G711.c \
    src/mp/codecs/plgpcmapcmu/PlgPcmaPcmu.c \
    contrib/libspandsp/src/g711.c


LOCAL_CFLAGS += -DCODEC_STATIC \
                -DDISABLE_STREAM_PLAYER


LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXtackLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/libspandsp/src \


LOCAL_LDLIBS += -lstdc++ -ldl

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)

# =======================

include $(CLEAR_VARS)

# Set up the target identity.
LOCAL_MODULE := libcodec_tones


LOCAL_SRC_FILES := \
    src/mp/codecs/plgtones/PlgTones.c \

LOCAL_CFLAGS += -DCODEC_STATIC \
                -DDISABLE_STREAM_PLAYER

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXtackLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \


LOCAL_LDLIBS += -lstdc++ -ldl

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)

# =======================

include $(CLEAR_VARS)

# Set up the target identity.
LOCAL_MODULE := libcodec_g722


LOCAL_SRC_FILES := \
    src/mp/codecs/plgg722/plgg722.c \
    contrib/libspandsp/src/g722_encode.c \
    contrib/libspandsp/src/g722_decode.c \


LOCAL_CFLAGS += -DCODEC_STATIC \
                -DDISABLE_STREAM_PLAYER


LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXtackLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/libspandsp/src \


LOCAL_LDLIBS += -lstdc++ -ldl

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)

# =======================

include $(CLEAR_VARS)

# Set up the target identity.
LOCAL_MODULE := libcodec_speex


LOCAL_SRC_FILES := \
    src/mp/codecs/plgspeex/PlgSpeex.c \
    src/mp/codecs/plgspeex/speex_nb.c \
    src/mp/codecs/plgspeex/speex_wb.c \
    src/mp/codecs/plgspeex/speex_uwb.c \


LOCAL_CFLAGS += -DCODEC_STATIC \
                -DDISABLE_STREAM_PLAYER


LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXtackLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/libspeex/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/android \


LOCAL_LDLIBS += -lstdc++ -ldl

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)

# =======================

include $(CLEAR_VARS)

# Set up the target identity.
LOCAL_MODULE := libcodec_opus


LOCAL_SRC_FILES := \
    src/mp/codecs/plgopus/plgopus.c 


LOCAL_CFLAGS += -DCODEC_STATIC \
                -DDISABLE_STREAM_PLAYER


LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXtackLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/libopus/opus/include \
    $(SIPX_HOME)/sipXmediaLib/contrib/libopus/ \
    $(SIPX_HOME)/sipXmediaLib/contrib/android \


LOCAL_LDLIBS += -lstdc++ -ldl
LOCAL_STATIC_LIBRARIES := libopus

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)

# =======================

# shared lib for JNI run of unit tests

include $(CLEAR_VARS)

LOCAL_MODULE := libsipxmediajnisandbox

LOCAL_SRC_FILES := \
  ../sipXportLib/src/test/sipxportunit/unitJni.cpp \
    src/test/mp/MpAudioBufTest.cpp \
    src/test/mp/MpBufTest.cpp \
    src/test/mp/MpCodecsPerformanceTest.cpp \
    src/test/mp/MpDspUtilsTest.cpp \
    src/test/mp/MpGenericResourceTest.cpp \
    src/test/mp/MpInputDeviceDriverTest.cpp \
    src/test/mp/MpFlowGraphTest.cpp \
    src/test/mp/MpMediaTaskTest.cpp \
    src/test/mp/MpOutputDriverTest.cpp \
    src/test/mp/MpOutputFrameworkTest.cpp \
    src/test/mp/MpResourceTest.cpp \
    src/test/mp/MpResourceTopologyTest.cpp \
    src/test/mp/MpTestResource.cpp \
    src/test/mp/MprBridgeTest.cpp \
    src/test/mp/MprBridgeTestWB.cpp \
    src/test/mp/MprFromMicTest.cpp \
    src/test/mp/MprMixerTest.cpp \
    src/test/mp/MprSpeakerSelectorTest.cpp \
    src/test/mp/MprSplitterTest.cpp \
    src/test/mp/MprToSpkrTest.cpp \
    src/test/mp/MprToneGenTest.cpp \

Unit_tests_crash := \
    src/test/mp/MpMMTimerTest.cpp \
    src/test/mp/MpOutputManagerTest.cpp \

#  src/test/mp/MpInputDeviceDriverTest.cpp
#  src/test/mp/MpOutputDriverTest.cpp
#  src/test/mp/MpOutputFrameworkTest.cpp

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/src/test \
    $(SIPX_HOME)/sipXportLib/src/test/sipxportunit \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXmediaLib/include \
    $(SIPX_HOME)/sipXmediaLib/src/test


LOCAL_SHARED_LIBRARIES := libpcre 
LOCAL_STATIC_LIBRARIES := libsipxUnit libsipXmedia libsipXsdp libsipXtack libsipXport $(SIPX_CODEC_LIBS)

LOCAL_LDLIBS += -lstdc++ -ldl

# Add sipXmediaLib dependencies
LOCAL_SHARED_LIBRARIES += $(SIPX_MEDIA_SHARED_LIBS)
LOCAL_STATIC_LIBRARIES += $(SIPX_MEDIA_STATIC_LIBS)
LOCAL_LDLIBS += $(SIPX_MEDIA_LDLIBS)
LOCAL_CFLAGS += $(SIPX_MEDIA_CFLAGS)
LOCAL_C_INCLUDES += $(SIPX_MEDIA_C_INCLUDES)

#include $(BUILD_EXECUTABLE)
include $(BUILD_SHARED_LIBRARY)


# =======================

