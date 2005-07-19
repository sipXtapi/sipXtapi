/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import org.springframework.context.ApplicationEvent;

/**
 * Signifying sipXconfig application is initialized and ready to run.  Good place
 * to start timers, etc.
 * <pre>
 *  Example:
 *    
 *  </pre>
 */
public class ApplicationInitializedEvent extends ApplicationEvent {

    public ApplicationInitializedEvent(Object src) {
        super(src);
    }
}
