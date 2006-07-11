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
 * The PActionListener interface informs listeners whenever an 'action' is 
 * performed.  The PActionEvent contains an action command that can be checked
 * for details.
 *
 * @see PActionEvent
 * 
 * @author Robert J. Andreasen, Jr.
 */
public interface PActionListener
{
    public void actionEvent(PActionEvent event) ;
}