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
 * The listener interface for receiving text adjustment events.
 *
 * @author Robert J. Andreasen, Jr.
 */
public interface PTextListener
{
    /**
     * This listener method is invoked when the value of the text has changed.
     *
     * @param PTextEvent The event object describing the state change.
     */
    public void textValueChanged(PTextEvent e) ;
    

    /**
     * This listener method is invoked when the caret position is changed
     * <p><b>NOTE</b>: This is an extension to the AWT TextListener interface.
     *
     * @param PTextEvent The event object describing the state change.
     */
    public void caretPositionChanged(PTextEvent e) ;
    
}
