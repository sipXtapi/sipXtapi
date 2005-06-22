/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import java.io.Writer;

import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.setting.Setting;

public class MediantGateway extends Gateway {
    public static final String FACTORY_ID = "gwMediant1000";
    public static final String MANUFACTURER = "audiocodes";

    public Setting getSettingModel() {
        return getGatewayContext().loadModelFile(MANUFACTURER, "mediant-gateway.xml");
    }

    public void generateProfiles(Writer writer) {
        IniFileWriter iniFileWriter = new IniFileWriter(writer);
        iniFileWriter.setGroupNameSuffix(" Params");
        getSettings().acceptVisitor(iniFileWriter);
    }
}
