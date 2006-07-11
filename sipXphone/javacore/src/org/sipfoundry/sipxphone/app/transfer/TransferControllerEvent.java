/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/app/transfer/TransferControllerEvent.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.app.transfer ;

import org.sipfoundry.stapi.* ;


/**
 * The transfer controller event is delievered as part of the
 * TransferControllerListener whenever there is a major change of state in the
 * transfer progress.
 *
 * @author Robert J. Andreasen
 */
public class TransferControllerEvent
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private PAddress m_addressTransferee ;
    private PCall    m_callTransferee ;
    private PAddress m_addressTarget ;
    private PCall    m_callTarget ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * Constructs a TransferControllerEvent with all of the required
     * parameters.
     *
     * @param addressTransferee The address of the transferee
     * @param callTransferee The call with the TC and transferee
     * @param addressTarget The address of the transfer target
     * @param callTarget The call with the TC and the transferee
     */
    public TransferControllerEvent(PAddress addressTransferee,
                                   PCall    callTransferee,
                                   PAddress addressTarget,
                                   PCall    callTarget)
    {
        m_addressTransferee = addressTransferee ;
        m_callTransferee = callTransferee ;
        m_addressTarget = addressTarget ;
        m_callTarget = callTarget ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Get the address of the transferee
     *
     * @return address of the transferee
     */
    public PAddress getTransfereeAddress()
    {
        return m_addressTransferee ;
    }


    /**
     * Get the address of the transfere target
     *
     * @return address of the transfer target
     */
    public PAddress getTargetAddress()
    {
        return m_addressTarget ;
    }


    /**
     * Get the call between the transferee and the transfer controller
     *
     * @return call between transferee and transfer controller
     */
    public PCall getTransfereeCall()
    {
        return m_callTransferee ;
    }


    /**
     * Get the call between the transferee and the transfer target
     *
     * @return call between transferee and transfer target
     */
    public PCall getTargetCall()
    {
        return m_callTarget ;
    }
}
