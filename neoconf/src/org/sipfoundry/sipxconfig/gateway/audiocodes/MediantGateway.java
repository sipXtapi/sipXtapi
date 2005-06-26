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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.Writer;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.setting.Setting;

public class MediantGateway extends Gateway {
    public static final String FACTORY_ID = "gwMediant1000";
    public static final String MANUFACTURER = "audiocodes";

    public Setting getSettingModel() {
        return getGatewayContext().loadModelFile(MANUFACTURER, "mediant-gateway.xml");
    }

    public void generateProfiles(Writer writer) throws IOException {
        SettingIniFilter iniWriter = new SettingIniFilter(writer, getSettings());
        BufferedReader iniReader = null;
        try {
            File iniTemplate = getGatewayContext().getModelFile(MANUFACTURER, "mp104.ini");
            iniReader = new BufferedReader(new FileReader(iniTemplate));
            while (iniReader.ready()) {
                String line = iniReader.readLine();
                iniWriter.write(line);
                iniWriter.write('\n');
            }
        } finally {
            IOUtils.closeQuietly(iniWriter);
            IOUtils.closeQuietly(iniReader);
        }
    }
}
