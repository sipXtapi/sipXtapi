//  
// Copyright (C) 2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _INCLUDED_RTPHEADER_H /* [ */
#define _INCLUDED_RTPHEADER_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsIntTypes.h"
#include "mp/MpTypes.h"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// DEFINES
#define RTP_V_MASK    0xC0  ///< Mask for Version bit in RtpHeader::vpxcc
#define RTP_V_SHIFT   6     ///< Shift for Version bit in RtpHeader::vpxcc
#define RTP_P_MASK    0x20  ///< Mask for Padding bit in RtpHeader::vpxcc
#define RTP_P_SHIFT   5     ///< Shift for Padding bit in RtpHeader::vpxcc
#define RTP_X_MASK    0x10  ///< Mask for Extension bit in RtpHeader::vpxcc
#define RTP_X_SHIFT   4     ///< Shift for Extension bit in RtpHeader::vpxcc
#define RTP_CC_MASK   0x0F  ///< Mask for CCSRC bits in RtpHeader::vpxcc
#define RTP_CC_SHIFT  0     ///< Shift for CCSRC bits in RtpHeader::vpxcc

#define RTP_M_MASK    0x80  ///< Mask for Marker bit in RtpHeader::mpt
#define RTP_M_SHIFT   7     ///< Shift for Marker bit in RtpHeader::mpt
#define RTP_PT_MASK   0x7F  ///< Mask for Payload Type bits in RtpHeader::mpt
#define RTP_PT_SHIFT  0     ///< Shift for Payload Type bits in RtpHeader::mpt

#define RTP_MAX_CSRCS 16    ///< Maximum number of CSRCs in RTP packet

/// RTP header as described in RFC 3550.
struct RtpHeader {
   uint8_t vpxcc;           ///< Version, Padding, Extension and CSRC Count bits.
   uint8_t mpt;             ///< Marker and Payload Type bits.
   RtpSeq seq;              ///< Sequence Number (Big Endian!)
   RtpTimestamp timestamp;  ///< Timestamp (Big Endian!)
   RtpSRC ssrc;             ///< SSRC (Big Endian!)
};


#endif /* ] _INCLUDED_RTPHEADER_H */
