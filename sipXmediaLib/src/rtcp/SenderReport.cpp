//
// Copyright (C) 2006-2013 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////



// Includes

#include <os/OsIntTypes.h>
#include <os/OsLock.h>
#include <os/OsDateTime.h>

#include <assert.h>

#ifdef WIN32
#   ifndef WINCE
#       include <sys/timeb.h>
#   endif
#elif defined(__pingtel_on_posix__)
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#define ERROR (-1)
#endif
#include "rtcp/SenderReport.h"
#ifdef INCLUDE_RTCP /* [ */

#ifdef WIN32
#   include <winsock2.h>
#endif

#include "os/OsSysLog.h"

// Constants
// Difference between LocalTime and Wall Time:
        const unsigned long WALLTIMEOFFSET  = 2208992400UL;

// 2**32 ("4 Gig"):
        const double        FOUR_GIGABYTES  = (65536.0*65536.0);

// Microsecond to second conversion
        const double        MICRO2SEC       = 1000000.0;

// Millisecond to MicroSecond Conversion
        const int           MILLI2MICRO     = 1000;
        const int           MICRO2NANO      = 1000;

// Receiver Report Length:
        const int           RR_LENGTH       = 24;


//  A lock to protect access to timestamp members
OsBSem SR_sMultiThreadLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

/**
 *
 * Method Name:  CSenderReport() - Constructor
 *
 *
 * Inputs:   ssrc_t           ulSSRC  - The Identifier for this source
 *           ISetReceiverStatistics *piSetStatistics
 *                                     - Interface for setting receiver stats
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description:  Performs routine CSenderReport object initialization.
 *
 * Usage Notes:  A CSenderReport object shall be created by the CRTCPRender
 *               with this constructor.  The Sender shall be responsible for
 *               maintain sender statistics related to an outbound RTP
 *               connection.  The constructor shall be pass the SSRC and an
 *               optional pointer to the Set Statistics interface of the
 *               receiver report.
 *
 */
CSenderReport::CSenderReport(ssrc_t ulSSRC,
                             ISetReceiverStatistics *piSetStatistics) :
          CBaseClass(CBASECLASS_CALL_ARGS("CSenderReport", __LINE__)),
          CRTCPHeader(ulSSRC, etSenderReport),  // Base class construction
          m_ulPacketCount(0),
          m_ulOctetCount(0),
          m_bMediaSent(FALSE),
          m_ulRTPTimestamp(0)
          , m_iTSCollectState(0)
          , m_iUSecAdjust(0)
{


    // Store the Statistics interface as an attribute
    m_piSetReceiverStatistics = piSetStatistics;

    // Increment the interface's reference counter
    if(m_piSetReceiverStatistics)
        m_piSetReceiverStatistics->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));

    // Initialize NTP Timestamps
    m_aulNTPTimestamp[0]      = 0;
    m_aulNTPTimestamp[1]      = 0;
    m_aulNTPStartTime[0]      = 0;
    m_aulNTPStartTime[1]      = 0;

    ResetStatistics();
}



/**
 *
 * Method Name: ~CSenderReport() - Destructor
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Shall deallocate and/or release all resources that were
 *              acquired over the course of runtime.
 *
 * Usage Notes:
 *
 *
 */
CSenderReport::~CSenderReport(void)
{

// Our reference count must have gone to 0 to get here.  We have not allocated
// any memory so we shall now go quietly into that good night!

    // Release the interface's reference counter
    if(m_piSetReceiverStatistics)
        m_piSetReceiverStatistics->Release(ADD_RELEASE_CALL_ARGS(__LINE__));

}


/**
 *
 * Method Name:  WasMediaSent
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  bool
 *
 * Description:  A method to determine whether media has been sent out since
 *               the last reporting period.  This will determine whether a
 *               Sender Report or Receiver Report is in order.
 *
 * Usage Notes:
 *
 */
