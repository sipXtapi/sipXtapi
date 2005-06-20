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

import org.sipfoundry.sipxconfig.common.PrimaryKeySource;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.Tag;

public abstract class AbstractLine implements Line, PrimaryKeySource {

    private LineData m_meta;

    private Phone m_phone;

    private Setting m_settings;
    
    private String m_modelFile;

    private PhoneDefaults m_defaults;
    
    private String m_uri;
    
    public PhoneDefaults getDefaults() {
        return m_defaults;
    }

    public void setDefaults(PhoneDefaults defaults) {
        m_defaults = defaults;
    }

    /**
     * No adapters supported in generic implementation
     */
    public Object getAdapter(Class interfac_) {
        return null;
    }

    public PhoneContext getPhoneContext() {
        return getPhone().getPhoneContext();
    }

    public String getModelFile() {
        return m_modelFile;
    }

    public void setModelFile(String modelFile) {
        m_modelFile = modelFile;
    }

    public void setLineData(LineData meta) {
        m_meta = meta;
        m_meta.setPhoneData(getPhone().getPhoneData());
    }

    public LineData getLineData() {
        return m_meta;
    }

    public Phone getPhone() {
        return m_phone;
    }

    public void setPhone(Phone phone) {
        m_phone = phone;
    }

    public Setting getSettings() {
        if (m_settings == null) {
            m_settings = getSettingModel();
            setDefaults();
            Tag rootTag = getPhoneContext().loadRootLineTag();
            m_settings = m_meta.decorate(rootTag, m_settings);
        }

        return m_settings;
    }
    
    protected void setDefaults() {
        getDefaults().setLineDefaults(this, m_meta.getUser());        
    }

    public int getPosition() {
        return m_meta.getPosition();
    }

    public void setPosition(int position) {
        m_meta.setPosition(position);
    }

    public Object getPrimaryKey() {
        return m_meta.getId();
    }

    public String getUri() {
        // HACK: uri is determined by it's settings, so need to initialize
        // them before determining the URI. hesitated to create an initialize()
        // method on phones and lines for the moment as to not introduce yet 
        // another setup requirement on related objects.
        getSettings();
        
        return m_uri;
    }
    
    public void setUri(String uri) {
        m_uri = uri;
    }
}
