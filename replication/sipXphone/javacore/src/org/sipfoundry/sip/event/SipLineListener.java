/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/sip/event/SipLineListener.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sip.event;

import org.sipfoundry.sip.* ;

public interface SipLineListener
{
    public void lineEnabled(SipLineEvent event) ;

    public void lineFailed(SipLineEvent event) ;

    public void lineTimeout(SipLineEvent event) ;

    public void lineAdded(SipLineEvent event) ;

    public void lineDeleted(SipLineEvent event) ;

    public void lineChanged(SipLineEvent event) ;

    public void lineDefault(SipLineEvent event) ;
}
