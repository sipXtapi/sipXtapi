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

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.gateway.Gateway;

public class AcmeGateway extends Gateway {

    private DeviceDefaults m_defaults;

    protected void defaultSettings() {
        super.defaultSettings();
        setSettingValue("basic/proxyAddress", m_defaults.getProxyServerAddr());
    }

    public void generateProfile(String template, Writer out) {
        VelocityProfileGenerator profile = new VelocityProfileGenerator(this);
        profile.setVelocityEngine(getVelocityEngine());
        profile.generateProfile(template, out);
    }

    private String getTemplate() {
        return "acme/acme-gateway.vm";
    }

    private String getProfileFilename() {
        return getSerialNumber() + ".ini";
    }
    
    public void generateProfiles() {
        String profileFileName = getProfileFilename();
        String template = getTemplate();
        if (profileFileName == null || template == null) {
            return;
        }

        Writer wtr = null;
        try {
            File file = new File(profileFileName);
            VelocityProfileGenerator.makeParentDirectory(file);
            wtr = new FileWriter(file);
            generateProfile(template, wtr);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(wtr);
        }
    }

    public void removeProfiles() {
        String profileFileName = getProfileFilename();
        if (profileFileName == null) {
            return;
        }
        VelocityProfileGenerator.removeProfileFiles(new File[] {
            new File(profileFileName)
        });
    }

    public void setDefaults(DeviceDefaults defaults) {
        m_defaults = defaults;
    }
}
