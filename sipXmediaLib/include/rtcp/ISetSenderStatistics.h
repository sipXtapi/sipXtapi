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
#ifndef _ISetSenderStatistics_h
#define _ISetSenderStatistics_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "IBaseClass.h"


//  Defines
#define SAMPLES_PER_SEC     8000       // Default samples per second

/**
 *
 * Interface Name:  ISetSenderStatistics
 *
 * Inheritance:     None
 *
 *
 * Description:     The ISetSenderStatistics interface allows consumers to
 *                  increment the cumulative packet and octet count of either
 *                  an inbound or outbound RTP connection.
 *
 * Notes:
 *
 */
interface ISetSenderStatistics : public IBaseClass
 {

//  Public Methods

public:

/**
 *
 * Method Name:  IncrementCounts
 *
 *
 * Inputs:       unsigned long  ulOctetCount    -   RTP Octets Sent
 *
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
    virtual void IncrementCounts(uint32_t ulOctetCount, rtpts_t RTPTimestampBase, rtpts_t RTPTimestamp, ssrc_t ssrc) = 0;

/**
 *
 * Method Name:  SetRTPTimestamp
 *
 *
 * Inputs:       unsigned long ulRandomOffset -
 *                                  Random Offset for RTP Timestamp
 *               unsigned long ulSamplesPerSecond  -
 *                                  Number of sample per second
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  The SetRTPTimestamp method shall initialized values that are
 *               used to determine the RTP Timestamp value to be sent out in
 *               an SR Report.
 *
 * Usage Notes:
 *
 */
    virtual void CSR_SetRTPTimestamp(uint32_t ulRandomOffset,
                     uint32_t ulSamplesPerSecond = SAMPLES_PER_SEC) = 0;

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
    virtual void SetSRAdjustUSecs(int iUSecs = 0) = 0;

};

#endif
