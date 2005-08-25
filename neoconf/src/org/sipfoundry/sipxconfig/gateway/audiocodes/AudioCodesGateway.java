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
import org.sipfoundry.sipxconfig.phone.PhoneDefaults;

public class AudioCodesGateway extends Gateway {

    private PhoneDefaults m_defaults;

    public void prepareSettings() {
        super.prepareSettings();
        AudioCodesModel model = (AudioCodesModel) getModel();
        setSettingValue(model.getProxyNameSetting(), m_defaults.getDomainName());
        setSettingValue(model.getProxyIpSetting(), m_defaults.getOutboundProxy() + ":"
                + m_defaults.getOutboundProxyPort());
    }

    /**
     * Create file and call generateProfiles function for that writer
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
            AudioCodesModel model = (AudioCodesModel) getModel();
            File iniTemplate = model.getIniFileTemplate();
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

    public void setDefaults(PhoneDefaults defaults) {
        m_defaults = defaults;
    }
}
