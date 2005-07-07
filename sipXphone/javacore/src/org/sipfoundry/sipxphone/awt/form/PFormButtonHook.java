/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/awt/form/PFormButtonHook.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.awt.form;

import org.sipfoundry.sipxphone.awt.event.*;


/**
 * Interface to hook/intercept button events for a specific form.
 *
 * @author Robert J. Andreasen, Jr.
 */
public interface PFormButtonHook
{
    /**
     * Invoked whenever a new button event is dispatched to particular form.
     *
     * @return TRUE to consume the event (form will not receive it) or FALSE
     *         to allow further processing by form.
     */
    public boolean buttonEvent(PButtonEvent event, PForm form) ;
}