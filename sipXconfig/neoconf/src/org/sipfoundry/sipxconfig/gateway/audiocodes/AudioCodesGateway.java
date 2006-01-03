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
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.phone.PhoneDefaults;

public class AudioCodesGateway extends Gateway {

    private PhoneDefaults m_defaults;

    protected void defaultSettings() {
        super.defaultSettings();
        AudioCodesModel model = (AudioCodesModel) getModel();
        setSettingValue(model.getProxyNameSetting(), m_defaults.getDomainName());
        setSettingValue(model.getProxyIpSetting(), m_defaults.getProxyServerAddr());

        setSettingValue("SIP_Params/SIPGATEWAYNAME", getDefaults().getDomainName());
        setSettingValue("SIP_Params/SIPDESTINATIONPORT", getDefaults().getProxyServerSipPort());
    }

    /**
     * Create file and call generateProfiles function for that writer
     */
    public void generateProfiles() {
        Writer wtr = null;
        try {
            File iniFile = getIniFile();
            wtr = new FileWriter(iniFile);
            generateProfiles(wtr);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(wtr);
        }
    }

    public void removeProfiles() {
        try {
            FileUtils.forceDelete(getIniFile());
        } catch (IOException e) {
            LogFactory.getLog(this.getClass()).info(e.getMessage());
        }
    }

    /**
     * @return object representing ini file - does not create the file but does create parent
     *         directory if needed
     */
    File getIniFile() throws IOException {
        File tftpDir = new File(getTftpRoot());
        if (!tftpDir.exists()) {
            FileUtils.forceMkdir(tftpDir);
        }
        File iniFile = new File(tftpDir, getSerialNumber() + ".ini");
        return iniFile;
    }

    /**
     * Generates profiles by copying and enhancing template file. 
     * 
     * Do not change it to IOUtils.copy since SettingIniFilter is not fully implemented. It onky
     * works fif write(String str) method is used.
     */
    public void generateProfiles(Writer writer) throws IOException {
        BufferedReader iniReader = null;
        try {
            SettingIniFilter iniWriter = new SettingIniFilter(writer, getSettings());
            AudioCodesModel model = (AudioCodesModel) getModel();
            File iniTemplate = model.getIniFileTemplate();
            iniReader = new BufferedReader(new FileReader(iniTemplate));
            String line;
            while ((line = iniReader.readLine()) != null) {
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

    public PhoneDefaults getDefaults() {
        return m_defaults;
    }
}
