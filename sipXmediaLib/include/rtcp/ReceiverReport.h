//
// Copyright (C) 2006-2013 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


//  Border Guard
#ifndef _ReceiverReport_h
#define _ReceiverReport_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "RTPHeader.h"
#include "RTCPHeader.h"
#include "IReceiverReport.h"
#include "IGetReceiverStatistics.h"
#include "ISetReceiverStatistics.h"



/**
 *
 * Class Name:  CReceiverReport
 *
 * Inheritance: CRTCPHeader          - RTCP Header Base Class
 *
 *
 * Interfaces:  IReceiverReport      - RTCP Receiver Report Interface
 *
 * Description: The CReceiverReport Class coordinates the processing and
 *              generation of RTCP receiver reports associated with either an
 *              inbound or outbound RTP connection.
 *
 * Notes:       CReceiverReport is derived from CBaseClass which provides
 *              basic Initialization and reference counting support.
 *
 */
class CReceiverReport : public CBaseClass,
                                 // Inherits CBaseClass implementation
                        public CRTCPHeader,
                                 // Inherits the CRTCPHeader implementation
                        public IReceiverReport,
                                 // Receiver Report Control Interface
                        public IGetReceiverStatistics,
                                 // Get Receiver Statistics Interface
                        public ISetReceiverStatistics
                                 // Set Receiver Statistics Interface
 {

//  Public Methods
public:

/**
 *
 * Method Name:  CReceiverReport() - Constructor
 *
 *
 * Inputs:       ssrc_t ulSSRC     - The the Identifier for this source
 *               uint32_t ulVersion  -
 *                                  Version of the RFC Standard being followed
 *
 *
 * Outputs:      None
 *
 * Returns:      None
 *
 * Description:  Performs routine CReceiverReport object initialization.
 *
 * Usage Notes:  A CReceiverReport object shall be created by the CRTCPRender
 *               with this constructor.  The Sender shall be responsible for
 *               maintain receiver statistics related to an outbound RTP
 *               connection.  The CReceiverReport shall pass the SSRC and
 *               version number to the CRTCPHeader at object construction.
 *
 */
    CReceiverReport(ssrc_t ulSSRC=0, unsigned long ulVersion=2);


/**
 *
 * Method Name: ~CReceiverReport() - Destructor
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Shall deallocated and/or release all resources which was
 *               acquired over the course of runtime.
 *
 * Usage Notes:
 *
 *
 */
    ~CReceiverReport(void);


/**
 *
 * Method Name:  SetRTPStatistics
 *
 *
 * Inputs:      IRTPHeader *piRTPHeader -
 *                            RTP Packet Header received from RTP Source
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Takes the RTPHeader interface passed by the RTP Source object
 *              and updates Receiver Report statistics based upon its contents.
 *
 * Usage Notes:
 *
 *
 *
 */
    void SetRTPStatistics(IRTPHeader *piRTPHeader);


/**
 *
 * Method Name: SetLastRcvdSRTime
 *
 *
 * Inputs:      uint32_t aulNTPTimestamp[]
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Store the RTP timestamp from the last Sender Report received
 *              and store the time that this report was received on the system.
 *
 * Usage Notes:
 *
 *
 *
 */
    void SetLastRcvdSRTime(uint32_t aulNTPTimestamp[]);


/**
 *
 * Method Name:  GetReceiverStatistics
 *
 *
 * Inputs:      None
 *
 * Outputs:     uint32_t  *pulFractionalLoss
 *                                - Fractional Packet Loss
 *              uint32_t  *pulCumulativeLoss
 *                                - Cumulative Packet Loss
 *              uint32_t  *pulHighestSequenceNo
 *                                - Highest Sequence Number Received
 *              uint32_t  *pulInterarrivalJitter
 *                                - Interarrival Packet Variance
 *              uint32_t  *pulSRTimestamp
 *                                - Timestamp of last Sender Report received
 *              uint32_t  *pulPacketDelay
 *                                - 1/65536 granular Delay between last Sender
 *                                  Report Received and sending this report
 *
 * Returns:     void
 *
 * Description: Returns a number of receiver report statistics associated with
 *              an inbound or outbound RTP connection.
 *
 * Usage Notes:
 *
 *
 *
 */
    void GetReceiverStatistics(uint32_t  *pulFractionalLoss,
                               uint32_t  *pulCumulativeLoss,
                               uint32_t  *pulHighestSequenceNo,
                               uint32_t  *pulInterarrivalJitter,
                               uint32_t  *pulSRTimestamp,
                               uint32_t  *pulPacketDelay);



/**
 *
 * Method Name:  FormatReceiverReport
 *
 *
 * Inputs:      bool           bSRPresent
 *                      - Specifies whether a Sender Report is prepended
 *              unsigned long  ulBufferSize
 *                      - length of the buffer
 *
 * Outputs:     unsigned char *puchReportBuffer
 *                      - Buffer used to store the Receiver Report
 *
 * Returns:     unsigned long
 *                      - Returns the number of octet written into the buffer.
 *
 * Description: Constructs a Receiver report using the buffer passed in by the
 *              caller.  A Receiver Report may be appended to the contents of
 *              a Sender Report or sent along in the case where no data was
 *              transmitted during the reporting period.  The header flag set
 *              to True shall cause the Receiver Report to be appended while
 *              False will cause a header to be prepended to the Report
 *              information.
 *
 *           A call to this method shall cause all period counts to be reset.
 *
 * Usage Notes: The header of the RTCP Report shall be formatted by delegating
 *              to the base class.
 *
 *
 *
 */
    unsigned long FormatReceiverReport(bool           bSRPresent,
                                       unsigned char *puchReportBuffer,
                                       unsigned long ulBufferSize);



/**
 *
 * Method Name:  ParseReceiverReport
 *
 *
 * Inputs:      boolean        bHeader
 *                               - Specifies whether a header is included
 *              unsigned char *puchReportBuffer
 *                               - Buffer used to store the Receiver Report
 *
 * Outputs:
 *
 * Returns:     unsigned long
 *                     - Returns the number of octets processed
 *
 * Description: Processes a Receiver report using the buffer passed in by the
 *              caller.  The header flag shall identify whether the report is
 *              prepended with a header.
 *
 * Usage Notes: The header of the RTCP Report, if provided, shall be parsed by
 *              delegating to the base class.
 *
 *
 */
    unsigned long  ParseReceiverReport(bool bHeader,
                                    unsigned char *puchReportBuffer);

/**
 *
 * Method Name:  GetSSRC
 *
 *
 * Inputs:       None
 *
 *
 * Outputs:      None
 *
 * Returns:     ssrc_t - The SSRC of the Bye Report
 *
 * Description: Returns the SSRC Associated with the Bye Report.
 *
 * Usage Notes:
 *
 *
 */
    ssrc_t GetSSRC(void);

/**
 *
 * Method Name:  SetSSRC
 *
 *
 * Inputs:      ssrc_t   ulSSRC   - Source ID
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Stores the Source Identifier associated with an RTP connection.
 *
 * Usage Notes:
 *
 *
 *
 */
    virtual void SetSSRC(ssrc_t ulSSRC);


/**
 *
 * Macro Name:  DECLARE_IBASE_M
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: This implements the IBaseClass functions used and exposed by
 *              derived classes.
 *
 * Usage Notes:
 *
 *
 */
DECLARE_IBASE_M

   private:     // Private Methods


/**
 *
 * Method Name:  SetRemoteSSRC
 *
 *
 * Inputs:      IRTPHeader *piRTPHeader
 *                            - Interface pointer to RTP Header services
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Stores the Source Identifier associated with an RTP source.
 *
 * Usage Notes:
 *
 *
 *
 */
    void SetRemoteSSRC(IRTPHeader *piRTPHeader);

/**
 *
 * Method Name:  UpdateJitter
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  This method shall use the current and previous packet send and
 *               receive time to calculate interarrival jitter.  The jitter
 *               calculated shall be used to update cumulative jitter and
 *               average jitter statistics.
 *
 * Usage Notes:
 *
 */
    void UpdateJitter(IRTPHeader *pIRTPHeader);


/**
 *
 * Method Name:  UpdateSequence
 *
 * Inputs:       IRTPHeader *piRTPHeader
 *                         - RTP Header of a recently received RTP packet
 *
 * Outputs:      None
 *
 * Returns:      None
 *
 * Description:  This method shall queue the RTP Header to a packet sequencing
 *               list and update the Highest Sequence Number statistic.  It
 *               shall then evaluate the sequence list to determine whether
 *               any packets have been lost.  Detection of one or more lost
 *               packets shall cause the UpdatePacketLoss() to be called with
 *               the number of packets lost.
 *
 * Usage Notes:
 *
 */
    void UpdateSequence(IRTPHeader *piRTPHeader);

/**
 *
 * Method Name:  UpdateSequenceNumbers
 *
 * Inputs:       IRTPHeader *piRTPHeader
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  This method shall calculate the highest extended sequence
 *               number.  This value shall be a combination of the highest
 *               sequence number received (low 16 bits) and the number of
 *               sequence number cycles (high 16 bits).
 *
 * Usage Notes:
 *
 */
    void UpdateSequenceNumbers(ssrc_t ulSequenceNo);

/**
 *
 * Method Name:  IncrementPacketCounters
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  This method shall increment both the period and cumulative
 *               packet counters.
 *
 * Usage Notes:
 *
 */
    void IncrementPacketCounters(void);


/**
 *
 * Method Name:  UpdateLostPackets
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  This method shall update the packet loss counts for the period
 *               and the session as well as calculate the fractional packet
 *               loss for the period.
 *
 * Usage Notes:
 *
 */
    void UpdateLostPackets(void);


/**
 *
 * Method Name:  LoadRemoteSSRC
 *
 * Inputs:       ssrc_t *pulPayloadBuffer
 *                                    - Payload buffer for loading data
 *
 * Outputs:      None
 *
 * Returns:     unsigned long
 *                     - Returns the number of octets processed
 *
 * Description:  This method shall load the SSRC ID of the reportee into the
 *               payload buffer.
 *
 * Usage Notes:
 *
 */
    unsigned long LoadRemoteSSRC(ssrc_t *pulPayloadBuffer);

/**
 *
 * Method Name:  LoadLossStatistics
 *
 * Inputs:       uint32_t *pulPayloadBuffer
 *                                    - Payload buffer for loading data
 *
 * Outputs:      None
 *
 * Returns:     unsigned long
 *                     - Returns the number of octets processed
 *
 * Description:  This method shall load the fractional and cumulative loss
 *               statistics into the payload buffer.
 *
 * Usage Notes:
 *
 */
    unsigned long LoadLossStatistics(uint32_t *pulPayloadBuffer);

/**
 *
 * Method Name:  LoadExtendedSequence
 *
 * Inputs:       uint32_t *pulPayloadBuffer
 *                                    - Payload buffer for loading data
 *
 * Outputs:      None
 *
 * Returns:     unsigned long
 *                     - Returns the number of octets processed
 *
 * Description:  This method shall load the extended highest sequence number.
 *               This is the number of times the 16 bit sequence number has
 *               wrapped shifted into the high 16 bits with the actual
 *               sequence number in the low 16 bits.
 *
 * Usage Notes:
 *
 */
    unsigned long LoadExtendedSequence(uint32_t *pulPayloadBuffer);

/**
 *
 * Method Name:  LoadJitter
 *
 * Inputs:       uint32_t *pulPayloadBuffer
 *                                    - Payload buffer for loading data
 *
 * Outputs:      None
 *
 * Returns:     unsigned long
 *                     - Returns the number of octets processed
 *
 * Description:  This method shall load the RTP packet interarrival jitter
 *               statistic.  This is an estimate of the statistical variance
 *               of the RTP data packet interarrival time, measured in
 *               timestamp units and expressed as an unsigned integer.
 *
 * Usage Notes:
 *
 */
    unsigned long LoadJitter(uint32_t *pulPayloadBuffer);

/**
 *
 * Method Name:  LoadReportTimes
 *
 * Inputs:       uint32_t *pulPayloadBuffer
 *                                    - Payload buffer for loading data
 *
 * Outputs:      None
 *
 * Returns:     unsigned long
 *                     - Returns the number of octets processed
 *
 * Description:  This method shall load the timestamp of the last SR Report
 *               received from an SSRC in addition to loading the delay
 *               between our receiving this SR packet and responding with an
 *               RR packet.
 *
 * Usage Notes:
 *
 */
    unsigned long LoadReportTimes(uint32_t *pulPayloadBuffer);

/**
 *
 * Method Name:  ExtractRemoteSSRC
 *
 * Inputs:       uint32_t *pulPayloadBuffer
 *                                    - Payload buffer for loading data
 *
 * Outputs:      None
 *
 * Returns:     unsigned long
 *                     - Returns the number of octets processed
 *
 * Description:  This method shall store the SSRC ID of the reportee found in
 *               the payload buffer.
 *
 * Usage Notes:
 *
 */
    unsigned long ExtractRemoteSSRC(uint32_t *pulPayloadBuffer);

/**
 *
 * Method Name:  ExtractLossStatistics
 *
 * Inputs:       uint32_t *pulPayloadBuffer
 *                                    - Payload buffer for loading data
 *
 * Outputs:      None
 *
 * Returns:     unsigned long
 *                     - Returns the number of octets processed
 *
 * Description:  This method shall store the fractional and cumulative loss
 *               statistics from the payload buffer.
 *
 * Usage Notes:
 *
 */
    unsigned long ExtractLossStatistics(uint32_t *pulPayloadBuffer);

/**
 *
 * Method Name:  ExtractExtendedSequence
 *
 * Inputs:       uint32_t *pulPayloadBuffer
 *                                    - Payload buffer for loading data
 *
 * Outputs:      None
 *
 * Returns:     unsigned long
 *                     - Returns the number of octets processed
 *
 * Description:  This method shall extract the extended highest sequence
 *               number.  This is the number of times the 16 bit sequence
 *               number has wrapped shifted into the high 16 bits with the
 *               actual sequence number in the low 16 bits.
 *
 * Usage Notes:
 *
 */
    unsigned long ExtractExtendedSequence(uint32_t *pulPayloadBuffer);

/**
 *
 * Method Name:  ExtractJitter
 *
 * Inputs:       uint32_t *pulPayloadBuffer
 *                                    - Payload buffer for loading data
 *
 * Outputs:      None
 *
 * Returns:     unsigned long
 *                     - Returns the number of octets processed
 *
 * Description:  This method shall extract the RTP packet interarrival jitter
 *               statistic.  This is an estimate of the statistical variance
 *               of the RTP data packet interarrival time, measured in
 *               timestamp units and expressed as an unsigned integer.
 *
 * Usage Notes:
 *
 */
    unsigned long ExtractJitter(uint32_t *pulPayloadBuffer);

/**
 *
 * Method Name:  ExtractReportTimes
 *
 * Inputs:       uint32_t *pulPayloadBuffer
 *                                    - Payload buffer for loading data
 *
 * Outputs:      None
 *
 * Returns:     unsigned long
 *                     - Returns the number of octets processed
 *
 * Description:  This method shall extract the timestamp of the last SR Report
 *               received from an SSRC in addition to loading the delay between
 *               our receiving this SR packet and responding with an RR packet.
 *
 * Usage Notes:
 *
 */
    unsigned long ExtractReportTimes(uint32_t *pulPayloadBuffer);


/**
 *
 * Method Name:  ResetPeriodCounts
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  This method shall reset all period counters and accumulators.
 *
 * Usage Notes:
 *
 */
    void ResetPeriodCounts(void);

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
 * Description:  This method shall reset all receiver report statistics.  A
 *               reset shall occur when the SSRC ID is reset due to a
 *               collision with a participating source.
 *
 * Usage Notes:
 *
 */
    void ResetStatistics(void);


   private:     // Private Data Members


/**
*
* Attribute Name:   m_csSynchronized
*
* Type:             CRITICAL_SECTION
*
* Description:   This is a critical section used to synchronize thread
*                access to this class.  This shall prevent several contending
*                threads from executing interleaving linked list operations.
*
*/
      CRITICAL_SECTION m_csSynchronized;

/**
 *
 * Attribute Name:  m_ulRemoteSSRC
 *
 * Type:            long
 *
 * Description:     This member shall store the SSRC ID of the FE participant
 *                  engaged in an RTP connection.
 *
 */
      ssrc_t m_ulRemoteSSRC;

/**
 *
 * Attribute Name:  m_ulTotalPacketCount
 *
 * Type:            long
 *
 * Description:     This member shall store the number of packets received
 *                  since the start of the inbound RTP connection.
 *
 */
      uint32_t m_ulTotalPacketCount;

/**
 *
 * Attribute Name:  m_ulPeriodPacketCount
 *
 * Type:            uint32_t
 *
 * Description:     This member shall store the number of packets received
 *                  over the reporting period.
 *
 */
      uint32_t m_ulPeriodPacketCount;

/**
 *
 * Attribute Name:  m_ulPeriodPacketLoss
 *
 * Type:            uint32_t
 *
 * Description:     This member shall store the number of packets lost over
 *                  the reporting period.
 *
 */
      uint32_t m_ulPeriodPacketLoss;

/**
 *
 * Attribute Name:  m_ulFractionalLoss
 *
 * Type:            uint32_t
 *
 * Description: This member shall store the Fractional Packet Loss express as
 *       m_ulPeriodPacketLoss / m_ulPeriodPacketCount + m_ulPeriodPacketLoss
 *
 */
      uint32_t m_ulFractionalLoss;


/**
 *
 * Attribute Name:  m_ulCumulativeLoss
 *
 * Type:            uint32_t
 *
 * Description:     This member shall store the cumulative packet loss since
 *                  the start of the inbound RTP connection.
 *
 */
      uint32_t m_ulCumulativeLoss;

/**
 *
 * Attribute Name:  m_ulFirstSequenceNo
 *
 * Type:            uint32_t
 *
 * Description:     This member shall store the first sequence number received
 *                  in an inbound RTP data packet.
 */
     uint32_t m_ulFirstSequenceNo;

/**
 *
 * Attribute Name:  m_ulLastSequenceNo
 *
 * Type:            uint32_t
 *
 * Description:     This member shall store the last sequence number received
 *                  in an inbound RTP data packet.
 */
     uint32_t m_ulLastSequenceNo;

/**
 *
 * Attribute Name:  m_ulSequenceWraps
 *
 * Type:            uint32_t
 *
 * Description:     This member shall store the number of time the sequence
 *                  number has wrapped given its 16 bit resolution.
 */
     uint32_t m_ulSequenceWraps;


/**
 *
 * Attribute Name:  m_ulHighestSequenceNo
 *
 * Type:            uint32_t
 *
 * Description:     This member shall store the highest sequence number
 *                  received in an inbound RTP data packet in the low 16 bits
 *                  and the corresponding count of sequence number cycles in
 *                  the most significant 16 bits.
 */
     uint32_t m_ulHighestSequenceNo;

/**
 *
 * Attribute Name:  m_ulLastPeriodExpectedCount
 *
 * Type:            uint32_t
 *
 * Description:     This member shall store the number of packets expected for
 *                  the last reporting period.  This calculation is based on
 *                  the highest and first sequence numbers acquired for that
 *                  period.
 */
     uint32_t m_ulLastPeriodExpectedCount;


/**
 *
 * Attribute Name:  m_ulLastPeriodPacketCount
 *
 * Type:            uint32_t
 *
 * Description:     This member shall store the number of actual packets
 *                  received over the previous reporting period.
 */
     uint32_t m_ulLastPeriodPacketCount;

/**
 *
 * Attribute Name:  m_ulLastPacketSendTime
 *
 * Type:            uint32_t
 *
 * Description:     This member shall store the RTP timestamp associated with
 *                  the last RTP packet received.
 */
     uint32_t m_ulLastPacketSendTime;

/**
 *
 * Attribute Name:  m_ulLastPacketReceiveTime
 *
 * Type:            uint32_t
 *
 * Description:     This member shall store the receive timestamp associated
 *                  with the last RTP packet received.
 */
     uint32_t m_ulLastPacketReceiveTime;


/**
 *
 * Attribute Name:  m_ulMeanJitter
 *
 * Type:            uint32_t
 *
 * Description:     This member shall store the mean interarrival jitter.
 */
     uint32_t  m_ulMeanJitter;


/**
 *
 * Attribute Name:  m_ulLastSRTimestamp
 *
 * Type:            uint32_t
 *
 * Description:     This member shall store the timestamp for the last Send
 *                  Report sent by the FE.  This is the middle 32 bits of
 *                  the NTP timestamp of the SR.
 */
     uint32_t m_ulLastSRTimestamp;

/**
 *
 * Attribute Name:  m_dLastSRRcvdTimestamp
 *
 * Type:            double
 *
 * Description:     This member shall store the timestamp for the last Send
 *                  Report received from the FE.  This is the system time when
 *                  the packet was received.
 */
     double m_dLastSRRcvdTimestamp;


/**
 *
 * Attribute Name:  m_ulSRDelay
 *
 * Type:            uint32_t
 *
 * Description:     This member shall store the delay between the formation
 *                  of this Receiver Report and the last Send Report received
 *                  for this RTP connection.
 */
     uint32_t  m_ulSRDelay;

/**
 *
 * Attribute Name:  m_ulCached....
 *
 * Type:            uint32_t
 *
 * Description:     These member shall store the statistics from the last
 *                  reporting period so that they may be viewed by interested
 *                  clients.  This is only a temporary solution that should be
 *                  addressed through creating a copy of the Receiver Report
 *                  object after the reporting period has expired.
 */
     uint32_t  m_ulCachedFractionalLoss;
     uint32_t  m_ulCachedCumulativeLoss;
     uint32_t  m_ulCachedHighestSequenceNo;
     uint32_t  m_ulCachedMeanJitter;
     uint32_t  m_ulCachedLastSRTimestamp;
     uint32_t  m_ulCachedSRDelay;

     int mTotalPackets;
     int mTotalWarnings;

};

/**
 *
 * Method Name:  GetSSRC
 *
 *
 * Inputs:       None
 *
 *
 * Outputs:      None
 *
 * Returns:     ssrc_t - The SSRC of the Bye Report
 *
 * Description: Returns the SSRC Associated with the Bye Report.
 *
 * Usage Notes:
 *
 *
 */
inline ssrc_t CReceiverReport::GetSSRC(void)
{

    return(CRTCPHeader::GetSSRC());

}

#endif
