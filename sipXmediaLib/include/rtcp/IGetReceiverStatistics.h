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
#ifndef _IGetReceiverStatistics_h
#define _IGetReceiverStatistics_h

#include "rtcp/RtcpConfig.h"

// Include
#include "IBaseClass.h"

/**
 *
 * Interface Name:  IGetReceiverStatistics
 *
 * Inheritance:     None
 *
 *
 * Description:     The IGetReceiverStatistics interface allows consumers to
 *                  retrieve the performance and continuity statistics of an
 *                  RTCP Receiver Report associated with an inbound or
 *                  outbound RTP connection.
 *
 * Notes:
 *
 */
interface IGetReceiverStatistics : public IBaseClass
 {

//  Public Methods

public:

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
    virtual ssrc_t GetSSRC(void)=0;

/**
 *
 * Method Name:  GetReceiverStatistics
 *
 *
 * Inputs:   None
 *
 * Outputs:  uint32_t  *pulFractionalLoss
 *             Fractional Packet Loss
 *           uint32_t  *pulCumulativeLoss
 *             Cumulative Packet Loss
 *           uint32_t  *pulHighestSequenceNo
 *             Highest Sequence Number Received
 *           uint32_t  *pulInterarrivalJitter
 *             Interarrival Packet Variance
 *           uint32_t  *pulSRTimestamp
 *             Timestamp of last Sender Report received
 *           uint32_t  *pulPacketDelay
 *             Delay between last Sender Report Received and sending this
 *             report
 *
 * Returns:     void
 *
 * Description: Returns a number of receiver report statistics associated
 *              with an inbound or outbound RTP connection.
 *
 * Usage Notes:
 *
 *
 *
 */
virtual void GetReceiverStatistics(uint32_t   *pulFractionalLoss,
                                   uint32_t   *pulCumulativeLoss,
                                   uint32_t   *pulHighestSequenceNo,
                                   uint32_t   *pulInterarrivalJitter,
                                   uint32_t   *pulSRTimestamp,
                                   uint32_t   *pulPacketDelay) = 0;



};

#endif

