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


    // Includes
#include "rtcp/RTCPHeader.h"
#ifdef INCLUDE_RTCP /* [ */

#ifdef __pingtel_on_posix__
#include <netinet/in.h>
#endif

#ifdef WIN32
#   include <winsock2.h>
#endif

#include "os/OsSysLog.h"

    // Constants
const int   PAD_MASK            = 0x20;
const int   VERSION_MASK        = 0xC0;

const int   PAD_SHIFT           = 5;
const int   VERSION_SHIFT       = 6;

/**
 *
 * Method Name:  CRTCPHeader() - Constructor
 *
 *
 * Inputs:    ssrc_t ulSSRC
 *                              - The the IDentifier for this source
 *            unsigned long ulVersion
 *                              - Version of the RFC Standard being followed
 *            unsigned long ulPayload
 *                              - The Payload type associated with this report
 *
 * Outputs:   None
 *
 * Returns:   None
 *
 * Description:  The CRTCPHeader is an abstract class that is initialized by a
 *               derived object at construction time.
 *
 * Usage Notes:
 *
 */
CRTCPHeader::CRTCPHeader(ssrc_t ulSSRC, RTCP_REPORTS_ET etPayloadType,
                         unsigned long ulVersion)
            : m_ulPadding(FALSE), m_ulCount(0), m_ulLength(0)
{

    // Assign initial values to attributes
    m_ulSSRC        = ulSSRC;
    m_ulVersion     = ulVersion;
    m_etPayloadType = etPayloadType;

}

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
 *               acquired over the course of runtime.
 *
 * Usage Notes:
 *
 *
 */
CRTCPHeader::~CRTCPHeader(void)
{
// Our reference count must have gone to 0 to get here.  We have not allocated
// any memory so we shall now go quietly into that good night!
}


/**
 *
 * Method Name: VetPacket -- static
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


int CRTCPHeader::VetPacket(unsigned char* buffer, int bufferLen)
{
    int okLen = 0;
    int nRemain = -1;
    int rt;
    uint16_t tShort;
    int nOctets;
    int originalLen = bufferLen;

    // Special rule for LifeSize box and its non-compliant APP packets...
    if ((63 == bufferLen) && ('L' == buffer[44]) && ('S' == buffer[45])) {
        // Silently fix it... we know it is happening, don't nag
        buffer[bufferLen++] = 0;
    }
    if (bufferLen > 7) {
        if (0 != (bufferLen % 4)) {
            OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPSource::VetPacket: packet length, %d, is not a multiple of 4; padding and adjusting", bufferLen);
            while (0 != (bufferLen % 4)) buffer[bufferLen++] = 0;
        }
        nRemain = bufferLen;

        while (nRemain > 7) {
           // Is the first byte reasonable (we only know that the top 2 bits should be 0b10)
            if (0x80 != (buffer[okLen] & 0xc0)) break;
            // OsSysLog::add(FAC_MP, PRI_DEBUG, "CRTCPSource::VetPacket: header at offset %d, RTCP version OK", okLen);
           // Now, see if the report type is one of SR, RR, SDES, BYE, or APP (200..204, resp)
            rt = buffer[okLen+1];
            if ((rt < 200) || (rt > 204)) {
               // Only warn if not, if everything else is OK, let the parser deal with it.
                OsSysLog::add(FAC_MP, PRI_WARNING, "CRTCPSource::VetPacket: report type, %d, is not in one defined in RFC-3550", rt);
            }
           // Now, check the length.  It is
            tShort = *((unsigned short*)(buffer+okLen+2));
            nOctets = (ntohs(tShort) + 1) * 4;
            // OsSysLog::add(FAC_MP, PRI_DEBUG, "CRTCPSource::VetPacket: tShort=%d, nOctets=%d, nRemain=%d", tShort, nOctets, nRemain);
            if (nRemain < nOctets) break;
            nRemain -= nOctets;
            okLen += nOctets;
        }
    }
    if (nRemain > 0) {
        OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPSource::VetPacket: packet failed sanity check. Original len=%d, nRemain=%d", originalLen, nRemain);
        UtlString buf;
        unsigned char *tp = buffer;
        int tl = originalLen;
        while(tl > 0) {
            buf.appendFormat(" 0x%02X,", *tp++);
            tl--;
        }
        buf.strip(UtlString::trailing, ',');
        OsSysLog::add(FAC_MP, PRI_ERR, "RTCP Packet:    %s", buf.data());
    }
    return okLen;
}


/**
 *
 * Method Name:  GetHeaderLength
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long  - the size of the RTCP Header
 *
 * Description: Returns the size of the RTCP Header that preceeds the payload.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTCPHeader::GetHeaderLength(void)
{

    // Load the argument passed with the header length
    return(HEADER_LENGTH);

}

/**
 *
 * Method Name:  GetSSRC
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     ssrc_t  - Return the SSRC ID
 *
 * Description: Retrieves the SSRC attribute stored within the object.
 *
 * Usage Notes:
 *
 *
 */
