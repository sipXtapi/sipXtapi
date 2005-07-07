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
package org.sipfoundry.sipxconfig.phone;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.setting.Setting;

public class GenericPhone extends AbstractPhone {

    private String m_tftpRoot;

    private VelocityEngine m_velocityEngine;

    private SipService m_sip;
    
    private String m_phoneTemplate;

    private String m_webDirectory;
    
    public String getWebDirectory() {
        return m_webDirectory;
    }
    
    public void setWebDirectory(String webDirectory) {
        m_webDirectory = webDirectory;
    }

    public void setSipService(SipService sip) {
        m_sip = sip;
    }

    public SipService getSipService() {
        return m_sip;
    }

    public String getTftpRoot() {
        return m_tftpRoot;
    }

    public void setTftpRoot(String tftpRoot) {
        m_tftpRoot = tftpRoot;
    }

    public VelocityEngine getVelocityEngine() {
        return m_velocityEngine;
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    protected void makeParentDirectory(File f) {
        if (!f.getParentFile().exists()) {
            if (!f.getParentFile().mkdirs()) {
                throw new RuntimeException("Could not create parent directory for file "
                        + f.getPath());
            }
        }
    }

    public Setting getSettingModel() {
        return null;
    }

    public String getPhoneFilename() {
        String phoneFilename = getPhoneData().getSerialNumber();
        return getTftpRoot() + "/" + phoneFilename.toUpperCase() + ".cfg";
    }

    public void generateProfiles() {
        Writer wtr = null;
        try {
            File file = new File(getTftpRoot() + '/' + getPhoneFilename());
            makeParentDirectory(file);
            wtr = new FileWriter(file);
            generateProfile(wtr);
        } catch (IOException e) {
            throw new RuntimeException(e);
        } finally {
            IOUtils.closeQuietly(wtr);
        }
    }    
    
    public String getPhoneTemplate() {
        return m_phoneTemplate;
    }

    public void setPhoneTemplate(String phoneTemplate) {
        m_phoneTemplate = phoneTemplate;
    }

    public void generateProfile(Writer out) {
        VelocityProfileGenerator profile = new VelocityProfileGenerator(this);
        generateProfile(profile, getPhoneTemplate(), out);        
    }

    protected void generateProfile(VelocityProfileGenerator cfg, String template, Writer out) {
        cfg.setVelocityEngine(getVelocityEngine());
        cfg.generateProfile(template, out);
    }

    /**
     * @throws RestartException is cannot complete operation
     */
    public void restart() {
        if (getLines().size() == 0) {
            throw new RestartException("Restart command is sent to first line and "
                    + "first phone line is not valid");
        }

        Line line = getLine(0);
        m_sip.sendCheckSync(line);
    }
}
