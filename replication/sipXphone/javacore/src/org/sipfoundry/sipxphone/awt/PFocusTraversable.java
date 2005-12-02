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


package org.sipfoundry.sipxphone.awt ;

import org.sipfoundry.sipxphone.awt.event.* ;

/**
 * This interface identifies components that are focus traversable (requires
 * the equivalent of keyboard input).
 *
 * @author Robert J. Andreasen, Jr.
 */
public interface PFocusTraversable 
{
    /**
     * Set the focus state of this a traversable component.  A component
     * is required to have focus before receiving keyboard and/or button 
     * events.
     *
     * @param bSet boolean true to set focus otherwise false to give it up.
     */
    public void setFocus(boolean bSet) ;
    
    
    /**
     * Add a focus listener to this element. Focus listeners receive 
     * notification whenever this component gains or loses input focus.  
     * If the specified listener has already been added, the add should 
     * be ignored.
     *
     * @param listener The PFocusListener that will receive focus change
     *        notifications.
     *
     * @see org.sipfoundry.sipxphone.awt.event.PFocusListener
     */
    public void addFocusListener(PFocusListener listener) ;

    
    /**
     * Remove a focus listener from this element.  If the specified focus 
     * listener is not listening, the request should be ignored.
     *
     * @param listener The PFocusListenre that should stop receiving focus change
     *        notifications.
     *
     * @see org.sipfoundry.sipxphone.awt.event.PFocusListener
     */    
    public void removeFocusListener(PFocusListener listener) ;
}

    
    
