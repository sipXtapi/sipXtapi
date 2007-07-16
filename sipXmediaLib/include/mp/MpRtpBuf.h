//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _INCLUDED_MPRTPBUF_H /* [ */
#define _INCLUDED_MPRTPBUF_H

// SYSTEM INCLUDES
#ifdef __pingtel_on_posix__ // [
#  include <netinet/in.h>
#endif // __pigntel_on_posix__ ]

#ifdef WIN32
#   include <winsock2.h>
#endif

// APPLICATION INCLUDES
#include "mp/MpDataBuf.h"
#include "mp/MpTypes.h"
#include "os/OsIntTypes.h"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef uint16_t RtpSeq;        ///< RTP sequence number
typedef uint32_t RtpTimestamp;  ///< RTP timestamp
typedef uint32_t RtpSRC;        ///< RTP SSRC or CSRC identifier

/// RTP header as described in RFC 3550.
struct RtpHeader {
   uint8_t vpxcc;           ///< Version, Padding, Extension and CSRC Count bits.
   uint8_t mpt;             ///< Marker and Payload Type bits.
   RtpSeq seq;              ///< Sequence Number (Big Endian!)
   RtpTimestamp timestamp;  ///< Timestamp (Big Endian!)
   RtpSRC ssrc;             ///< SSRC (Big Endian!)
};

// DEFINES
#define RTP_V_MASK   0xC0   ///< Mask for Version bit in RtpHeader::vpxcc
#define RTP_V_SHIFT  6      ///< Shift for Version bit in RtpHeader::vpxcc
#define RTP_P_MASK   0x20   ///< Mask for Padding bit in RtpHeader::vpxcc
#define RTP_P_SHIFT  5      ///< Shift for Padding bit in RtpHeader::vpxcc
#define RTP_X_MASK   0x10   ///< Mask for Extension bit in RtpHeader::vpxcc
#define RTP_X_SHIFT  4      ///< Shift for Extension bit in RtpHeader::vpxcc
#define RTP_CC_MASK  0x0F   ///< Mask for CCSRC bits in RtpHeader::vpxcc
#define RTP_CC_SHIFT 0      ///< Shift for CCSRC bits in RtpHeader::vpxcc

#define RTP_M_MASK   0x80   ///< Mask for Marker bit in RtpHeader::mpt
#define RTP_M_SHIFT  7      ///< Shift for Marker bit in RtpHeader::mpt
#define RTP_PT_MASK  0x7F   ///< Mask for Payload Type bits in RtpHeader::mpt
#define RTP_PT_SHIFT 0      ///< Shift for Payload Type bits in RtpHeader::mpt

#define RTP_MAX_CSRCS       16      ///< Maximum number of CSRCs in RTP packet

///  Buffer for RTP packet data.
/**
*  This is only the header for RTP data. It contain some RTP-related
*  parameters and pointer to external data (cause it is based on MpDataBuf).
*/
struct MpRtpBuf : public MpDataBuf
{
    friend class MpRtpBufPtr;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    /// Default pool for MpDataBuf objects.
    static MpBufPool *smpDefaultPool;

/* ============================ CREATORS ================================== */
///@name Creators
//@{


//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

    /// Set current number of samples in payload data.
    /**
    * @see MpArrayBuf::setDataSize() for more details
    */
    bool setPayloadSize(unsigned samplesNum)
    {return mpData->setDataSize(samplesNum);}

    /// Set RTP version of this packet.
    /**
    *  @see See getRtpVersion() for details.
    */
    void setRtpVersion(uint8_t version)
    { mRtpHeader.vpxcc = ( (version<<RTP_V_SHIFT)&RTP_V_MASK)
                         | (mRtpHeader.vpxcc&(~RTP_V_MASK));}

    /// Set padding bit it RTP header.
    /**
    *  @see See isRtpPadding() for details.
    */
    void enableRtpPadding()
    { mRtpHeader.vpxcc |= 1<<RTP_P_SHIFT;}

    /// Clear padding bit it RTP header.
    /**
    *  @see See isRtpPadding() for details.
    */
    void disableRtpPadding()
    { mRtpHeader.vpxcc &= ~RTP_P_MASK;}

    /// Set extension bit it RTP header.
    /**
    *  @see See isRtpExtension() for details.
    */
    void enableRtpExtension()
    { mRtpHeader.vpxcc |= 1<<RTP_X_SHIFT;}

    /// Clear extension bit it RTP header.
    /**
    *  @see See isRtpExtension() for details.
    */
    void disableRtpExtension()
    { mRtpHeader.vpxcc &= ~RTP_X_MASK;}

