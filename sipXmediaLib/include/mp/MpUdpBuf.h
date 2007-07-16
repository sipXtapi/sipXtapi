//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _INCLUDED_MPUDPBUF_H /* [ */
#define _INCLUDED_MPUDPBUF_H

// SYSTEM INCLUDES
#ifdef __pingtel_on_posix__ // [
#  include <netinet/in.h>
#endif // __pigntel_on_posix__ ]

#ifdef WIN32
#  include <winsock2.h>
#endif

// APPLICATION INCLUDES
#include "mp/MpDataBuf.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

///  Buffer for raw UDP packet.
/**
*  This is only the header for UDP packet. It contain some UDP-specific
*  data and pointer to external data (cause it is based on MpDataBuf).
*/
struct MpUdpBuf : public MpDataBuf
{
    friend class MpUdpBufPtr;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static MpBufPool *smpDefaultPool; ///< Default pool for this type of buffer

/* ============================ CREATORS ================================== */
///@name Creators
//@{


//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

    /// Set size of packet data in bytes.
    /**
    * @see MpArrayBuf::setDataSize() for more details
    */
    bool setPacketSize(unsigned size) {return mpData->setDataSize(size);}

    /// Set time code for this packet
    void setTimecode(unsigned timecode) {mTimecode=timecode;}

    /// Set source/destination address for this packet
    void setIP(const in_addr &ip) {mIP=ip;}

    /// Set source/destination UDP port for this packet
    void setUdpPort(unsigned port) {mPort=port;}

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Get size of packet data in bytes.
    unsigned getPacketSize() const {return mpData->getDataSize();}

    /// Get maximum size of data this buffer can bear.
    unsigned getMaximumPacketSize() const {return mpData->getMaxDataSize();}

    /// Get time code for this packet
    unsigned getTimecode() const {return mTimecode;}

    /// Get source/destination address for this packet
    const in_addr &getIP() const {return mIP;}

    /// Get source/destination UDP port for this packet
    unsigned getUdpPort() const {return mPort;}

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{


//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    unsigned   mTimecode;    ///< Time when this packet is received. It may be 
                             ///< used in jitter buffer or somewhere else.
    in_addr    mIP;          ///< Source/destination IP-addres of this packet
    unsigned   mPort;        ///< Source/destination UDP-port of this packet

    /// This is called in place of constructor.
    void init();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /// Disable copy (and other) constructor.
    MpUdpBuf(const MpBuf &);
    /**<
    * This struct will be initialized by init() member.
    */

    /// Disable assignment operator.
    MpUdpBuf &operator=(const MpBuf &);
    /**<
    * Buffers may be copied. But do we need this?
    */
};

///  Smart pointer to MpUdpBuf.
/**
*  You should only use this smart pointer, not #MpUdpBuf* itself.
*  The goal of this smart pointer is to care about reference counter and
*  buffer deallocation.
*/
class MpUdpBufPtr : public MpDataBufPtr {

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

    /// Default constructor - construct invalid pointer.
    MPBUF_DEFAULT_CONSTRUCTOR(MpUdpBuf)

    /// This constructor owns MpBuf object.
    MPBUFDATA_FROM_BASE_CONSTRUCTOR(MpUdpBuf, MP_BUF_UDP, MpDataBuf)

    /// Copy object from base type with type check.
    MPBUF_TYPECHECKED_COPY(MpUdpBuf, MP_BUF_UDP, MpDataBuf)

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{


//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Return pointer to MpUdpBuf.
    MPBUF_MEMBER_ACCESS_OPERATOR(MpUdpBuf)

    /// Return readonly pointer to MpUdpBuf.
    MPBUF_CONST_MEMBER_ACCESS_OPERATOR(MpUdpBuf)

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

#endif /* ] _INCLUDED_MPUDPBUF_H */
