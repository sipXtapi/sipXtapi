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

import org.sipfoundry.sipxconfig.phone.Endpoint;

/**
 * Velocity model for generating sip.cfg, pointer to all other 
 * config files.  See page 11 of Administration guide for more information
 */
public class SipConfiguration  extends ConfigurationTemplate {
    
    public SipConfiguration(PolycomPhone phone, Endpoint endpoint) {
        super(phone, endpoint);
    }
}