    /// Set CSRC Count for this packet.
    /**
    *  @see See getRtpCSRCCount() for details.
    */
    void setRtpCSRCCount(uint8_t csrcCount)
    { mRtpHeader.vpxcc = ( (csrcCount<<RTP_CC_SHIFT)&RTP_CC_MASK)
                         | (mRtpHeader.vpxcc&(~RTP_CC_MASK));}

    /// Set marker bit it RTP header.
    /**
    *  @see See isRtpMarker() for details.
    */
    void enableRtpMarker()
    { mRtpHeader.mpt |= 1<<RTP_M_SHIFT;}


    /// Clear marker bit it RTP header.
    /**
    *  @see See isRtpMarker() for details.
    */
    void disableRtpMarker()
    { mRtpHeader.mpt &= ~RTP_M_MASK;}

    /// Set Payload Type of this packet.
    /**
    *  @see See getRtpPayloadType() for details.
    */
    void setRtpPayloadType(uint8_t type)
    { mRtpHeader.mpt = ( (type<<RTP_PT_SHIFT)&RTP_PT_MASK)
                       | (mRtpHeader.mpt&(~RTP_PT_MASK));}

    /// Set Sequence Number of this packet.
    /**
    *  @see See getRtpSequenceNumber() for details.
    */
    void setRtpSequenceNumber(RtpSeq sequenceNumber)
    { mRtpHeader.seq = htons(sequenceNumber);}

    /// Set Timestamp of this packet.
    /**
    *  @see See getRtpTimestamp() for details.
    */
    void setRtpTimestamp(RtpTimestamp timestamp)
    { mRtpHeader.timestamp = htonl(timestamp);}

    /// Set SSRC of this packet.
    /**
    *  @see See getRtpSSRC() for details.
    */
    void setRtpSSRC(RtpSRC ssrc)
    { mRtpHeader.ssrc = htonl(ssrc);}

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Get current number of samples in payload data.
    unsigned getPayloadSize() const {return mpData->getDataSize();}

    /// Get direct access to RtpHeader structure.
    RtpHeader &getRtpHeader() {return mRtpHeader;}

    /// Get readonly direct access to RtpHeader structure.
    const RtpHeader &getRtpHeader() const {return mRtpHeader;}

    /// Get direct access to RtpHeader structure.
    RtpSRC *getRtpCSRCs() {return mRtpCSRCs;}

    /// Get readonly direct access to RtpHeader structure.
    const RtpSRC *getRtpCSRCs() const {return mRtpCSRCs;}

    /// Get RTP version of this packet. Should be equal to 2.
    /**
    *  @note From RFC 3550:
    *  <i>"This field identifies the version of RTP.  The version defined by
    *  this specification is two (2).  (The value 1 is used by the first
    *  draft version of RTP and the value 0 is used by the protocol
    *  initially implemented in the "vat" audio tool.)"</i>
    */
    uint8_t getRtpVersion() const
    {return (mRtpHeader.vpxcc&RTP_V_MASK) >> RTP_V_SHIFT;}

    /// Get CSRC Count for this packet.
    /**
    *  @note From RFC 3550:
    *  <i>"The CSRC count contains the number of CSRC identifiers that follow
    *  the fixed header. "</i>
    */
    uint8_t getRtpCSRCCount() const {return mRtpHeader.vpxcc & RTP_CC_MASK;}

    /// Get Payload Type of this packet.
    /**
    *  @note From RFC 3550:
    *  <i>"This field identifies the format of the RTP payload and determines
    *  its interpretation by the application.  A profile MAY specify a
    *  default static mapping of payload type codes to payload formats.
    *  Additional payload type codes MAY be defined dynamically through
    *  non-RTP means.  A set of default mappings for
    *  audio and video is specified in the companion RFC 3551.  An
    *  RTP source MAY change the payload type during a session, but this
    *  field SHOULD NOT be used for multiplexing separate media streams.
    *  <br>
    *  A receiver MUST ignore packets with payload types that it does not
    *  understand."</i>
    */
    uint8_t getRtpPayloadType() const
    {return (mRtpHeader.mpt&RTP_PT_MASK) >> RTP_PT_SHIFT;}

    /// Get Sequence Number of this packet.
    /**
    *  @note From RFC 3550:
    *  <i>"The sequence number increments by one for each RTP data packet
    *  sent, and may be used by the receiver to detect packet loss and to
    *  restore packet sequence.  The initial value of the sequence number
    *  SHOULD be random (unpredictable) to make known-plaintext attacks
    *  on encryption more difficult, even if the source itself does not
    *  encrypt, because the
    *  packets may flow through a translator that does."</i>
    */
    RtpSeq getRtpSequenceNumber() const {return ntohs(mRtpHeader.seq);}

