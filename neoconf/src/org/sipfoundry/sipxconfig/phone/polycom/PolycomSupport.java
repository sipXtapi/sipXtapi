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

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Support for Polycom phones
 */
public interface PolycomSupport {

    public Setting getLineSettingModel();

    public Setting getEndpointSettingModel();

    public String getSystemDirectory();

    public String getDnsDomain();

    public String getClearTextPassword(User user);
}
