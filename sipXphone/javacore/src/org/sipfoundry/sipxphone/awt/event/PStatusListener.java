/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/awt/event/PStatusListener.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sipxphone.awt.event;

/**
 * PStatusListener.java
 *
 * Listener to listen to status event when status gets opened,
 * closed or aborted.
 * 
 * Created: Tue Jun 05 13:21:13 2001
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */

public interface  PStatusListener {
    /**
     * this gets called when status frame gets opened( displayed ).
     */
    public void statusOpened( PStatusEvent event);
    
    /**
     * this gets called when status frame gets closed.
     */
    public void statusClosed( PStatusEvent event);
    
    /**
     * this gets called when status frame gets aborted.
     */
    public void statusAborted( PStatusEvent event);
}// PStatusListener

