bool CSenderReport::WasMediaSent(void)
{
    return(m_bMediaSent && (m_iTSCollectState == 3));
}

/**
 *
 * Method Name:  IncrementCounts
 *
 *
 * Inputs:       unsigned long  ulOctetCount    -   RTP Octets Sent
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  The IncrementCounts method shall add the number of octets
 *               passed to the cumulative octet count stored as an attribute
 *               to this object. Each call to IncrementCounts() shall also
 *               increment the packet count by 1.
 *
 * Usage Notes:
 *
 */
void CSenderReport::IncrementCounts(uint32_t ulOctetCount, rtpts_t RTPTimestampBase, rtpts_t RTPTimestamp, ssrc_t ssrc)
{
    uint32_t ntp_secs;
    uint32_t ntp_usec;

    OsTime now;
    OsDateTime::getCurTime(now);
    // Load Most Significant word with Wall time seconds
    ntp_secs = now.seconds();
    // Load Least Significant word with Wall time microseconds
    ntp_usec = now.usecs();

    // OsSysLog::add(FAC_MP, PRI_DEBUG, "CSenderReport::IncrementCounts: this=%p, NTP = {%2d.%06d}, octets=%04d, rtpTSBase=%u=0x%08X, rtpTS=%u=0x%08X, SSRC=0x%08X,0x%08X", this, (ntp_secs&0x3F), ntp_usec, ulOctetCount, RTPTimestampBase, RTPTimestampBase, RTPTimestamp, RTPTimestamp, ssrc, GetSSRC());

    OsLock lock(SR_sMultiThreadLock);

    if ((m_ulRTPTimestampBase != RTPTimestampBase) || ((0xFFFFFFFF & GetSSRC()) != (0xFFFFFFFF & ssrc))) {
        SetSSRC(ssrc); // NOTE: Calls ResetStatistics()
        m_ulRTPTimestampBase = RTPTimestampBase;
    }

    // We will increment the packet count by 1 and the Octet count by the
    //  number specified within the octet count
    m_ulPacketCount++;
    m_ulOctetCount += ulOctetCount;

    switch (m_iTSCollectState)
    {
    case 0: // Initial state, save first values
        m_ulRTPTimestamps[0] = RTPTimestamp;
        m_ulNTPSeconds[0] = ntp_secs;
        m_ulNTPuSecs[0] = ntp_usec;
        m_iTSCollectState = 1;
        break;

    case 1: // Second state, wait for first value to change
        if (m_ulRTPTimestamps[0] != RTPTimestamp) {
            m_ulRTPTimestamps[1] = RTPTimestamp; // needed for state #2
            m_ulRTPTimestamps[0] = RTPTimestamp;
            m_ulNTPSeconds[0] = ntp_secs;
            m_ulNTPuSecs[0] = ntp_usec;
            m_iTSCollectState = 2;
        }
        break;

    case 2: // Save last values for the first time
    case 3: // Steady state, save last values, allow SR construction
        if (m_ulRTPTimestamps[1] != RTPTimestamp) {
            m_ulRTPTimestamps[1] = RTPTimestamp;
            m_ulNTPSeconds[1] = ntp_secs;
            m_ulNTPuSecs[1] = ntp_usec;
            m_iTSCollectState = 3;
        }
        break;

    default:
        assert(0);
    }

    // Set the Media Sent flag so that we know to transmit a Sender
    // Report in the next reporting period.
    m_bMediaSent = TRUE;

}

/**
 *
 * Method Name:  SetRTPTimestamp
 *
 *
 * Inputs:   unsigned long ulRandomOffset     - Random Offset for RTP Timestamp
 *           unsigned long ulSamplesPerSecond - Number of samples per second
 *
 * Outputs:  None
 *
 * Returns:  void
 *
 * Description:  The SetRTPTimestamp method shall initialize the values
 *               that are used to determine the RTP Timestamp value to be
 *               sent out in an SR Report.
 *
 * Usage Notes:
 *
 */

