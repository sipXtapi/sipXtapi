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
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.PhoneMetaData;
import org.sipfoundry.sipxconfig.phone.RestartException;
import org.sipfoundry.sipxconfig.phone.SipService;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Support for Polycom 300, 400, and 500 series phones and model 3000 conference phone
 */
public class PolycomPhone extends AbstractPhone {

    public static final String SERVER = "server";
    
    public static final String ADDRESS = "address";

    public static final String FIRST = "1";
    
    private String m_phoneConfigDir = "polycom/mac-address.d";

    private String m_phoneTemplate = m_phoneConfigDir + "/phone.cfg.vm";

    private String m_sipTemplate = m_phoneConfigDir + "/sip.cfg.vm";

    private String m_coreTemplate = m_phoneConfigDir + "/ipmid.cfg.vm";

    private String m_applicationTemplate = "polycom/mac-address.cfg.vm";

    private PolycomSupport m_polycom;

    private String m_tftpRoot;

    private VelocityEngine m_velocityEngine;

    private SipService m_sip;
    
    /** BEAN ACCESS ONLY */
    public PolycomPhone() {        
    }

    public PolycomPhone(PolycomSupport polycom, PhoneMetaData meta) {
        super(meta);
        setPolycom(polycom);
    }
    
    public PolycomModel getModel() {
        return PolycomModel.getModel(getPhoneMetaData().getFactoryId());
    }
    
    public Line createLine() {
        return new PolycomLine(this);
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

    PolycomSupport getPolycom() {
        return m_polycom;
    }

    public void setPolycom(PolycomSupport polycom) {
        m_polycom = polycom;
    }

    public String getDisplayLabel() {
        return getModel().getDisplayLabel();
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

    public void generateProfiles() throws IOException {
        initialize();

        ApplicationConfiguration app = new ApplicationConfiguration(this);
        app.setTemplate(getApplicationTemplate());
        generateProfile(app, app.getAppFilename());

        CoreConfiguration core = new CoreConfiguration(this);
        core.setTemplate(getCoreTemplate());
        generateProfile(core, app.getCoreFilename());

        PhoneConfiguration phone = new PhoneConfiguration(this);
        phone.setTemplate(getPhoneTemplate());
        generateProfile(phone, app.getPhoneFilename());

        ConfigurationFile sip = new ConfigurationFile(this);
        sip.setTemplate(getSipTemplate());
        generateProfile(sip, app.getSipFilename());

        app.deleteStaleDirectories();
    }

    /**
     * @throws RestartException is cannot complete operation
     */
    public void restart() {
        if (getLineCount() == 0) {
            throw new RestartException("Restart command is sent to first line and "
                    + "first phone line is not valid");
        }
        
        PolycomLine line = (PolycomLine) getLine(0);

        String restartSip = "NOTIFY {0} SIP/2.0\r\n" + "Via: SIP/2.0/TCP {1}\r\n"
                + "From: <sip:{2}>\r\n" + "To: <sip:{0}>\r\n" + "Event: check-sync\r\n"
                + "Date: {3}\r\n" + "Call-ID: {4}\r\n" + "CSeq: 1 NOTIFY\r\n"
                + "Contact: <sip:{2}>\r\n" + "Content-Length: 0\r\n" + "\r\n";
        Object[] sipParams = new Object[] { 
            line.getUri(), 
            m_sip.getServerVia(),
            m_sip.getServerUri(), 
            m_sip.getCurrentDate(), 
            m_sip.generateCallId() 
        };
        String msg = MessageFormat.format(restartSip, sipParams);
        try {
            String to = line.getPrimaryRegistrationServerAddress();
            String sPort = line.getPrimaryRegistrationServerPort();
            int port = PolycomLine.getSipPort(sPort);
            m_sip.send(to, port, msg);
        } catch (IOException e) {
            throw new RestartException("Could not send restart SIP message", e);
        }
    }

    /**
     * HACK: should be private, avoiding checkstyle error
     */
    void generateProfile(ConfigurationFile cfg, String outputFile) throws IOException {
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
            cfg.generateProfile(out);
        } finally {
            if (out != null) {
                out.close();
            }
        }
    }

    public Setting getSettingModel() {
        Setting settings = m_polycom.getEndpointSettingModel().copy();
        String domainName = m_polycom.getDnsDomain();
        Setting voip = settings.getSetting("voIpProt");
        voip.getSetting(SERVER).getSetting(FIRST).getSetting(ADDRESS).setValue(domainName);
        voip.getSetting("SIP.outboundProxy").getSetting(ADDRESS).setValue(domainName);
        
        return settings;
    }
    
}

