//
// Copyright (C) 2022 SIP Spectrum, Inc.  All rights reserved.
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
#ifndef _IRTCPHeader_h
#define _IRTCPHeader_h

#include "rtcp/RtcpConfig.h"

//  Enumerations
typedef enum {
    etInvalidReport,
    etSenderReport = 200,
    etReceiverReport,
    etSDESReport,
    etByeReport,
    etAppReport,
    etXRReport = 207  // RFC3611 Extended Report
} RTCP_REPORTS_ET;

/**
 *
 * Interface Name:  IRTCPHeader
 *
 * Inheritance:     None
 *
 *
 * Description:     The IRTCPHeader interface allows a user to parse and form
 *                  a RTCP Report headers according to the format defined in
 *                  RFC 1889.It also provides a user with services to extract
 *                  information from this header once it has been parsed or
 *                  formed.
 *
 * Notes:
 *
 */

interface IRTCPHeader
 {

//  Public Methods
public:

virtual ~IRTCPHeader() {}

/**
 *
 * Method Name:  GetHeaderLength
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long  - Return the size of the RTCP Header
 *
 * Description: Retrieves the size of the RTCP Header that preceeds the payload.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetHeaderLength(void)=0;

/**
 *
 * Method Name:  GetSSRC
 *
 *
 * Inputs:      None
 *
 * Outputs:     Bobe
 *
 * Returns:     unsigned long  - Return the SSRC ID
 *
 * Description: Retrieves the SSRC attribute stored within the object.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetSSRC(void)=0;


/**
 *
 * Method Name: GetVersion
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long  - Protocol Version #
 *
 * Description: Returns the protocol version number associated with the RTCP
 *              packet.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetVersion(void)=0;

/**
 *
 * Method Name: GetPadding
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long  - Padding Flag
 *
 * Description: Returns the padding flag associated with the RTCP packet.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetPadding(void)=0;


/**
 *
 * Method Name:  GetReportCount
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Returns Report Count
 *
 * Description: Retrieves the report count associated with this RTCP report.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetReportCount(void)=0;


/**
 *
 * Method Name:  GetReportlength
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Returns Report Length
 *
 * Description: Retrieves the report length associated with this RTCP report.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetReportLength(void)=0;


/**
 *
 * Method Name: GetPayload
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     RTCP_REPORTS_ET - Returns Payload Type
 *
 * Description: Returns the payload type value associated with the RTCP packet.
 *
 * Usage Notes:
 *
 *
 */
    virtual RTCP_REPORTS_ET GetPayload(void)=0;

/**
 *
 * Method Name:  IsOurSSRC
 *
 *
 * Inputs:      None
 *
 * Outputs:     ssrc_t ulSSRC  - SSRC ID
 *
 * Returns:     boolean
 *
 * Description: Compares the SSRC ID passed to that stored as an attribute
 *              within this object instance.  Will return either True or
 *              False based on the match.
 *
 * Usage Notes:
 *
 *
 */
    virtual bool IsOurSSRC(ssrc_t ulSSRC)=0;



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
 * Usage Notes:
 *
 *
 *
 */
    virtual void SetSSRC(unsigned long ulSSRC)=0;



};


#endif
