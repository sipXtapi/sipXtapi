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
package org.sipfoundry.sipxconfig.settings;

import org.sipfoundry.sipxconfig.phone.AbstractSettings;
import org.sipfoundry.sipxconfig.phone.SettingSet;

/**
 * Settings related to network parameters like proxy, tftp boot server, etc
 */
public class NetworkSettings extends AbstractSettings {

    public NetworkSettings(SettingSet settings) {
        super(settings);
    }
}