/*****************************************************************************
 * Feb 11, 2013 (hzm)
 * IMHO, this is garbage.  Much like ExtractTimestamps...
 * Also, ulSamplesPerSecond is not used -- this may explain part of the
 *   bogosity of ExtractTimestamps.
 ****************************************************************************/

void CSenderReport::CSR_SetRTPTimestamp(uint32_t ulRandomOffset,
                                    uint32_t ulSamplesPerSecond)
{

    // OsSysLog::add(FAC_MP, PRI_DEBUG, "CSenderReport::SetRTPTimestamp(%d=0x%X, %d=0x%X)", ulRandomOffset, ulRandomOffset, ulSamplesPerSecond, ulSamplesPerSecond);

    // Set Timestamp Information

    // Let's check whether an initial NTP timestamp has been established.
    //  If so, ignore.
    if(!m_aulNTPStartTime[0] && !m_aulNTPStartTime[1])
    {
        double dTimestamp;

#ifdef WIN32
        struct _timeb stLocalTime;

        // Get the LocalTime expressed as seconds since 1/1/70 (UTC)
        _ftime(&stLocalTime);

        // Load Most Significant word with Wall time seconds
        m_aulNTPStartTime[0] = (uint32_t)stLocalTime.time + WALLTIMEOFFSET;

        // Load Least Significant word with Wall time microseconds
        dTimestamp = stLocalTime.millitm * MILLI2MICRO;
        dTimestamp *= (double)(FOUR_GIGABYTES/MICRO2SEC);

#elif defined(__pingtel_on_posix__)
        struct timeval tv;

        gettimeofday(&tv, NULL);
        // OsSysLog::add(FAC_MP, PRI_DEBUG, "CSenderReport::SetRTPTimestamp: tv = {%ld.%06ld}, sizes = %ld+%ld = %ld = %ld", tv.tv_sec, tv.tv_usec, sizeof(tv.tv_sec), sizeof(tv.tv_usec), sizeof(tv), sizeof(struct timeval));
        // Load Most Significant word with Wall time seconds
        m_aulNTPStartTime[0] = tv.tv_sec + WALLTIMEOFFSET;

        // Load Least Significant word with Wall time microseconds
        dTimestamp = (double) tv.tv_usec / MILLI2MICRO;
        dTimestamp *= (double) (FOUR_GIGABYTES / MICRO2SEC);

#else
        struct timespec stLocalTime;

        // Make a call to VxWorks to get this timestamp
        if (clock_gettime(CLOCK_REALTIME, &stLocalTime) == ERROR)
        {
            osPrintf("**** FAILURE **** SetRTPTimestamp() - clock_gettime failure\n");
            stLocalTime.tv_sec = 0;
            stLocalTime.tv_nsec = 0;
        }

        // Load Most Significant word with Wall time seconds
        m_aulNTPStartTime[0] = stLocalTime.tv_sec + WALLTIMEOFFSET;

        // Load Least Significant word with Wall time microseconds
        dTimestamp = (double)stLocalTime.tv_nsec / MILLI2MICRO;
        dTimestamp *= (double)(FOUR_GIGABYTES / MICRO2SEC);

#endif

        // Store microsecond portion of NTP
        m_aulNTPStartTime[1] = (uint32_t)dTimestamp;

    }
}

/**
 *
 * Method Name:  GetSenderStatistics
 *
 *
 * Inputs:      None
 *
 * Outputs:     unsigned long   *pulPacketCount   - Sender Packet Count
 *              unsigned long   *pulOctetCount    - Sender Octet Count
 *
 * Returns:     void
 *
 * Description: Returns the packet and octet counts values stored as members.
 *
 * Usage Notes:
 *
 *
 *
 */
