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
#ifndef _IRTPDispatch_h
#define _IRTPDispatch_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "IRTPHeader.h"
#include "IBaseClass.h"

/**
 *
 * Interface Name:  IRTPDispatch
 *
 * Inheritance:     None
 *
 *
 * Description:     The IRTPDispatch interface allows a RTP Source object to
 *                  forward an RTP packet header to a recipient object as they
 *                  are received from the a Network Source.
 *
 * Notes:
 *
 */
interface IRTPDispatch : public IBaseClass
 {

//  Public Methods

public:

/**
 *
 * Method Name:  ForwardRTPHeader
 *
 *
 * Inputs:      CRTPHeader *poRTPHeader -
 *                             RTP Packet Header received from RTP Source
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Takes an instance of an RTP Header object received from the RTP
 *              Source and dispatches it to the CSenderReport object for
 *              analysis and update of receiver report statistics associated
 *              with that RTP source.
 *
 * Usage Notes: For the time being, it has been decided to process the RTP
 *              Header on the thread of the Network interface rather than
 *              waiting for the RTC Manager to process it.  If found to affect
 *              network throughput, the RTP Headers shall be queued for batch
 *              processing by the RTC Manager.
 *
 *
 */
    virtual void ForwardRTPHeader(IRTPHeader *piRTPHeader) = 0;

};

#endif
