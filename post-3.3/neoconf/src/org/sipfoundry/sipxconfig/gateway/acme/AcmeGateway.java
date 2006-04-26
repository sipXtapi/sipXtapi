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
package org.sipfoundry.sipxconfig.gateway.acme;

import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.phone.PhoneDefaults;

public class AcmeGateway extends Gateway {

    private PhoneDefaults m_defaults;

    protected void defaultSettings() {
        super.defaultSettings();
        // TODO: add code that sets common settings for all gateways
    }

    public void generateProfiles() {
        // TODO: add code that generates configuration files
    }

    public void removeProfiles() {
        // TODO: add code that cleans generated configuration files
    }

    public void setDefaults(PhoneDefaults defaults) {
        m_defaults = defaults;
    }

}
