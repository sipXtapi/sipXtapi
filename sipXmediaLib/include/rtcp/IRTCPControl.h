//  
// Copyright (C) 2022 SIP Spectrum, Inc.  All rights reserved.
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
#ifndef _IRTCPControl_h
#define _IRTCPControl_h

#include "rtcp/RtcpConfig.h"

// Include
#include "IBaseClass.h"
#include "IRTCPRegister.h"
#include "IRTCPSession.h"

/**
 *
 * Interface Name:  IRTCPControl
 *
 * Inheritance:     None
 *
 *
 * Description:     The IRTCPControl interface allows the comsumers to control
 *                  the creation and destruction of RTCP Sessions.  One RTCP
 *                  Session shall exist per call and shall be responsible for
 *                  coordinating one or more RTCP Connection engaged in a call.
 *                  The IRTCPControl interface also inherits the IRTCPRegister
 *                  interface, allowing a consumer to enroll and unenroll for
 *                  RTCP events.
 *
 * Notes:
 *
 */


interface IRTCPControl : public IRTCPRegister     // Interface exposed for
                                                  // event registration

 {

//  Public Methods
public:

/**
 *
 * Method Name: CreateSession
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  IRTCPSession *
 *             Returns an interface for use in controlling an RTCP Session
 *
 * Description: The CreateSession shall manage the instantiation of RTCP
 *              Session objects used to report and track the identity and
 *              performance of an RTP connections active within a call.
 *
 *
 * Usage Notes: An RTCPSession object shall be create per call.
 *
 *
 */
    virtual IRTCPSession* CreateSession() = 0;


/**
 *
 * Method Name: TerminateSession
 *
 *
 * Inputs:   IRTCPSession *piRTCPSession
 *             Interface used as a unqiue handle for identifying session
 *
 * Outputs:  None
 *
 * Returns:  boolean

 *
 * Description: The TerminateSession() method shall manage the termination of
 *              and RTCP session.  This shall include the graceful release of
 *              all associated objects as well as the deallocation of all
 *              resources associated with each contained RTCP connection.
 *
 *
 * Usage Notes:
 *
 *
 */
    virtual bool TerminateSession(IRTCPSession *piRTCPSession) = 0;


/**
 *
 * Method Name:  GenerateUniqueLocalSSRC()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     ssrc_t   - The newly generated local SSRC
 *
 * Description: Generate a new random local SSRC, ensuring that it doesn't
 *              conflict with any other currently known local or remote SSRCs.
 *
 * Usage Notes:
 *
 */
    virtual ssrc_t GenerateUniqueLocalSSRC() = 0;

};

#endif