    /// Get Timestamp of this packet.
    /**
    *  @note From RFC 3550:
    *  <i>"The timestamp reflects the sampling instant of the first octet in
    *  the RTP data packet.  The sampling instant MUST be derived from a
    *  clock that increments monotonically and linearly in time to allow
    *  synchronization and jitter calculations.  The
    *  resolution of the clock MUST be sufficient for the desired
    *  synchronization accuracy and for measuring packet arrival jitter
    *  (one tick per video frame is typically not sufficient)."</i>
    */
    RtpTimestamp getRtpTimestamp() const {return ntohl(mRtpHeader.timestamp);}

    /// Get SSRC of this packet.
    /**
    *  @note From RFC 3550:
    *  <i>"The SSRC field identifies the synchronization source.  This
    *  identifier SHOULD be chosen randomly, with the intent that no two
    *  synchronization sources within the same RTP session will have the
    *  same SSRC identifier.  Although the
    *  probability of multiple sources choosing the same identifier is
    *  low, all RTP implementations must be prepared to detect and
    *  resolve collisions."</i>
    */
    RtpSRC getRtpSSRC() const {return ntohl(mRtpHeader.ssrc);}

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

    /// Check padding bit it RTP header.
    /**
    *  @note From RFC 3550:
    *  <i>"If the padding bit is set, the packet contains one or more
    *  additional padding octets at the end which are not part of the
    *  payload.  The last octet of the padding contains a count of how
    *  many padding octets should be ignored, including itself.  Padding
    *  may be needed by some encryption algorithms with fixed block sizes
    *  or for carrying several RTP packets in a lower-layer protocol data
    *  unit. "</i>
    */
    bool isRtpPadding() const
    {return (mRtpHeader.vpxcc&RTP_P_MASK) == (1<<RTP_P_SHIFT);}

    /// Check extension bit it RTP header.
    /**
    *  @note From RFC 3550:
    *  <i>"If the extension bit is set, the fixed header MUST be followed by
    *  exactly one header extension."</i>
    */
    bool isRtpExtension() const
    {return (mRtpHeader.vpxcc&RTP_X_MASK) == (1<<RTP_X_SHIFT);}

    /// Check marker bit it RTP header.
    /**
    *  @note From RFC 3550:
    *  <i>"The interpretation of the marker is defined by a profile.  It is
    *  intended to allow significant events such as frame boundaries to
    *  be marked in the packet stream.  A profile MAY define additional
    *  marker bits or specify that there is no marker bit by changing the
    *  number of bits in the payload type field"</i>
    */
    bool isRtpMarker() const
    {return (mRtpHeader.mpt&RTP_M_MASK) == (1<<RTP_M_SHIFT);}

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    RtpHeader  mRtpHeader;   ///< Fixed header of RTP packet. It is contained as
                             ///< is and functions to access its components are
                             ///< provided.
    RtpSRC mRtpCSRCs[RTP_MAX_CSRCS]; ///< CSRCs list of RTP packet.

    /// This is called in place of constructor.
    void init();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /// Disable copy (and other) constructor.
    MpRtpBuf(const MpBuf &);
    /**<
    * This struct will be initialized by init() member.
    */

    /// Disable assignment operator.
    MpRtpBuf &operator=(const MpBuf &);
    /**<
    * Buffers may be copied. But do we need this?
    */
};

///  Smart pointer to MpRtpBuf.
/**
*  You should only use this smart pointer, not #MpRtpBuf* itself.
*  The goal of this smart pointer is to care about reference counter and
*  buffer deallocation.
*/
class MpRtpBufPtr : public MpDataBufPtr {

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

    /// Default constructor - construct invalid pointer.
    MPBUF_DEFAULT_CONSTRUCTOR(MpRtpBuf)

    /// This constructor owns MpBuf object.
    MPBUFDATA_FROM_BASE_CONSTRUCTOR(MpRtpBuf, MP_BUF_RTP, MpDataBuf)

    /// Copy object from base type with type check.
    MPBUF_TYPECHECKED_COPY(MpRtpBuf, MP_BUF_RTP, MpDataBuf)

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{


//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Return pointer to MpRtpBuf.
    MPBUF_MEMBER_ACCESS_OPERATOR(MpRtpBuf)

    /// Return readonly pointer to MpRtpBuf.
    MPBUF_CONST_MEMBER_ACCESS_OPERATOR(MpRtpBuf)

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif /* ] _INCLUDED_MPRTPBUF_H */
