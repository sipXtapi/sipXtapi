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
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.setting.Setting;

public class MediantGateway extends Gateway {
    public static final String FACTORY_ID = "gwMediant1000";
    public static final String MANUFACTURER = "audiocodes";
    
    public MediantGateway() {
        setFactoryId(FACTORY_ID);
    }
    
    public Setting getSettingModel() {
        return getModelFilesContext().loadModelFile(MANUFACTURER, "mediant-gateway.xml");
    }

    /**
     * create file and call generateProfiles function for that writer
     */
    public void generateProfiles() {
        Writer wtr = null;
        try {
            File tftpDir = new File(getTftpRoot());
            if (!tftpDir.exists()) {
                FileUtils.forceMkdir(tftpDir);
            }
            File iniFile = new File(tftpDir, getSerialNumber() + ".ini");
            wtr = new FileWriter(iniFile);
            generateProfiles(wtr);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(wtr);
        }
    }

    public void generateProfiles(Writer writer) throws IOException {
        BufferedReader iniReader = null;
        try {
            SettingIniFilter iniWriter = new SettingIniFilter(writer, getSettings());
            File iniTemplate = getModelFilesContext().getModelFile(MANUFACTURER, "mp104.ini");
            iniReader = new BufferedReader(new FileReader(iniTemplate));
            while (iniReader.ready()) {
                String line = iniReader.readLine();
                iniWriter.write(line);
                iniWriter.write('\n');
            }
        } finally {
            IOUtils.closeQuietly(iniReader);
        }
    }
}
