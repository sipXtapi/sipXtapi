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
#ifndef _RTCPHeader_h
#define _RTCPHeader_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "BaseClass.h"
#include "IRTCPHeader.h"


// Defines
#define MAX_SOURCE_LENGTH   256 // Max Length of a NULL terminated SDES element
#define HEADER_LENGTH         8 // Header Size

#define PAYLOAD_OFFSET        1
#define LENGTH_OFFSET         2
#define SSRC_OFFSET           4
#define COUNT_MASK         0x1F


/**
 *
 * Class Name:  CRTCPHeader
 *
 * Inheritance: CBaseClass   - Base Class Implementation
 *
 *
 * Interfaces:  IRTCPHeader  - Services for loading and extracting RTCP header
 *                             info.
 *
 * Description: The CRTCPHeader Class manages the basic header and structure
 *              information associated with an RTCP report.
 *
 * Notes:       The CRTCPHeader is inherited by all RTCP Report objects.
 *
 */
class CRTCPHeader
{

//  Public Methods
public:

/**
 *
 * Method Name:  CRTCPHeader() - Constructor
 *
 *
 * Inputs:    unsigned long ulSSRC
 *                          - The the Identifier for this source
 *            RTCP_REPORTS_ET etPayloadType
 *                          - The Payload type associated with this report
 *            unsigned long ulVersion
 *                          - Version of the RFC Standard being followed
 *
 *
 * Outputs:   None
 *
 * Returns:   None
 *
 * Description:  The CRTCPHeader is an abstract class that is initialized by
 *               a derived object at construction time.
 *
 * Usage Notes:
 *
 */
    CRTCPHeader(ssrc_t ulSSRC,
                RTCP_REPORTS_ET etPayloadType,
                unsigned long ulVersion=2);



/**
 *
 * Method Name: ~CRTCPHeader() - Destructor
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Shall deallocated and/or release all resources which was
 *              acquired over the course of runtime.
 *
 * Usage Notes:
 *
 *
 */
    virtual ~CRTCPHeader(void);

/**
 *
 * Method Name: VetPacket
 *
 *
 * Inputs:   unsigned char *buffer   - Data Buffer received from Network Source
 *           int           bufferLen - Length of Data Buffer
 *
 * Outputs:  OsSysLog messages
 *
 * Returns:  int: length of valid RTCP packet; may be 0 (first chunk not valid,
 *           or any multiple of 4 up to as much as 3 larger than the input length.
 *           (e.g. 61, 62, or 63 may return 64).
 *
 * Description: VetPacket() walks the headers in the report chunks in a received
 *              RTCP packet applying various sanity checks.  It is to be called
 *              before calling ProcessPacket so that ProcessPacket (and its
 *              subsidiaries) can assume a degree of basic correctness.
 *
 *              In order to fix up for a relatively harmless deviation from
 *              the RFC, if the length of the packet as read from the socket
 *              is not a multiple of 4, this routine will write 0 to the
 *              1, 2, or 3 bytes following the end of the packet and then
 *              round the length up to that next multiple of 4 before walking
 *              the chunks.
 *
 *              After making sure that the length is a multiple of 4, the
 *              headers will be walked.  The checks are
 *               1.  The first two bits each header must be 0b10 (RTCP ver 2)
 *               2.  The PT is in the range 200..204; if not, emit warning
 *               3.  The length indicates that the chunk ends within the packet
 *                   data, and either 8 or more bytes from the end, or exactly
 *                   at the end.
 *
 */

