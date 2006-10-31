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
package org.sipfoundry.sipxconfig.gateway;

import org.sipfoundry.sipxconfig.setting.Setting;

public class SipTrunk extends Gateway {
    public static final String BEAN_ID = "gwSipTrunk";

    public SipTrunk() {
    }

    public SipTrunk(GatewayModel model) {
        super(model);
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("siptrunk.xml", "commserver");
    }

    @Override
    public String getRoute() {
        return getSettingValue("trunk/route");
    }
}