void CSenderReport::GetSenderStatistics(uint32_t *pulPacketCount,
                                        uint32_t *pulOctetCount)
{

    // Pass back the current packet and octet counts
    *pulPacketCount = m_ulPacketCount;
    *pulOctetCount  = m_ulOctetCount;
    // OsSysLog::add(FAC_MP, PRI_DEBUG, "CSenderReport::GetSenderStatistics: Packets=%d, Octets=%d", m_ulPacketCount, m_ulOctetCount);

}


/**
 *
 * Method Name:  SetSSRC
 *
 *
 * Inputs:      unsigned long   ulSSRC   - Source ID
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Stores the Source Identifier associated with an RTP connection.
 *
 * Usage Notes: This is an override of the base class method defined in
 *              CRTCPHeader.  This method shall additionally reset the octet
 *              and packet count accumulators as mandated by standard.
 *
 *
 *
 */
void CSenderReport::SetSSRC(ssrc_t ulSSRC)
{
    OsSysLog::add(FAC_MP, PRI_DEBUG, "CSenderReport::SetSSRC(0x%8X) ", ulSSRC);

    // An SSRC collision must have been detected for this to occur.
    // Let's reset our statistics.
    ResetStatistics();

    // Let's delegate to the base class method to set the new SSRC ID
    CRTCPHeader::SetSSRC(ulSSRC);

}



/**
 *
 * Method Name:  FormatSenderReport
 *
 *
 * Inputs:   unsigned long  ulBufferSize     - length allocated for the buffer
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
 *
 * Outputs:  unsigned char *puchReportBuffer
 *                                    - Buffer used to store the Sender Report
 *
 * Returns:  unsigned long - number of octets written into the buffer.
 *
 * Description: Constructs a Sender report using the buffer passed in by the
 *              caller.  The Sender Report object shall keep track of the
 *              reporting periods that have passed an which information should
 *              be used to populate the report.
 *
 * Usage Notes: The header of the RTCP Report shall be formatted by delegating
 *              to the base class.
 *
 *
 */
unsigned long CSenderReport::FormatSenderReport(
                  unsigned char *puchReportBuffer, unsigned long ulBufferSize)
{

    unsigned long ulReportLength=0;

    // Let's offset into the Formatting buffer enough to
    //  start depositing payload
    unsigned char *puchPayloadBuffer = puchReportBuffer + GetHeaderLength();

    OsLock lock(SR_sMultiThreadLock);

    // Let's load the NTP and RTP timestamps into the Sender Report
    puchPayloadBuffer += LoadTimestamps((uint32_t *)puchPayloadBuffer);

    // Let's load the Sender Statistics
    puchPayloadBuffer += LoadSenderStats((uint32_t *)puchPayloadBuffer);

    // Set the sender report length
    ulReportLength = puchPayloadBuffer - puchReportBuffer;

    // Let's slap a header on this report
    FormatRTCPHeader( puchReportBuffer,  // RTCP Report Buffer
          FALSE,                         // No Padding
          1,                             // Receiver Count set to 1 for now
          ulReportLength + RR_LENGTH);   // Report Length

    return(ulReportLength);

}


/**
 *
 * Method Name:  ParseSenderReport
 *
 *
 * Inputs:   unsigned char *puchReportBuffer
 *                               - Buffer containing the Sender Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: Extracts the contents of an Sender report using the buffer
 *              passed in by the caller.  The Sender Report object shall store
 *              the content and length of data fields extracted from the Sender
 *              Report.  The timestamps identifying the time of SR report
 *              reception shall obtained and sent with the SR Send timestamp to
 *              the associated Receiver Report through the SetLastRcvdSRTime()
 *              method of the ISetReceiverStatistics interface.
 *
 * Usage Notes: The header of the RTCP Report shall be parsed by delegating to
 *              the base class.
 *
 *
 */
