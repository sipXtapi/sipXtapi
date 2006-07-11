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

import java.util.* ;



/**
 * The PListListener interface is used to deliver selection change events from
 * the list control to any interested parties.
 *
 *
 * @author Robert J. Andreasen, Jr.
 */
public interface PListListener
{
    /**
     * This listener method is invoked when the list selection changes.
     *
     * @param PListEvent The event object describing the state change.
     * @see org.sipfoundry.sipxphone.awt.event.PListEvent     
     */    
    public void selectChanged(PListEvent event) ;
}