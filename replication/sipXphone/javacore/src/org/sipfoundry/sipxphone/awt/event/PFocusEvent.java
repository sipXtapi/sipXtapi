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
 * Event object sent via the PFocusListener interface whenever a component
 * gains or loses input focus.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PFocusEvent extends PEvent
{
    /**
     * Constructs a focus event with the specified source object
     *
     * @param source object that is gaining or losing focus
     */
    public PFocusEvent(Object objSource)
    {
        super(objSource, TYPE_FOCUS_EVENT) ;        
    }
}