unsigned long CSenderReport::ParseSenderReport(unsigned char *puchReportBuffer)
{

    unsigned char   *puchPayloadBuffer = puchReportBuffer;

    // Check whether the RTCP Header has been correctly
    //  formed (Version, etc...).
    if(!ParseRTCPHeader(puchReportBuffer))
        return(GetReportLength());

    // Good header.  Let's bump the payload pointer and continue.
    puchPayloadBuffer += GetHeaderLength();

    // Let's extract the NTP and RTP timestamps from the Sender Report
    puchPayloadBuffer += ExtractTimestamps((uint32_t *)puchPayloadBuffer);

    // Let's extract the Sender Statistics
    puchPayloadBuffer += ExtractSenderStats((uint32_t *)puchPayloadBuffer);

    return(puchPayloadBuffer - puchReportBuffer);

}




/**
 *
 * Method Name:  ResetStatistics
 *
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  This method shall reset all sedner report statistics.
 *               A reset shall occur when the SSRC ID is reset due to a
 *               collision with a participating source.
 *
 * Usage Notes:
 *
 */
void CSenderReport::ResetStatistics(void)
{

    // We must set both the packet and octet counts to 0
    OsSysLog::add(FAC_MP, PRI_DEBUG, "CSenderReport::ResetStatistics: SSRC=0x%8X", CRTCPHeader::GetSSRC());
    m_ulPacketCount = m_ulOctetCount = 0;
    m_ulRTPTimestampBase = m_ulRTPTimestamps[0] = m_ulRTPTimestamps[1] = 0;
    m_ulNTPSeconds[0] = m_ulNTPSeconds[1] = m_ulNTPuSecs[0] = m_ulNTPuSecs[1] = 0;
    m_bMediaSent = FALSE;
    m_iTSCollectState = 0;
}


/**
 *
 * Method Name:  SetSRAdjustUSecs
 *
 *
 * Inputs:       int iUSecs - signed # of microseconds of skew adjustment
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  The SetSRAdjustUSecs method sets an adjustment for skew, in
 *               microseconds, for the RTP time in the SR Report.
 *
 * Usage Notes:
 *
 */
void CSenderReport::SetSRAdjustUSecs(int iUSecs)
{
    m_iUSecAdjust = iUSecs;
}

/**
 *
 * Method Name:  LoadTimestamps
 *
 *
 * Inputs:   None
 *
 * Outputs:  uint32_t *aulTimestamps - Long word array in which to load
 *                                          the NTP and RTP timestamps
 *                                          (WHAT FORMAT???)
 *
 * Returns:  unsigned long - Size of the data loaded (WHAT UNITS???)
 *
 * Description:  This method shall use the VxWorks Network time protocol
 *               service to get a 64 bit representation of the current Network
 *               time and 32 bit for RTP time.
 *
 * Usage Notes:
 *
 */
