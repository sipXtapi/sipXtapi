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
 * The listener interface for receiving item events.
 *
 * @author Robert J. Andreasen, Jr.
 */
public interface PItemListener
{
    /**
     * The listener method that is invoked when an item's state is changed.
     * You should look at the PItemEvent to determine which item 
     * changed and that changed item's new state.
     *
     * @param PItemEvent The event object describing the item's state change.
     * @see org.sipfoundry.sipxphone.awt.event.PItemEvent     
     */    
    public void itemStateChanged(PItemEvent e) ;    
}
