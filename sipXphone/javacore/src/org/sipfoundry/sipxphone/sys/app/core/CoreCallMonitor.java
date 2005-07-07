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

package org.sipfoundry.sipxphone.sys.app.core ; 

import org.sipfoundry.stapi.* ;


/**
 * Any objects that the core application uses to display call status must 
 * implement this interface.  This interfaces defines the mechanism when/how
 * it can determine what call the form is monitoring.
 *
 * @author Robert J. Andreasen, Jr.
 */
public interface CoreCallMonitor
{    
    /**
     * What call is this form monitoring?
     */
    public PCall getMonitoredCall() ;
}