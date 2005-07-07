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
 
package org.sipfoundry.stapi.event ;

import org.sipfoundry.stapi.* ;


/**
 * PConnectionListenerAdapter implements the PConnectionListener interface and
 * adds a do-nothing convenience implementation.
 *
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PConnectionListenerAdapter implements PConnectionListener
{
    public void callCreated(PConnectionEvent event) { }
    public void callDestroyed(PConnectionEvent event) { }
    public void callHeld(PConnectionEvent event) { }
    public void callReleased(PConnectionEvent event) { }
    
    public void connectionTrying(PConnectionEvent event) { }
    public void connectionOutboundAlerting(PConnectionEvent event) { }
    public void connectionInboundAlerting(PConnectionEvent event) { }
    public void connectionConnected(PConnectionEvent event) { }
    public void connectionFailed(PConnectionEvent event) { } 
    public void connectionUnknown(PConnectionEvent event) { }
    public void connectionDisconnected(PConnectionEvent event) { }
}
