/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/sip/event/NewSipMessageListener.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sip.event ;

import org.sipfoundry.sip.* ;


/**
 * This listener interface is implemented by objects interested in receiving
 * notifications of new Sip messages.  Listeners can be added from the 
 * SipUserAgent.
 *
 * @see SipUserAgent
 * @see NewSipMessageEvent
 *
 * @author Robert J. Andreasen, Jr.
 */
public interface NewSipMessageListener
{
    /**
     * Listener call back method that indicates a new SIP message is 
     * available for inspection.
     *
     * @param event A NewSipMessageEvent object containing the actual Sip 
     *        message
     */
    public void newMessage(NewSipMessageEvent event) ;
}
