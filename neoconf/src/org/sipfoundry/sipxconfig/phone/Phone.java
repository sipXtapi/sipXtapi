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
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Base class for managed phone subclasses  
 */
public class Phone extends BeanWithGroups {
        
    // public because of checkstyle
    public static final String PHONE_CONSTANT = "phone";

    public static final PhoneModel MODEL = new PhoneModel("unmanagedPhone", "Unmanaged phone");
    
    public static final String PHONE_GROUP_RESOURCE = PHONE_CONSTANT;
    
    private String m_description;

    private String m_serialNumber;
    
    private PhoneModel m_model; 
    
    private List m_lines = Collections.EMPTY_LIST;
    
    private PhoneContext m_phoneContext;
    
    private String m_tftpRoot;

    private VelocityEngine m_velocityEngine;

    private SipService m_sip;
    
    private String m_phoneTemplate;

    private String m_webDirectory;
    
    private String m_beanId;
    
    public Phone() {
        this(MODEL);
    }
    
    protected Phone(String beanId) {
        m_beanId = beanId;
    }
    
    protected Phone(PhoneModel model) {
        m_beanId = model.getBeanId();
        m_model = model;
    }
    
    public String getModelLabel() {
        return m_model.getLabel();
    }
    
    public void setModelId(String modelId) {
        m_model = PhoneModel.getModel(getBeanId(), modelId);
    }
    
    public PhoneModel getModel() {
        return m_model;
    }
    
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
        Setting model = super.getSettingModel();
        if (model == null) {
            return null;
        }
        
        String name = getModel().getName();
        Setting modelType = model.getSetting(name);
        Setting phoneModel = modelType.getSetting(PHONE_CONSTANT);
        phoneModel.setName(StringUtils.EMPTY);
        phoneModel.setParentPath(null); // detach
        
        return phoneModel;
    }

    public Setting getLineSettingModel() {
        Setting model = super.getSettingModel();
        if (model == null) {
            return null;
        }
        
        String name = getModel().getName();
        Setting modelType = model.getSetting(name);
        Setting lineModel = modelType.getSetting("line");
        lineModel.setName(StringUtils.EMPTY);
        lineModel.setParentPath(null); // detach
        
        return lineModel;
    }
    
    public String getPhoneFilename() {
        String phoneFilename = getSerialNumber();
        return getTftpRoot() + "/" + phoneFilename.toUpperCase() + ".cfg";
    }

    public void generateProfiles() {
        Writer wtr = null;
        try {
            File file = new File(getPhoneFilename());
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
        if (getPhoneTemplate() == null) {
            return;
        }
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
    }
    
    protected void sendCheckSyncToFirstLine() {
        if (getLines().size() == 0) {
            throw new RestartException("Restart command is sent to first line and "
                    + "first phone line is not valid");
        }

        Line line = getLine(0);
        m_sip.sendCheckSync(line);
    }
    
    /**
     * @return ids used in PhoneFactory
     */
    public String getBeanId() {
        return m_beanId;
    }
    
    /**
     * Internal, do not call this method. Hibnerate property declared update=false, but 
     * still required method be defined.
     */
    public void setBeanId(String illegal_) {
    }

    public String getModelId() {
        return m_model.getModelId();
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getSerialNumber() {
        return m_serialNumber;
    }

    public void setSerialNumber(String serialNumber) {
        m_serialNumber = cleanSerialNumber(serialNumber);
    }
    
    private static String cleanSerialNumber(String rawNumber) {
        String clean = rawNumber.toLowerCase();
        clean = clean.replaceAll("[:\\s]*", "");
        
        return clean;        
    }
    
    /**
     * No adapters supported in generic implementation
     */
    public Object getAdapter(Class interfac_) {
        return null;
    }
    
    /**
     * No line adapters supported in generic implementation
     */
    public Object getLineAdapter(Line line_, Class interfac_) {
        return null;
    }

    public List getLines() {
        return m_lines;
    }
    
    public void setLines(List lines) {
        m_lines = lines;
    }
    
    public void addLine(Line line) {
        if (m_lines == Collections.EMPTY_LIST) {
            m_lines = new ArrayList();
        }
        line.setPhone(this);
        line.setPosition(m_lines.size());
        m_lines.add(line);
    }
    
    public Line getLine(int position) {
        return (Line) m_lines.get(position);
    }
    
    protected void defaultSettings() {
        getPhoneContext().getPhoneDefaults().setPhoneDefaults(this);
    }
    
    protected void defaultLineSettings(Line line) {
        getPhoneContext().getPhoneDefaults().setLineDefaults(line, line.getUser());
    }

    public PhoneContext getPhoneContext() {
        return m_phoneContext;
    }
    
    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }
    
    public Line createLine() {
        Line line = new Line();
        line.setPhone(this);
        return line;
    }
}