unsigned long CSenderReport::LoadTimestamps(uint32_t *aulTimestamps)
{
    double dTimestampUSec;  // # uSec, 0..999999 as a double, then converted to a fraction between 0.0 and 0.999999
    double dTimestampFrac;  // dTimestampUSec as a fixed point fraction, ie. dTimestampUSec * 2**32
    double dNow, dFirst, dSecond;
    double dExtrap = 1.0; // the extrapolation factor for the RTP timestamp
    double dSkewAdjust = 0.0;
    double dSampleRate = -1.0;
    unsigned long ret = 0;
    uint32_t ntp_secs;
    uint32_t ulRTPDelta1 = 0;
    uint32_t ulRTPDelta2 = 0;

        // OsSysLog::add(FAC_MP, PRI_DEBUG, "CSenderReport::LoadTimestamps");
#ifdef WIN32
    struct _timeb stLocalTime;

    // Get the LocalTime expressed as seconds since 1/1/70 (UTC)
    _ftime(&stLocalTime);

    // Load Most Significant word with Wall time seconds
    ntp_secs = (unsigned long)stLocalTime.time;

    // Load Least Significant word with Wall time microseconds
    dTimestampUSec = stLocalTime.millitm * MILLI2MICRO;

#elif defined(__pingtel_on_posix__)
    struct timeval tv;

    gettimeofday(&tv, NULL);
    // OsSysLog::add(FAC_MP, PRI_DEBUG, "CSenderReport::LoadTimestamps: tv = {%ld.%06ld}", tv.tv_sec, tv.tv_usec);
    // Load Most Significant word with Wall time seconds
    ntp_secs = tv.tv_sec;

    // Load Least Significant word with Wall time microseconds
    dTimestampUSec = (double) tv.tv_usec;

#else
    struct timespec stLocalTime;

    // Make a call to VxWorks to get this timestamp
    if (clock_gettime(CLOCK_REALTIME, &stLocalTime) == ERROR)
    {
        osPrintf("**** FAILURE **** LoadTimestamps() - clock_gettime failure\n");
        stLocalTime.tv_sec = 0;
        stLocalTime.tv_nsec = 0;
    }

    // Load Most Significant word with Wall time seconds
    ntp_secs = stLocalTime.tv_sec;

    // Load Least Significant word with Wall time microseconds
    dTimestampUSec = (double)stLocalTime.tv_nsec / MICRO2NANO;


#endif

    dTimestampUSec = dTimestampUSec / ((double) MICRO2SEC);  // convert to fraction
    dTimestampFrac = dTimestampUSec * ((double) FOUR_GIGABYTES); // convert to fixed point 32-bit fraction
    dNow = (double) ntp_secs + dTimestampUSec;
    dFirst =  (double) m_ulNTPSeconds[0] + (((double) m_ulNTPuSecs[0]) / ((double) MICRO2SEC));
    dSecond = (double) m_ulNTPSeconds[1] + (((double) m_ulNTPuSecs[1]) / ((double) MICRO2SEC));
    dSkewAdjust = (((double) m_iUSecAdjust) / ((double) MICRO2SEC));
    if (m_ulRTPTimestamps[0]) {
        ulRTPDelta1 = (m_ulRTPTimestamps[1] - m_ulRTPTimestamps[0]);
        dExtrap = ((dNow + dSkewAdjust - dFirst) / (dSecond - dFirst));
        dSampleRate = (double) ulRTPDelta1 / (dSecond - dFirst);
        ulRTPDelta2 = (uint32_t) (((double)ulRTPDelta1) * dExtrap);
    }
    m_ulRTPTimestamp = m_ulRTPTimestampBase + m_ulRTPTimestamps[0] + ulRTPDelta2;

    // OsSysLog::add(FAC_MP, PRI_DEBUG, "CSenderReport::LoadTimestamps:@dFirst=%.6f, dSecond=%.6f, dNow=%.6f, dSkewAdjust=%.4f, dExtrap=%.4f,@ts0=%d, ts1=%d, dTS1=%d, dTS2=%d, TSB=%d, uSec=%d, SampleRate=%.2f", dFirst, dSecond, dNow, dSkewAdjust, dExtrap, m_ulRTPTimestamps[0], m_ulRTPTimestamps[1], ulRTPDelta1 , ulRTPDelta2, m_ulRTPTimestampBase, m_iUSecAdjust, dSampleRate);

    // Adjust to 1900-based from 1970-based.
    m_aulNTPTimestamp[0] = ntp_secs + WALLTIMEOFFSET;

    // Store microsecond portion of NTP
    m_aulNTPTimestamp[1] = (uint32_t)dTimestampFrac;
    // OsSysLog::add(FAC_MP, PRI_DEBUG, "CSenderReport::LoadTimestamps: m_aulNTPTimestamp={%u.%10u}", m_aulNTPTimestamp[0], m_aulNTPTimestamp[1]);

    // Assign NTP Time to Sender Report Buffer
    aulTimestamps[0] = htonl(m_aulNTPTimestamp[0]);
    aulTimestamps[1] = htonl(m_aulNTPTimestamp[1]);
    // OsSysLog::add(FAC_MP, PRI_DEBUG, "CSenderReport::LoadTimestamps: m_aulNTPTimestamp = {0x%08X,0x%08X}, aulTimestamps = {0x%08X,0x%08X}", m_aulNTPTimestamp[0], m_aulNTPTimestamp[1], aulTimestamps[0], aulTimestamps[1]);
    aulTimestamps += 2;

    *aulTimestamps =  htonl(m_ulRTPTimestamp);

    ret += sizeof(m_aulNTPTimestamp);
    ret += sizeof(m_ulRTPTimestamp);
    // OsSysLog::add(FAC_MP, PRI_DEBUG, "CSenderReport::LoadTimestamps: sizeof(m_aulNTPTimestamp) = %ld, sizeof(m_ulRTPTimestamp) = %ld, ret = %ld", sizeof(m_aulNTPTimestamp), sizeof(m_ulRTPTimestamp), ret);
    return (ret);

}


