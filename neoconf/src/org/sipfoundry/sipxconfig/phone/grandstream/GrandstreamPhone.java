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
package org.sipfoundry.sipxconfig.phone.grandstream;

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
import org.sipfoundry.sipxconfig.phone.SipService;
import org.sipfoundry.sipxconfig.phone.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

/**
 * Support for Grandstream BudgeTone / HandyTone
 */
public abstract class GrandstreamPhone extends AbstractPhone {

    public static final String FACTORY_ID = "grandstream";
    
    public static final String SIP = "sip";

    private String m_tftpRoot;

    private VelocityEngine m_velocityEngine;

    private SipService m_sip;
    
    private String m_phoneTemplate;

    public Setting getSettingModel() {
        String sysPath = getPhoneContext().getSystemDirectory(); 
        File sysDir = new File(sysPath);
        File modelDefsFile = new File(sysDir, getModelFile());
        Setting all = new XmlModelBuilder(sysPath).buildModel(modelDefsFile);
        Setting model = all.getSetting(getModel().getModelId());
        
        return model;
    }
    
    public GrandstreamModel getModel() {
        return GrandstreamModel.getModel(getPhoneData().getFactoryId());
    }

    public void setSipService(SipService sip) {
        m_sip = sip;
    }

    public SipService getSip() {
        return m_sip;
    }

    public String getPhoneFilename() {
        String phoneFilename = getPhoneData().getSerialNumber();
        return getTftpRoot() + "/cfg" + phoneFilename.toUpperCase();
    }

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
        // waiting for fix, doesn't work yet for some reason
        if (true) {
            return;
        }

        if (getLines().size() == 0) {
            return;
            // throw new RestartException("Restart command is sent to first line and "
            //        + "first phone line is not valid");
        }

        GrandstreamLine line = (GrandstreamLine) getLine(0);

        // The check-sync message is a flavor of unsolicited NOTIFY
        // this message does not require that the phone be enrolled
        // the message allows us to reboot a specific phone 
        String restartSip = "NOTIFY {0} SIP/2.0\r\n" + "Via: {1}\r\n" + "From: {2}\r\n"
                + "To: {3}\r\n" + "Event: check-sync\r\n" + "Date: {4}\r\n" + "Call-ID: {5}\r\n"
                + "CSeq: 1 NOTIFY\r\n" + "Contact: null\r\n" + "Content-Length: 0\r\n" + "\r\n";
        Object[] sipParams = new Object[] { 
            line.getNotifyRequestUri(), m_sip.getServerVia(),
            m_sip.getServerUri(), line.getUri(), m_sip.getCurrentDate(),
            m_sip.generateCallId() 
        };
        String msg = MessageFormat.format(restartSip, sipParams);
        try {
            m_sip.send(msg);
        } catch (IOException e) {
            return;
            // throw new RestartException("Could not send restart SIP message", e);
        }
    }
}
