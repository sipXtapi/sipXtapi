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
package org.sipfoundry.sipxconfig.phone.polycom;

import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.phone.Endpoint;

/**
 * Responsible for generating ipmid.cfg
 */
public class CoreConfiguration extends ConfigurationTemplate {
    
    public CoreConfiguration(PolycomPhone phone, Endpoint endpoint) {
        super(phone, endpoint);
    }
    
    public void addContext(VelocityContext context) {
        context.put("settings", getEndpoint().getSettings(getPhone()));
    }
}