/**
 *
 * Method Name:  LoadSenderStats
 *
 *
 * Inputs:   None
 *
 * Outputs:  uint32_t *aulSenderStats - Long word array in which
 *                                           to load the statistics
 *
 * Returns:  unsigned long - Amount of data loaded
 *
 * Description:  This method shall retrieve the packet and octet counts.
 *
 * Usage Notes:
 *
 */
unsigned long  CSenderReport::LoadSenderStats(uint32_t *aulSenderStats)
{

    // The RTP timestamp shall be based on the NTP timestamp
    *aulSenderStats     = htonl(m_ulPacketCount);
    *(aulSenderStats+1) = htonl(m_ulOctetCount);

    // Reset the Media Sent flag to so that we can determine whether a Sender
    // Report is necessary.
    m_bMediaSent = FALSE;
    m_iTSCollectState = 0;
    // OsSysLog::add(FAC_MP, PRI_DEBUG, "CSenderReport::LoadSenderStats: this=%p, P=%d, C=%d, ret=%d", this, m_ulPacketCount, m_ulOctetCount, (int)(sizeof(m_ulPacketCount) + sizeof(m_ulOctetCount)));

    return(sizeof(m_ulPacketCount) + sizeof(m_ulOctetCount));

}


/**
 *
 * Method Name:  ExtractTimestamps
 *
 *
 * Inputs:   uint32_t *paulTimestamps
 *                                   - Array containing the NTP/RTP Timestamps
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Size of the data extracted
 *
 * Description:  This method shall extract the 64 bits of NTP time information
 *               and the 32-bits of RTP timestamp and store them both in
 *               respective report attributes.
 *
 * Usage Notes:
 *
 */

/*****************************************************************************
 * Feb 11, 2013 (hzm)
 * IMHO, this is garbage.
 * In the pingtel_on_posix case it seems to do an extra divide by 1000.
 * I do not see where it gets any notion of samples per second for the
 *   SECOND assignment to m_ulRTPTimestamp (the FIRST seems to be only
 *   for practice, since it is never used...).
 * The separate host cases should just extract the current time as seconds
 *   and microseconds, then do all the adjustments after the #endif.  Better,
 *   use the portable OsTime abstraction!
 *
 * The good news is that it does not appear that anything uses the results...
 ****************************************************************************/

