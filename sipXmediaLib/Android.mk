#
# Copyright (C) 2009 SIPfoundry Inc.
# Licensed by SIPfoundry under the LGPL license.
#
# Copyright (C) 2009 SIPez LLC.
# Licensed to SIPfoundry under a Contributor Agreement.
#
#
#//////////////////////////////////////////////////////////////////////////
#
# Author: Dan Petrie (dpetrie AT SIPez DOT com)
#
#
# This Makefile is for building sipXtackLib as a part of Android NDK.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Set up the target identity.
# LOCAL_MODULE/_CLASS are required for local-intermediates-dir to work.
LOCAL_MODULE := libsipXtack
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
#intermediates := $(call local-intermediates-dir)

fail_to_compile := \


LOCAL_SRC_FILES := \
    mp/dft.cpp \
    mp/dmaTaskPosix.cpp \
    mp/DSPlib.cpp \
    mp/dtmflib.cpp \
    mp/FilterBank.cpp \
    mp/HandsetFilterBank.cpp \
    mp/mpau.cpp \
    mp/MpAgcBase.cpp \
    mp/MpAgcSimple.cpp \
    mp/MpArrayBuf.cpp \
    mp/MpAudioAbstract.cpp \
    mp/MpAudioBuf.cpp \
    mp/MpAudioFileDecompress.cpp \
    mp/MpAudioFileOpen.cpp \
    mp/MpAudioFileUtils.cpp \
    mp/MpAudioOutputConnection.cpp \
    mp/MpAudioResource.cpp \
    mp/MpAudioUtils.cpp \
    mp/MpAudioWaveFileRead.cpp \
    mp/MpBridgeAlgLinear.cpp \
    mp/MpBridgeAlgSimple.cpp \
    mp/MpBuf.cpp \
    mp/MpBufPool.cpp \
    mp/MpBufferMsg.cpp \
    mp/MpCallFlowGraph.cpp \
    mp/MpCodec.cpp \
    mp/MpCodecFactory.cpp \
    mp/MpDataBuf.cpp \
    mp/MpDecoderBase.cpp \
    mp/MpDecoderPayloadMap.cpp \
    mp/MpDspUtils.cpp \
    mp/MpDTMFDetector.cpp \
    mp/MpEncoderBase.cpp \
    mp/MpFlowGraphBase.cpp \
    mp/MpFlowGraphMsg.cpp \
    mp/mpG711.cpp \
    mp/MpInputDeviceDriver.cpp \
    mp/MpInputDeviceManager.cpp \
    mp/MpJbeFixed.cpp \
    mp/MpJitterBuffer.cpp \
    mp/MpJitterBufferEstimation.cpp \
    mp/MprHook.cpp \
    mp/MpMediaTask.cpp \
    mp/MpMediaTaskMsg.cpp \
    mp/MpMisc.cpp \
    mp/MpMMTimer.cpp \
    mp/MpMMTimerPosix.cpp \
    mp/MpodBufferRecorder.cpp \
    mp/MpOutputDeviceDriver.cpp \
    mp/MpOutputDeviceManager.cpp \
    mp/MpPlayer.cpp \
    mp/MpPlayerEvent.cpp \
    mp/MprBridge.cpp \
    mp/MprDecode.cpp \
    mp/MprDejitter.cpp \
    mp/MprDelay.cpp \
    mp/MprEchoSuppress.cpp \
    mp/MprEncode.cpp \
    mp/MpResampler.cpp \
    mp/MpResamplerSpeex.cpp \
    mp/MpResource.cpp \
    mp/MpResourceFactory.cpp \
    mp/MpResourceMsg.cpp \
    mp/MpResourceSortAlg.cpp \
    mp/MpResourceTopology.cpp \
    mp/MpResNotificationMsg.cpp \
    mp/MpRtpBuf.cpp \
    mp/MpUdpBuf.cpp \
    mp/MprAudioFrameBuffer.cpp \
    mp/MprFromFile.cpp \
    mp/MprFromInputDevice.cpp \
    mp/MprFromMic.cpp \
    mp/MprFromNet.cpp \
    mp/MprFromStream.cpp \
    mp/MprMixer.cpp \
    mp/MprnDTMFMsg.cpp \
    mp/MprnIntMsg.cpp \
    mp/MprnProgressMsg.cpp \
    mp/MprnRtpStreamActivityMsg.cpp \
    mp/MprNull.cpp \
    mp/MprNullAec.cpp \
    mp/MprRecorder.cpp \
    mp/MprRtpDispatcher.cpp \
    mp/MprRtpDispatcherActiveSsrcs.cpp \
    mp/MprRtpDispatcherIpAffinity.cpp \
    mp/MprSpeakerSelector.cpp \
    mp/MpSineWaveGeneratorDeviceDriver.cpp \
    mp/MpStaticCodecInit.cpp \
    mp/MprSpeexEchoCancel.cpp \
    mp/MprSpeexPreProcess.cpp \
    mp/MprSplitter.cpp \
    mp/MprToneGen.cpp \
    mp/MprToNet.cpp \
    mp/MprToOutputDevice.cpp \
    mp/MprToSpkr.cpp \
    mp/MpRtpInputConnection.cpp \
    mp/MpRtpOutputConnection.cpp \
    mp/MprVad.cpp \
    mp/MprVoiceActivityNotifier.cpp \
    mp/MpStreamFeeder.cpp \
    mp/MpStreamMsg.cpp \
    mp/MpStreamPlayer.cpp \
    mp/MpStreamPlaylistPlayer.cpp \
    mp/MpStreamQueuePlayer.cpp \
    mp/MpSpeakerSelectBase.cpp \
    mp/MpPlcBase.cpp \
    mp/MpPlcSilence.cpp \
    mp/MpPlgStaffV1.cpp \
    mp/MpTopologyGraph.cpp \
    mp/MpTypes.cpp \
    mp/MpVadBase.cpp \
    mp/MpVadSimple.cpp \
    mp/NetInTask.cpp \
    mp/StreamBufferDataSource.cpp \
    mp/StreamDataSource.cpp \
    mp/StreamDataSourceListener.cpp \
    mp/StreamDecoderListener.cpp \
    mp/StreamFileDataSource.cpp \
    mp/StreamFormatDecoder.cpp \
    mp/StreamHttpDataSource.cpp \
    mp/StreamQueueingFormatDecoder.cpp \
    mp/StreamQueueMsg.cpp \
    mp/StreamRAWFormatDecoder.cpp \
    mp/StreamWAVFormatDecoder.cpp \
    rtcp/BaseClass.cpp \
    rtcp/ByeReport.cpp \
    rtcp/Message.cpp \
    rtcp/MsgQueue.cpp \
    rtcp/NetworkChannel.cpp \
    rtcp/ReceiverReport.cpp \
    rtcp/RTCManager.cpp \
    rtcp/RTCPConnection.cpp \
    rtcp/RTCPHeader.cpp \
    rtcp/RTCPRender.cpp \
    rtcp/RTCPSession.cpp \
    rtcp/RTCPSource.cpp \
    rtcp/RTCPTimer.cpp \
    rtcp/RTPHeader.cpp \
    rtcp/SenderReport.cpp \
    rtcp/SourceDescription.cpp

# Not immediately needed on Android
FOO_DONT_BUILD := \


LOCAL_CXXFLAGS += -D__pingtel_on_posix__ \
                  -DANDROID \
                  -DDEFINE_S_IREAD_IWRITE \
                  -DSIPX_TMPDIR=\"/usr/var/tmp\" -DSIPX_CONFDIR=\"/etc/sipxpbx\"

#ifeq ($(TARGET_ARCH),arm)
#	LOCAL_CFLAGS += -DARMv5_ASM
#endif

#ifeq ($(TARGET_BUILD_TYPE),debug)
#	LOCAL_CFLAGS += -DDEBUG
#endif

LOCAL_C_INCLUDES += \
    $(SIPX_HOME)/libpcre \
    $(SIPX_HOME)/sipXportLib/include \
    $(SIPX_HOME)/sipXsdpLib/include \
    $(SIPX_HOME)/sipXtackLib/include \
    $(SIPX_HOME)/sipXmediaLib/include

LOCAL_SHARED_LIBRARIES := libpcre libsipXport libsipXsdp libsipXtack

#LOCAL_STATIC_LIBRARIES := 

LOCAL_LDLIBS += -lstdc++ -ldl

include $(BUILD_SHARED_LIBRARY)
