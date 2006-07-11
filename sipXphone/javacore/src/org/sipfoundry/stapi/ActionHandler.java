/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/stapi/ActionHandler.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.stapi;


/**
 * The Action handler interface is used to report call related actions.
 *
 * @author Robert J. Andreasen, Jr.
 */
public interface ActionHandler
{
    /**
     * Invoked when an action is performed.
     */
    public boolean performAction(String strAction, Object objParam1, Object objParam2) ;


    /**
     * Invoked to determine the label string for displayed as a buttn on a call
     * progress form.
     */
    public String getLabel() ;


    /**
     * Invoked to determine the label hint string for displayed as a buttn on a
     * call progress form.
     */
    public String getHint() ;
}
