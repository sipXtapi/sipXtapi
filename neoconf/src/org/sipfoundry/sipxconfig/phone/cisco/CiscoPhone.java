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
package org.sipfoundry.sipxconfig.phone.cisco;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringWriter;
import java.io.Writer;
import java.text.MessageFormat;

import org.apache.commons.io.CopyUtils;
import org.apache.commons.io.IOUtils;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.phone.AbstractPhone;
import org.sipfoundry.sipxconfig.phone.RestartException;
import org.sipfoundry.sipxconfig.phone.SipService;
import org.sipfoundry.sipxconfig.phone.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

/**
 * Support for Cisco 7940/7960
 */
public abstract class CiscoPhone extends AbstractPhone {

    public static final String FACTORY_ID = "cisco";

    public static final String PORT = "port";
    
    public static final String SIP = "sip";

    private String m_tftpRoot;

    private VelocityEngine m_velocityEngine;

    private SipService m_sip;
    
    private String m_phoneTemplate;

    public Setting getSettingModel() {
        File sysDir = new File(getPhoneContext().getSystemDirectory());
        File modelDefsFile = new File(sysDir, getModelFile());
        Setting all = new XmlModelBuilder(sysDir).buildModel(modelDefsFile);
        Setting model = all.getSetting(getModel().getModelId());
        
        return model;
    }
    
    public CiscoModel getModel() {
        return CiscoModel.getModel(getPhoneData().getFactoryId());
    }

    public void setSipService(SipService sip) {
        m_sip = sip;
    }

    public SipService getSipService() {
        return m_sip;
    }

    public abstract String getPhoneFilename();

    public String getPhoneTemplate() {
        return m_phoneTemplate;
    }

    public void setPhoneTemplate(String phoneTemplate) {
        m_phoneTemplate = phoneTemplate;
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

    public int getMaxLineCount() {
        return getModel().getMaxLines();
    }

    private void initialize() {
        File tftpRootFile = new File(getTftpRoot());
        if (!tftpRootFile.exists()) {
            if (!tftpRootFile.mkdirs()) {
                throw new RuntimeException("Could not create TFTP root directory "
                        + tftpRootFile.getPath());
            }
        }
    }

    public void generateProfiles() {
        try {
            StringWriter out = new StringWriter();
            generateProfile(out);
            save(out.toString());
        } catch (IOException ioe) {
            throw new RuntimeException("Could not generate phones", ioe);
        }
    }
    
    void generateProfile(Writer out) throws IOException {
        initialize();
        VelocityProfileGenerator profile = new VelocityProfileGenerator(this);
        profile.setVelocityEngine(getVelocityEngine());
        profile.generateProfile(getPhoneTemplate(), out);
        save(out.toString());
    }
    
    protected void save(String profile) throws IOException {
        FileWriter wtr = new FileWriter(getPhoneFilename()); 
        try {
            CopyUtils.copy(profile, wtr);
        } finally {
            IOUtils.closeQuietly(wtr);
        }
    }

    public void restart() {
        if (getLines().size() == 0) {
            throw new RestartException("Restart command is sent to first line and "
                   + "first phone line is not valid");
        }

        CiscoLine line = (CiscoLine) getLine(0);

        // The check-sync message is a flavor of unsolicited NOTIFY
        // this message does not require that the phone be enrolled
        // the message allows us to reboot a specific phone 
        String restartSip = "NOTIFY {0} SIP/2.0\r\n" + "Via: {1}\r\n" + "From: {2}\r\n"
                + "To: {3}\r\n" + "Event: check-sync\r\n" + "Date: {4}\r\n" + "Call-ID: {5}\r\n"
                + "CSeq: 1300 NOTIFY\r\n" + "Contact: null\r\n" + "Content-Length: 0\r\n" + "\r\n";
        Object[] sipParams = new Object[] { 
            line.getNotifyRequestUri(), m_sip.getServerVia(),
            m_sip.getServerUri(), line.getUri(), m_sip.getCurrentDate(),
            m_sip.generateCallId() 
        };
        String msg = MessageFormat.format(restartSip, sipParams);
        try {
            m_sip.send(msg);
        } catch (IOException e) {
            throw new RestartException("Could not send restart SIP message", e);
        }
    }
}