ssrc_t CRTCPHeader::GetSSRC(void)
{

    // Return the SSRC
    return(m_ulSSRC);

}


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
 * Description: Returns the protocol version number from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTCPHeader::GetVersion(void)
{

    // Return Version Number
    return(m_ulVersion);

}

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
 * Description: Returns the padding flag from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTCPHeader::GetPadding(void)
{

    // Return Padding Flag
    return(m_ulPadding);

}


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
unsigned long CRTCPHeader::GetReportCount(void)
{

    // Return Report Count
    return(m_ulCount);

}


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
unsigned long CRTCPHeader::GetReportLength(void)
{

    // Return Report Length
    if (0 == m_ulLength) {
        OsSysLog::add(FAC_MP, PRI_ERR, "CRTCPHeader::GetReportLength: m_ulLength is %lu, returning %lu (payload is %d)", m_ulLength, (m_ulLength ? m_ulLength + sizeof(uint32_t) : m_ulLength), GetPayload());
    }
    return(m_ulLength ? m_ulLength + sizeof(uint32_t) : m_ulLength);

}


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
 * Description: Returns the payload type value from the RTCP packet.
 *
 * Usage Notes:
 *
 *
 */
RTCP_REPORTS_ET CRTCPHeader::GetPayload(void)
{

    // Return Payload Type
    return(m_etPayloadType);

}

/**
 *
 * Method Name:  IsOurSSRC
 *
 *
 * Inputs:      None
 *
 * Outputs:     ssrc_t ulSSRC  - SSRC ID
 *
 * Returns:     boolean - TRUE => match
 *
 * Description: Compares the SSRC ID passed to that stored as an attribute
 *              within this object instance.  Will return either True or False
 *              based on the match.
 *
 * Usage Notes:
 *
 *
 */
bool CRTCPHeader::IsOurSSRC(ssrc_t ulSSRC)
{

    // Compare the SSRC passed to the one that we have stored.
    return(ulSSRC == m_ulSSRC);

}


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
void CRTCPHeader::SetSSRC(ssrc_t ulSSRC)
{

    // Store the modified SSRC as an internal attribute
    m_ulSSRC    = ulSSRC;

}

/**
 *
 * Method Name:  FormatRTCPHeader
 *
 *
 * Inputs:      unsigned long ulPadding         - Padding used
 *              unsigned long ulCount           - Report Count
 *              unsigned long ulReportLength    - Report Length
 *
 * Outputs:     unsigned char *puchRTCPBuffer
 *                               - Buffer used to store the RTCP Report Header
 *
 * Returns:     unsigned long  - number of octets written into the buffer.
 *
 * Description: Constructs an RTCP Report report using information stored and
 *              passed by the caller.
 *
 * Usage Notes: A buffer of sufficient size should be allocated and passed to
 *              this formatting method.
 *
 *
 */
