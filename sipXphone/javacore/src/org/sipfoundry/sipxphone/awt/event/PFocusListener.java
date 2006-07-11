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
 
package org.sipfoundry.sipxphone.awt.event ;


/**
 * The listener interface for receiving focus events when text input/focus
 * is changed from one component to another.
 *
 * @author Robert J. Andreasen, Jr.
 */ 
public interface PFocusListener
{
    /**
     * Invoked when a component gains input focus
     */
    public void focusGained(PFocusEvent e) ;


    /**
     * Invoked when a component loses input focus
     */
    public void focusLost(PFocusEvent e) ;
}