unsigned long CSenderReport::ExtractTimestamps(uint32_t *paulTimestamps)
{

    unsigned long aulCurrentNTPTime[2];
    double        dTimestamp; // only the FRACTIONAL part?

    // Load the two long word into NTP timestamp array
    m_aulNTPTimestamp[0] = ntohl(*paulTimestamps);
    paulTimestamps++;
    m_aulNTPTimestamp[1] = ntohl(*paulTimestamps);
    paulTimestamps++;

    // Store the RTP timestamp associated with this report
    m_ulRTPTimestamp = ntohl(*paulTimestamps);


    // Let's perform some calculations that will be useful in
    //  determining SR Delay
#ifdef WIN32
    struct _timeb stLocalTime;

    // Get the LocalTime expressed as seconds since 1/1/70 (UTC)
    _ftime(&stLocalTime);

    // Load Most Significant word with Wall time seconds
    aulCurrentNTPTime[0] = (unsigned long)stLocalTime.time + WALLTIMEOFFSET;

    // Load Least Significant word with Wall time microseconds
    dTimestamp = stLocalTime.millitm * MILLI2MICRO;
    dTimestamp *= (double)(FOUR_GIGABYTES/MICRO2SEC);

#elif defined(__pingtel_on_posix__)
    struct timeval tv;

    gettimeofday(&tv, NULL);
    // Load Most Significant word with Wall time seconds
    m_aulNTPStartTime[0] = tv.tv_sec + WALLTIMEOFFSET;

    // Load Least Significant word with Wall time microseconds
    dTimestamp = (double) tv.tv_usec / MILLI2MICRO;
    dTimestamp *= (double) (FOUR_GIGABYTES / MICRO2SEC);

#else
    struct timespec stLocalTime;

    // Make a call to VxWorks to get this timestamp
    if (clock_gettime(CLOCK_REALTIME, &stLocalTime) == ERROR)
    {
        osPrintf("**** FAILURE **** LoadTimestamps() - clock_gettime failure\n");
        stLocalTime.tv_sec = 0;
        stLocalTime.tv_nsec = 0;
    }

    // Load Most Significant word with Wall time seconds
    aulCurrentNTPTime[0] = stLocalTime.tv_sec + WALLTIMEOFFSET;

    // Load Least Significant word with Wall time microseconds
    dTimestamp = (double)stLocalTime.tv_nsec / MILLI2MICRO;
    dTimestamp *= (double)(FOUR_GIGABYTES / MICRO2SEC);


#endif

    // Store microsecond portion of NTP
    aulCurrentNTPTime[1] = (uint32_t)dTimestamp;


    // Calculate Current RTP Timestamp by taking the difference
    //  between the current and starting NTP timestamps
    double dSecondsElapsed  =
                        (double)(aulCurrentNTPTime[0] - m_aulNTPStartTime[0]);
// *** rtcp/SenderReport.cpp:831: warning: 'aulCurrentNTPTime[0]' is used uninitialized in this function ***

    double dUSecondsElapsed =
                        (double)(aulCurrentNTPTime[1] - m_aulNTPStartTime[1]);

    // Round Seconds down if Microsecond difference is less that 0.
    while (dUSecondsElapsed < 0)
    {
        dSecondsElapsed--;
        dUSecondsElapsed += MICRO2SEC;
    }

    // Express in fractions of seconds
    dUSecondsElapsed /= MICRO2SEC;

    // Express total elapsed time in sample Units per seond
    m_ulRTPTimestamp = (uint32_t)(dSecondsElapsed + dUSecondsElapsed);


    // Use the CReceiverReport's ISetReceiverStatistics Interface to timestamp
    //  when the last Sender Report was Received.
    m_piSetReceiverStatistics->SetLastRcvdSRTime(m_aulNTPTimestamp);

    return(sizeof(m_aulNTPTimestamp) + sizeof(m_ulRTPTimestamp));

}

/**
 *
 * Method Name:  ExtractSenderStats
 *
 *
 * Inputs:   uint32_t *aulSenderStats
 *                           - Long word array in which to load the statistics
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Amount of data extracted
 *
 * Description:  This method shall extract the packet and octet counts from
 *               the Sender Report.
 *
 * Usage Notes:
 *
 */
unsigned long  CSenderReport::ExtractSenderStats(uint32_t *aulSenderStats)
{

    // The RTP timestamp shall be based on the NTP timestamp
    m_ulPacketCount = ntohl(*aulSenderStats);
    m_ulOctetCount =  ntohl(*(aulSenderStats+1));

    return(sizeof(m_ulPacketCount) + sizeof(m_ulOctetCount));

}


#endif /* INCLUDE_RTCP ] */