unsigned long CRTCPHeader::FormatRTCPHeader(unsigned char *puchRTCPBuffer,
                                            unsigned long ulPadding,
                                            unsigned long ulCount,
                                            unsigned long ulReportLength)
{

    unsigned char *puchRTCPHeader = puchRTCPBuffer;
    unsigned short l;
    unsigned long ret;

    // Report Count goes into the bits 4 - 8 of the first octet
    m_ulCount = ulCount;
    *puchRTCPHeader  = (unsigned char)ulCount;

    // Padding flag goes into the third bit of the first octet
    m_ulPadding = ulPadding;
    *puchRTCPHeader |= (unsigned char)((ulPadding << PAD_SHIFT) & PAD_MASK);

    // Version # goes into the first 2 bits of the first octet
    *puchRTCPHeader++ |=
               (unsigned char)((m_ulVersion << VERSION_SHIFT) & VERSION_MASK);

    // Payload Type goes into the second octet
    *puchRTCPHeader++ = (unsigned char)m_etPayloadType;

    // RTCP Report length goes into the third and fourth octet.  This length
    //  is expressed in long words.
    m_ulLength = ulReportLength;
    l = ((((unsigned short)ulReportLength) / sizeof(uint32_t)) - 1);
    *((unsigned short *)puchRTCPHeader) = htons(l);;
    puchRTCPHeader += sizeof(short);

    // SSRC goes into the next 4 octet
    *((ssrc_t *)puchRTCPHeader) = htonl(m_ulSSRC);

    ret = (puchRTCPBuffer - puchRTCPBuffer);
        // OsSysLog::add(FAC_MP, PRI_DEBUG, "CRTCPHeader::FormatRTCPHeader(%p, %ld, %ld, %ld), l=%d, ret=%ld, SSRC=0x%08X", puchRTCPBuffer, ulPadding, ulCount, ulReportLength, l, ret, GetSSRC());
    return ret;

}


/**
 *
 * Method Name:  ParseRTCPHeader
 *
 *
 * Inputs:   unsigned char *puchRTCPBuffer - Buffer containing the RTCP Report
 *
 * Outputs:  None
 *
 * Returns:  bool
 *
 * Description:  Extracts the header contents of an RTCP report using the
 *               buffer passed in by the caller.  The header will be validated
 *               to determine whether it has an appropriate version, payload
 *               type, and SSRC for this object.
 *
 * Usage Notes:
 *
 *
 */
bool CRTCPHeader::ParseRTCPHeader(unsigned char *puchRTCPBuffer)
{

    unsigned char *puchRTCPHeader = puchRTCPBuffer;

    // Extract Report Count
    m_ulCount = *puchRTCPHeader & COUNT_MASK;

    // Extract Padding
    m_ulPadding = ((*puchRTCPHeader & PAD_MASK) >> PAD_SHIFT);

    // Check for valid Version #
    if (((*puchRTCPHeader & VERSION_MASK) >> VERSION_SHIFT) != (char)m_ulVersion)
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG, "CRTCPHeader::ParseRTCPHeader - Invalid Version: %d", ((*puchRTCPHeader & VERSION_MASK) >> VERSION_SHIFT));
        return(FALSE);
    }
    puchRTCPHeader++;

    // Check for valid Payload Type
    if(*puchRTCPHeader != (unsigned char)m_etPayloadType)
    {
        OsSysLog::add(FAC_MP, PRI_DEBUG, "CRTCPHeader::ParseRTCPHeader -  Invalid Payload Type: %d", *puchRTCPHeader);
        return(FALSE);
    }
    puchRTCPHeader++;

    // Extract RTCP Report length and convert from word count to byte count
    m_ulLength = ntohs(*((unsigned short *)puchRTCPHeader)) + 1;
    m_ulLength *= sizeof(uint32_t);
    puchRTCPHeader += sizeof(short);

    // Assign SSRC if one hadn't previously existed
    if(m_ulSSRC == 0)
        m_ulSSRC = ntohl(*((ssrc_t *)puchRTCPHeader));

    // Check SSRC to be sure that the one received corresponds with the one
    //  previously established.
    else if(ntohl(*((ssrc_t *)puchRTCPHeader)) != m_ulSSRC)
    {
#if RTCP_DEBUG /* [ */
        if(bPingtelDebug)
        {
            osPrintf(">>>>> CRTCPHeader::ParseRTCPHeader() -"
                                                 " SSRC has Changed <<<<<\n");
        }
#endif /* RTCP_DEBUG ] */
        m_ulSSRC = ntohl(*((ssrc_t *)puchRTCPHeader));

    }
    puchRTCPHeader += sizeof(ssrc_t);

    return(TRUE);

}

#endif /* INCLUDE_RTCP ] */
