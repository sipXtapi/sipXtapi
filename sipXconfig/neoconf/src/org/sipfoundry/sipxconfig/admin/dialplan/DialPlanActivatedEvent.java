/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import org.springframework.context.ApplicationEvent;

/**
 * Event generated when dial plan actication is completed
 */
public class DialPlanActivatedEvent extends ApplicationEvent {

    public DialPlanActivatedEvent(Object eventSource) {
        super(eventSource);
    }
}
