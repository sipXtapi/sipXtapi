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
import org.sipfoundry.sipxconfig.setting.Folder;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingGroup;
import org.sipfoundry.sipxconfig.setting.ValueStorage;

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
    
    public String getModelFile() {
        return m_modelFile;
    }

    public void setModelFile(String modelFile) {
        m_modelFile = modelFile;
    }

    public void setPhoneData(PhoneData meta) {
        m_meta = meta;
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
            Setting settings = getSettingModel();
            setDefaults(settings);
            decorateSettings(settings);
        }

        return m_settings;
    }
    
    protected abstract void setDefaults(Setting settings);
    
    public abstract Setting getSettingModel();
    
    protected void decorateSettings(Setting settings) {
        m_settings = settings;
        ValueStorage valueStorage = m_meta.getValueStorage();
        if (valueStorage == null) {
            valueStorage = new ValueStorage();
            m_meta.setValueStorage(valueStorage);
        }

        Folder folder = m_meta.getFolder();
        if (folder != null) {
            m_settings = (SettingGroup) folder.decorate(m_settings);
        }

        m_settings = (SettingGroup) valueStorage.decorate(m_settings);        
    }
    
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