    static int VetPacket(unsigned char* buffer, int bufferLen);

/**
 *
 * Method Name:  GetHeaderLength
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long  - Returns the size of the RTCP Header
 *
 * Description: Retrieves the size of the RTCP Header that preceeds
 *              the payload.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetHeaderLength(void);

/**
 *
 * Method Name:  GetVersion
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long  - Returns the Version
 *
 * Description: Retrieves the Version attribute stored within the object.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetVersion(void);

/**
 *
 * Method Name:  GetPadding
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long  - Returns the Padding Flag
 *
 * Description: Retrieves the Padding attribute stored within the object.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetPadding(void);

/**
 *
 * Method Name:  GetPayload
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     RTCP_REPORTS_ET   - Returns the Payload Type
 *
 * Description: Retrieves the payload type associated with this RTCP report.
 *
 * Usage Notes:
 *
 *
 */
    virtual RTCP_REPORTS_ET GetPayload(void);


/**
 *
 * Method Name:  GetReportCount
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long   - Returns the Report Count
 *
 * Description: Retrieves the report count associated with this RTCP report.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetReportCount(void);


/**
 *
 * Method Name:  GetReportlength
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long   - Returns the Report Length
 *
 * Description: Retrieves the report length associated with this RTCP report.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetReportLength(void);

/**
 *
 * Method Name:  GetSSRC
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long  - Return the SSRC IDt
 *
 * Description: Retrieves the SSRC attribute stored within the object.
 *
 * Usage Notes:
 *
 *
 */
    virtual ssrc_t GetSSRC(void);

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
    virtual void SetSSRC(ssrc_t ulSSRC);

/**
 *
 * Method Name:  IsOurSSRC
 *
 *
 * Inputs:      ssrc_t ulSSRC  - SSRC ID
 *
 * Outputs:     None
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
    virtual bool IsOurSSRC(ssrc_t ulSSRC);

protected:   // Protected Methods


/**
 *
 * Method Name:  FormatRTCPHeader
 *
 *
 * Inputs:  unsigned long ulPadding       - Padding used
 *          unsigned long ulCount         - Report Count
 *          unsigned long ulPayloadLength - Payload Length (Excluding Header)
 *
 * Outputs: unsigned char *puchRTCPBuffer
 *                            - Buffer used to store the RTCP Report Header
 *
 * Returns: unsigned long
 *          - Returns the number of octets written into the buffer.
 *
 * Description: Constructs an RTCP Report report using information stored
 *              and passed by the caller.
 *
 * Usage Notes: A buffer of sufficient size should be allocated and passed
 *              to this formatting method.
 *
 *
 */
    unsigned long FormatRTCPHeader(unsigned char *puchRTCPBuffer,
                                   unsigned long ulPadding,
                                   unsigned long ulCount,
                                   unsigned long ulPayloadLength);


/**
 *
 * Method Name:  ParseRTCPHeader
 *
 *
 * Inputs:      unsigned char *puchRTCPBuffer
 *         - Character Buffer containing the contents of the RTCP Report
 *
 * Outputs:     None
 *
 * Returns:     bool
 *
 * Description: Extracts the header contents of an RTCP report using the
 *              buffer passed in by the caller.  The header will be validated
 *              to determine whether it has an appropriate version, payload
 *              type, and SSRC for this object.
 *
 * Usage Notes:
 *
 *
 */
    bool ParseRTCPHeader(unsigned char *puchRTCPBuffer);



protected:      // Protected Data Members



/**
 *
 * Attribute Name:  m_ulVersion
 *
 * Type:            unsigned long
 *
 * Description:     The protocol version of the RTCP Report.
 *
 */
      unsigned long m_ulVersion;

/**
 *
 * Attribute Name:  m_ulPadding
 *
 * Type:            unsigned long
 *
 * Description: A flag identifying the use of padding within an RTCP report.
 *
 */
      unsigned long m_ulPadding;

/**
 *
 * Attribute Name:  m_ulCount
 *
 * Type:            unsigned long
 *
 * Description: The number of composite records contained with an RTCP report.
 *
 */
      unsigned long m_ulCount;


/**
 *
 * Attribute Name:  m_etPayloadType
 *
 * Type:            RTCP_REPORTS_ET
 *
 * Description:     The RTCP Payload type.
 *
 */
      RTCP_REPORTS_ET m_etPayloadType;

/**
 *
 * Attribute Name:  m_ulLength
 *
 * Type:            unsigned long
 *
 * Description:     The RTCP Report Length.
 *
 */
      unsigned long m_ulLength;

/**
 *
 * Attribute Name:  m_ulSSRC
 *
 * Type:            unsigned long
 *
 * Description:     This member shall store the SSRC ID of the associated
 *                  RTP connection.
 *
 */
      unsigned long m_ulSSRC;


};


#endif
