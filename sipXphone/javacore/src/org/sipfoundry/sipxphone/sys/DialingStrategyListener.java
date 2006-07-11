/*
 * $Id$
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.sys ;

import org.sipfoundry.stapi.* ;

/**
 * Any dialing mechanism that supports a dialing strategy must also support
 * an external dialer listener.  This listener will inform the original
 * controller (form/application that kicks off the dialing process) when
 * dialing attempt begins or when the dialing process is aborted.
 *
 * @author Robert J. Andreasen, Jr.
 */
public interface DialingStrategyListener
{
    /**
     * dialInitiated is invoked when an external dialer has attempted to dial
     * or connect to a target.  There is no guarentee that the connect will
     * succeed or fail, or even if that connection is still trying when the
     * listener is notified.
     *
     * @param call that dialing was initiated on/for
     * @param address Address that we are dialing     
     */
    public void dialingInitiated(PCall call, PAddress address) ;


    /**
     * dialAborted is invoked when an external dialer has aborted a dialing
     * attempt.
     *
     * @param call that dialing was initiated on/for     
     */
    public void dialingAborted(PCall call) ;
}

