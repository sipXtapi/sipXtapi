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
import java.util.List;

import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.GenericPhone;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.RestartException;
import org.sipfoundry.sipxconfig.phone.SipService;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

/**
 * Support for Polycom 300, 400, and 500 series phones and model 3000 conference phone
 */
public class PolycomPhone extends GenericPhone {

    private Polycom m_model = Polycom.MODEL_300;

    private CoreContext m_coreContext;

    private String m_tftpRoot;

    private VelocityEngine m_velocityEngine;

    private String m_phoneConfigDir = "polycom/mac-address.d";

    private String m_phoneTemplate = m_phoneConfigDir + "/phone.cfg.vm";

    private String m_sipTemplate = m_phoneConfigDir + "/sip.cfg.vm";

    private String m_coreTemplate = m_phoneConfigDir + "/ipmid.cfg.vm";

    private String m_applicationTemplate = "polycom/mac-address.cfg.vm";

    private Setting m_lineModel;

    private Setting m_endpointModel;

    private String m_lineModelFilename;

    private String m_endpointModelFilename;

    private SipService m_sip;

    private int m_restartLineIndex;

    public PolycomPhone() {
        setEndpointModelFilename("polycom/phone.xml");
        setLineModelFilename("polycom/line.xml");
    }

    public void setSipService(SipService sip) {
        m_sip = sip;
    }

    /**
     * a SIP message to restart phone is sent to the line, this is the line index number
     * zero based,  default is zero (e.g. first line)
     */
    public int getRestartLineIndex() {
        return m_restartLineIndex;
    }

    public void setRestartLineIndex(int restartLineIndex) {
        m_restartLineIndex = restartLineIndex;
    }

    public VelocityEngine getVelocityEngine() {
        return m_velocityEngine;
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public String getTftpRoot() {
        return m_tftpRoot;
    }

    public void setTftpRoot(String tftpRoot) {
        m_tftpRoot = tftpRoot;
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

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    CoreContext getCoreContext() {
        return m_coreContext;
    }

    public String getModelId() {
        return m_model.getModelId();
    }

    public String getDisplayLabel() {
        return m_model.getDisplayLabel();
    }

    public void setModelId(String id) {
        m_model = Polycom.getModel(id);
    }

    public int getMaxLineCount() {
        return m_model.getMaxLines();
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

    /**
     * HACK: should be private, avoiding checkstyle error
     */
    void generateProfile(ConfigurationTemplate cfg, String outputFile) throws IOException {
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

    public void generateProfiles(Endpoint endpoint) throws IOException {
        initialize();

        ApplicationConfiguration app = new ApplicationConfiguration(this, endpoint);
        app.setTemplate(getApplicationTemplate());
        generateProfile(app, app.getAppFilename());

        CoreConfiguration core = new CoreConfiguration(this, endpoint);
        core.setTemplate(getCoreTemplate());
        generateProfile(core, app.getCoreFilename());

        PhoneConfiguration phone = new PhoneConfiguration(this, endpoint);
        phone.setTemplate(getPhoneTemplate());
        generateProfile(phone, app.getPhoneFilename());

        SipConfiguration sip = new SipConfiguration(this, endpoint);
        sip.setTemplate(getSipTemplate());
        generateProfile(sip, app.getSipFilename());

        app.deleteStaleDirectories();
    }

    /**
     * @throws RestartException is cannot complete operation
     */
    public void restart(Endpoint endpoint) {
        List lines = endpoint.getLines();
        if (lines.size() <= getRestartLineIndex()) {
            Object[] msgParams = new Object[] {
                new Integer(getRestartLineIndex() + 1), 
                endpoint.getDisplayLabel() 
            };
            String msg = MessageFormat.format("Restart command is sent to line {0} and "
                    + "phone {1} does a valid line", msgParams);
            throw new RestartException(msg);
        }
        Line lineData = (Line) lines.get(getRestartLineIndex());
        PolycomLine line = new PolycomLine(this, lineData);

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

    Setting getLineModel() {
        // cache it, but may be helpful to reload model on fly in future
        if (m_lineModel == null) {
            File modelDefsFile = getFile(getSystemDirectory(), getLineModelFilename());
            m_lineModel = new XmlModelBuilder().buildModel(modelDefsFile);
        }
        return m_lineModel;
    }

    Setting getEndpointModel() {
        // cache it, but may be helpful to reload model on fly in future
        if (m_endpointModel == null) {
            File modelDefsFile = getFile(getSystemDirectory(), getEndpointModelFilename());
            m_endpointModel = new XmlModelBuilder().buildModel(modelDefsFile);
        }
        return m_endpointModel;
    }

    public Setting getSettingModel(Line line) {
        return new PolycomLine(this, line).getSettings();
    }

    public Setting getSettingModel(Endpoint endpoint) {
        return new PolycomEndpoint(this, endpoint).getSettings();
    }

    public String getEndpointModelFilename() {
        return m_endpointModelFilename;
    }

    public void setEndpointModelFilename(String endpointModelFilename) {
        m_endpointModelFilename = endpointModelFilename;
    }

    protected File getFile(String root, String filename) {
        return new File(root + '/' + filename);
    }

    public String getLineModelFilename() {
        return m_lineModelFilename;
    }

    public void setLineModelFilename(String lineModelFilename) {
        m_lineModelFilename = lineModelFilename;
    }

}

