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
package org.sipfoundry.sipxconfig.phone.polycom;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.text.MessageFormat;

import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.phone.AbstractPhone;
import org.sipfoundry.sipxconfig.phone.RestartException;
import org.sipfoundry.sipxconfig.phone.SipService;
import org.sipfoundry.sipxconfig.phone.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

/**
 * Support for Polycom 300, 400, and 500 series phones and model 3000 conference phone
 */
public class PolycomPhone extends AbstractPhone {
    
    public static final String FACTORY_ID = "polycom";

    public static final String SERVER = "server";

    public static final String ADDRESS = "address";

    public static final String FIRST = "1";

    public static final String CALL = "call";
        
    public static final String REGISTRATION = "reg";

    private String m_phoneConfigDir = "polycom/mac-address.d";

    private String m_phoneTemplate = m_phoneConfigDir + "/phone.cfg.vm";

    private String m_sipTemplate = m_phoneConfigDir + "/sip.cfg.vm";

    private String m_coreTemplate = m_phoneConfigDir + "/ipmid.cfg.vm";

    private String m_applicationTemplate = "polycom/mac-address.cfg.vm";

    private String m_tftpRoot;

    private VelocityEngine m_velocityEngine;

    private SipService m_sip;
    
    private String m_outboundProxyAddress;
    
    /** BEAN ACCESS ONLY */
    public PolycomPhone() {
        setLineFactoryId(PolycomLine.FACTORY_ID);
    }

    public PolycomModel getModel() {
        return PolycomModel.getModel(getPhoneData().getFactoryId());
    }

    public void setSipService(SipService sip) {
        m_sip = sip;
    }

    public SipService getSip() {
        return m_sip;
    }

    public String getPhoneTemplate() {
        return m_phoneTemplate;
    }

    public void setPhoneTemplate(String phoneTemplate) {
        m_phoneTemplate = phoneTemplate;
    }

    public String getCoreTemplate() {
        return m_coreTemplate;
    }

    public void setCoreTemplate(String coreTemplate) {
        m_coreTemplate = coreTemplate;
    }

    public String getApplicationTemplate() {
        return m_applicationTemplate;
    }

    public void setApplicationTemplate(String applicationTemplate) {
        m_applicationTemplate = applicationTemplate;
    }

    public String getSipTemplate() {
        return m_sipTemplate;
    }

    public void setSipTemplate(String sipTemplate) {
        m_sipTemplate = sipTemplate;
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

    public String getOutboundProxyAddress() {
        return m_outboundProxyAddress;
    }    

    public void setOutboundProxyAddress(String outboundProxyAddress) {
        m_outboundProxyAddress = outboundProxyAddress;
    }
    
    public Setting getSettingModel() {
        File modelDefsFile = new File(getPhoneContext().getSystemDirectory() + '/' + FACTORY_ID + "/phone.xml");
        Setting model = new XmlModelBuilder().buildModel(modelDefsFile).copy();
        
        return model;
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
            initialize();

            ApplicationConfiguration app = new ApplicationConfiguration(this);
            generateProfile(app, getApplicationTemplate(), app.getAppFilename());

            CoreConfiguration core = new CoreConfiguration(this);
            generateProfile(core, getCoreTemplate(), app.getCoreFilename());

            PhoneConfiguration phone = new PhoneConfiguration(this);
            generateProfile(phone, getPhoneTemplate(), app.getPhoneFilename());

            VelocityProfileGenerator sip = new VelocityProfileGenerator(this);
            generateProfile(sip, getSipTemplate(), app.getSipFilename());

            app.deleteStaleDirectories();
        } catch (IOException ioe) {
            throw new RuntimeException("Could not generate phones", ioe);
        }
    }

    /**
     * @throws RestartException is cannot complete operation
     */
    public void restart() {
        if (getLines().size() == 0) {
            throw new RestartException("Restart command is sent to first line and "
                    + "first phone line is not valid");
        }

        PolycomLine line = (PolycomLine) getLine(0);

        // The check-sync message is a flavor of unsolicited NOTIFY
        // this message does not require that the phone be enrolled
        // the message allows us to reboot a specific phone 
        String restartSip = "NOTIFY {0} SIP/2.0\r\n" + "Via: {1}\r\n"
                + "From: {2}\r\n" + "To: {3}\r\n" + "Event: check-sync\r\n"
                + "Date: {4}\r\n" + "Call-ID: {5}\r\n" + "CSeq: 1 NOTIFY\r\n"
                + "Contact: null\r\n" + "Content-Length: 0\r\n" + "\r\n";
        Object[] sipParams = new Object[] { 
            line.getNotifyRequestUri(), 
            m_sip.getServerVia(),
            m_sip.getServerUri(), 
            line.getUri(),
            m_sip.getCurrentDate(), 
            m_sip.generateCallId()
        };
        String msg = MessageFormat.format(restartSip, sipParams);
        try {
            m_sip.send(msg);
        } catch (IOException e) {
            throw new RestartException("Could not send restart SIP message", e);
        }
    }

    /**
     * HACK: should be private, avoiding checkstyle error
     */
    void generateProfile(VelocityProfileGenerator cfg, String template, String outputFile) throws IOException {
        FileWriter out = null;
        String tftpRoot = getTftpRoot() + '/';
        try {
            File f = new File(tftpRoot + outputFile);
            File d = f.getParentFile();
            if (!d.exists()) {
                if (!d.mkdirs()) {
                    throw new RuntimeException("Could not create profile directory "
                            + d.getPath());
                }
            }
            out = new FileWriter(f);
            cfg.setVelocityEngine(getVelocityEngine());
            cfg.generateProfile(template, out);
        } finally {
            if (out != null) {
                out.close();
            }
        }
    }

    protected void setDefaults(Setting settings) {
        String domainName = getPhoneContext().getDnsDomain();
        Setting voip = settings.getSetting("voIpProt");
        voip.getSetting(SERVER).getSetting(FIRST).getSetting(ADDRESS).setValue(domainName);
        voip.getSetting("SIP.outboundProxy").getSetting(ADDRESS).setValue(getOutboundProxyAddress());
    }
}

