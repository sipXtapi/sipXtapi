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

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.PrimaryKeySource;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.Tag;

/**
 * Implements some of the more menial methods of Phone interface 
 */
public abstract class AbstractPhone implements Phone, PrimaryKeySource {

    private PhoneData m_meta;
    
    private Setting m_settings;
    
    private DataCollection m_lines = new DataCollection();
    
    private PhoneContext m_phoneContext;
    
    private String m_lineFactoryId;
    
    private String m_modelFile;
    
    private PhoneDefaults m_defaults;       
    
    public PhoneDefaults getDefaults() {
        return m_defaults;
    }

    public void setDefaults(PhoneDefaults defaults) {
        m_defaults = defaults;
    }

    public String getModelFile() {
        return m_modelFile;
    }

    public void setModelFile(String modelFile) {
        m_modelFile = modelFile;
    }

    public void setPhoneData(PhoneData meta) {
        m_meta = meta;
    }
    
    /**
     * No adapters supported in generic implementation
     */
    public Object getAdapter(Class interfac_) {
        return null;
    }
    
    public List getLines() {
        return m_lines;
    }
    
    public void addLine(Line line) {
        m_lines.add(line);
        DataCollectionUtil.updatePositions(m_lines);
    }
    
    public Line getLine(int position) {
        return (Line) m_lines.get(position);
    }
    
    public PhoneData getPhoneData() {
        return m_meta;
    }

    public Setting getSettings() {
        if (m_settings == null) {
            m_settings = getSettingModel();
            setDefaults();
            Tag rootTag = getPhoneContext().loadRootPhoneTag();
            m_settings = m_meta.decorate(rootTag, m_settings);
        }

        return m_settings;
    }
    
    protected void setDefaults() {
        getDefaults().setPhoneDefaults(this);        
    }
    
    public abstract Setting getSettingModel();
    
    public Object getPrimaryKey() {
        return m_meta.getPrimaryKey();
    }
    
    public Collection getDeletedLines() {
        return m_lines.getDeleted();
    }

    public PhoneContext getPhoneContext() {
        return m_phoneContext;
    }
    
    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }
    
    public void setLineFactoryId(String lineFactoryId) {
        m_lineFactoryId = lineFactoryId;
    }
    
    public String getLineFactoryId() {
        return m_lineFactoryId;
    }

    public Line createLine(LineData meta) {
        Line line = getPhoneContext().newLine(m_lineFactoryId);
        line.setPhone(this);
        line.setLineData(meta);
        return line;
    }
}

class DataCollection extends ArrayList {
    
    private Set m_deleted = new HashSet();
    
    /*  may call remove under the covers */
    public void clear() {
        m_deleted.addAll(this);
        super.clear();
    }
    
    public boolean addAll(Collection c) {
        boolean added = super.addAll(c);
        m_deleted.removeAll(c);
        return added;
    }
    
    public boolean remove(Object o) {
        boolean removed = super.remove(o);
        if (removed) {
            m_deleted.add(o);
        }
        return removed;
    }
    
    public boolean add(Object o) {
        boolean added = super.add(o);
        m_deleted.remove(o);
        return added;
    }
    
    Collection getDeleted() {
        return m_deleted;
    }
}
