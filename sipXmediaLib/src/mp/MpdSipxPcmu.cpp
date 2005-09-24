//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#if 1 /* [ */
#ifndef HAVE_GIPS /* [ */
#ifdef __pingtel_on_posix__ /* [ */
#include <sys/types.h>
#include <netinet/in.h>
#endif /* __pingtel_on_posix__ ] */
// APPLICATION INCLUDES
#include "mp/MpConnection.h"
#include "mp/MpdSipxPcmu.h"
#include "mp/JB/JB_API.h"
#include "mp/MprDejitter.h"
#include "mp/MpSipxDecoders.h"

const MpCodecInfo MpdSipxPcmu::smCodecInfo(
         SdpCodec::SDP_CODEC_PCMU, JB_API_VERSION, true,
         8000, 8, 1, 160, 64000, 1280, 1280, 1280, 160, 3);

MpdSipxPcmu::MpdSipxPcmu(int payloadType)
   : MpDecoderBase(payloadType, &smCodecInfo)
{
    mNextPullTimerCount = 0;
    mWaitTimeInFrames = 6;  // This is the jitter buffer size. 6 = 60ms   This is not yet adaptive, and pretty random in selection
    mUnderflowCount=0; 
    mLastSeqNo = 0;
    mTooManyPacketsInBuffer=0;
    mTooFewPacketsInBuffer=0;
    mLastReportSize=-1;
    osPrintf("MpdSipxPcmu::MpdSipxPcmu(%d)\n", payloadType);
}

MpdSipxPcmu::~MpdSipxPcmu()
{
   freeDecode();
}

OsStatus MpdSipxPcmu::initDecode(MpConnection* pConnection)
{
    if(pConnection != NULL) {
   //Get JB pointer
   pJBState = pConnection->getJBinst();

   // Set the payload number for JB
   JB_initCodepoint(pJBState, "PCMU", 8000, getPayloadType());
    }
//   osPrintf("MpdSipxPcmu::initDecode: payloadType=%d\n", getPayloadType());
   return OS_SUCCESS;
}

OsStatus MpdSipxPcmu::freeDecode(void)
{
   return OS_SUCCESS;
}

int MpdSipxPcmu::decode(JB_uchar *encoded, int inSamples, Sample *decoded) 
{
	return G711U_Decoder(inSamples, encoded, decoded);
}

int MpdSipxPcmu::reportBufferLength(int iAvePackets)
{
	// Every seconds, the decoder interface reports to us the average number of packets in the 
	// jitter buffer. We want to keep it near our target buffer size
	//osPrintf("Got a buffer report: %d\n",iAvePackets);  // DEBUG STATEMENT
    if(iAvePackets <= 1) return 0;  // Zero or one is the starting condition
    int iMax = mWaitTimeInFrames+2;
    int iMin = mWaitTimeInFrames-1; 
    if(iMin<1) iMin=1;
    if(mLastReportSize == -1) mLastReportSize = iAvePackets;
    if (iAvePackets < iMin) 
    {
        // There are too few packets in the buffer. Skip the pull the next time around.
        if(iAvePackets>=(mLastReportSize-1)) 
        {
            // Only react when the buffer length changes mildly, not dramatically, since the latter
            // probalbly isn't clock drift
            mTooFewPacketsInBuffer=1;
        }
        mLastReportSize = iAvePackets;
        return 0;
    }
    mLastReportSize = iAvePackets;
    return 0;
}

void MpdSipxPcmu::FrameIncrement(void) 
{
	// increment the pull timer count one frame's worth
    mNextPullTimerCount+=80;
}
int MpdSipxPcmu::decodeIn(MpBufPtr pPacket)
{
    // THIS METHOD CAN BE CALLED MORE THAN ONE TIME PER FRAME INTERVAL
    // This method figures out if we want this packet or not. If no, return a 0, else return the packet size
    unsigned int rtpTimestamp = MprDejitter::getTimestamp(pPacket);
    //osPrintf("rtpTimestamp %u\n",rtpTimestamp);   // DEBUG STATEMENT ONLY
    unsigned int delta = 0; // to contain the difference between the current pull pointer and the rtpTimestamp
    // Use the delta because in rtpTimestamp and mNextPullTimerCount are unsigned, so a straight subtraction will fail
    if(rtpTimestamp > mNextPullTimerCount) 
    {
        delta = rtpTimestamp - mNextPullTimerCount;
    } else {
        delta = mNextPullTimerCount - rtpTimestamp;
    }
    if(delta > (160*(mWaitTimeInFrames*2))) 
    {
        osPrintf("Detected timercount silence, skip or stream startup, resetting nextPullTimerCount: rtpTimestamp %u, mNextPullTimerCount %u, sTimerCountIncrement %u, mWaitTimeInFrames %d\n",
                rtpTimestamp, mNextPullTimerCount, 80,mWaitTimeInFrames);
        // if we have underflowed the JB, make the reset to a higher value
        if(mTooFewPacketsInBuffer>0) 
        {
            mNextPullTimerCount=rtpTimestamp-(160*(mWaitTimeInFrames*2));
            osPrintf("Clock drift detected, too few packets in buffer! Nexttimer is %u\n",mNextPullTimerCount);
        } else {
            mNextPullTimerCount=rtpTimestamp-(160*mWaitTimeInFrames);
        }
        return 0;
    }
    if(rtpTimestamp <= mNextPullTimerCount) 
    {
        // A packet is available within the allotted time span
        mUnderflowCount=0;
        // Process the frame if enough time has passed
        int iSeqNo = ntohs(((struct rtpHeader*) MpBuf_getStorage(pPacket))->seq);
        if(iSeqNo < mLastSeqNo) 
        {
            osPrintf("Out of Order Discard\n");
            return -1;  // Discard the packet, it is out of order
        }
        return MpBuf_getContentLen(pPacket);
    } else {
	    // Print error if we are not pulling packets for some reason
        mUnderflowCount++;
        //if(mUnderflowCount > 4) {           // DEBUGGING SEQUENCE ONLY
        //	osPrintf(" W%d ",mUnderflowCount
        //	osPrintf("WAITING on the pull %d\n",mUnderflowCount);
        //	osPrintf(" NP:rt=%u,tc=%u ", rtpTimestamp, mNextPullTimerCount);
        //}
        if(mUnderflowCount>40) 
        {
            osPrintf("Not Pulling: rtpTimestamp %u, mNextPullTimerCount %u, sTimerCountIncrement %u, mWaitTimeInFrames %d\n",
                    rtpTimestamp, mNextPullTimerCount, 80,mWaitTimeInFrames);
            mUnderflowCount=0;
        }
        return 0;  // Don't want the packet
    }
}

#endif /* HAVE_GIPS ] */
#endif /* ] */
