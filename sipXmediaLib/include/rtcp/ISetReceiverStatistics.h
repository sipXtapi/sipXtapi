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
#ifndef _ISetReceiverStatistics_h
#define _ISetReceiverStatistics_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "IRTPHeader.h"
#include "IBaseClass.h"

/**
 *
 * Interface Name:  ISetReceiverStatistics
 *
 * Inheritance:     None
 *
 *
 * Description:     The ISetReceiverStatistics interface allows the RTP Source
 *                  and RTP inbound Sender Reports to provide information
 *                  necessary in calculating RTCP Receiver Report statistics
 *                  associated within an inbound RTP connection.
 *
 * Notes:
 *
 */
interface ISetReceiverStatistics  : public IBaseClass
 {

//  Public Methods

public:

/**
 *
 * Method Name:  SetRTPStatistics
 *
 *
 * Inputs:      CRTPHeader *poRTPHeader -
 *                              RTP Packet Header received from RTP Source
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Takes the CRTPHeader object passed by the RTP Source object and
 *              updates Receiver Report statistics based upon its contents.
 *
 * Usage Notes:
 *
 *
 *
 */
    virtual void SetRTPStatistics(IRTPHeader *piRTPHeader) = 0;


/**
 *
 * Method Name: SetLastRcvdSRTime
 *
 *
 * Inputs:      unsigned long aulNTPTimestamp[]
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
    virtual void SetLastRcvdSRTime(unsigned long aulNTPTimestamp[]) = 0;

};


#endif

