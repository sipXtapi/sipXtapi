## Process this file with automake to produce Makefile.in

SUBDIRS = test mp/codecs

AM_CPPFLAGS = -I$(top_srcdir)/include 

lib_LTLIBRARIES = libsipXmedia.la


## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libsipXmedia_la_LDFLAGS = \
	-version-info ${version_Current}:${version_Revision}:${version_Age} \
	@GSM_LDFLAGS@

if ENABLE_LOCAL_AUDIO
LOCAL_AUDIO_CXXFLAGS = -D_INCLUDE_AUDIO_SUPPORT
else
LOCAL_AUDIO_CXXFLAGS = -DDISABLE_LOCAL_AUDIO
endif

libsipXmedia_la_CXXFLAGS = \
	-D_REALTIME_LINUX_AUDIO_THREADS \
	$(LOCAL_AUDIO_CXXFLAGS) \
	@GSM_CXXFLAGS@ \
	@OPUS_INCLUDE@ \
	@OPUS_CFLAGS@ \
	@SPEEX_CFLAGS@

libsipXmedia_la_LIBADD = \
	@SIPXTACK_LIBS@ \
	@SIPXPORT_LIBS@ \
	@SIPXSDP_LIBS@ \
	@CA_LIBS@ \
	@GSM_LIBS@ \
	@OPUS_STATIC_LIB@ \
	@SPEEXDSP_LIBS@ \
	@STATIC_CODEC_LIBS@ \
	-Lmp/codecs/ \
	-lasound \
	-lstdc++

if COREAUDIO_DRIVERS
AUDIODRIVERS_SOURCE = \
        mp/MpodCoreAudio.cpp \
        mp/MpidCoreAudio.cpp
else
AUDIODRIVERS_SOURCE = \
        mp/MpidOss.cpp \
        mp/MpodOss.cpp \
        mp/MpOss.cpp \
        mp/MpOssContainer.cpp \
        mp/MpidAlsa.cpp \
        mp/MpodAlsa.cpp \
        mp/MpAlsa.cpp \
        mp/MpAlsaContainer.cpp 
endif

libsipXmedia_la_SOURCES =  \
    ../config/sipxmedialib-buildstamp.h \
    ../config/sipxmedialib-buildstamp.cpp \
    $(AUDIODRIVERS_SOURCE) \
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
    mp/MprnStringMsg.cpp \
    mp/MprNotchFilter.cpp \
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
    mp/MprToneDetect.cpp \
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
    mp/MpVideoResource.cpp \
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
