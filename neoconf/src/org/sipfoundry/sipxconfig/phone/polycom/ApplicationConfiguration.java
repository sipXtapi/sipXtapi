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
 * Velocity model for generating XXXXXXXXXXXX-phone.cfg, pointer to all other 
 * config files.  See page 11 of Administration guide for more information
 */
public class ApplicationConfiguration extends ConfigurationTemplate {
    
    public ApplicationConfiguration(PolycomPhone phone, Endpoint endpoint) {
        super(phone, endpoint);
    }
    
    public void addContext(VelocityContext context) {
        context.put("app", this);
    }
    
    public String getSipBinaryFilename() {
        return "sip.ld";
    }
    
    public String getAppFilename() {
        // todo, put in a sequence generator for TFTP
        return getEndpoint().getSerialNumber() + "-phone.cfg";
    }
    
    public String getCoreFilename() {
        return getEndpoint().getSerialNumber() + ".d/ipmid.cfg";
    }

    public String getSipFilename() {
        return getEndpoint().getSerialNumber() + ".d/sip.cfg";
    }
    
    public String getPhoneFilename() {
        return getEndpoint().getSerialNumber() + ".d/phone1.cfg";        
    }
}
