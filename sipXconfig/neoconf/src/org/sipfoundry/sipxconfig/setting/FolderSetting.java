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
package org.sipfoundry.sipxconfig.setting;

/**
 * Attributes about settings administrators can influence.
 */
public class FolderSetting extends SettingDecorator {

    private String m_value;

    private Boolean m_hidden;

    private Folder m_folder;

    /** BEAN ACCESS ONLY */
    public FolderSetting() {
    }

    public FolderSetting(Folder folder, Setting delegate) {
        super(delegate);
        setFolder(folder);
    }

    public void setFolder(Folder folder) {
        m_folder = folder;
    }

    public Folder getFolder() {
        return m_folder;
    }

    public String getDefaultValue() {
        return getDelegate().getValue();
    }

    public String getValue() {
        return m_value != null ? m_value : getDelegate().getValue();
    }

    public void setValue(String value) {
        m_value = value;
        updateStorage();
    }

    public boolean isHidden() {
        return m_hidden != null ? m_hidden.booleanValue() : getDelegate().isHidden();
    }

    public void setHidden(boolean hidden) {
        m_hidden = hidden ? Boolean.TRUE : Boolean.FALSE;
        updateStorage();
    }

    /**
     * If any values are overridden, add yourself to meta storage, if values
     * match delegate, then no need to exist, and this remove itself from meta
     * storage
     */
    private void updateStorage() {
        if (getDelegate() != null) {
            if (isHidden() != getDelegate().isHidden() || !equalValue(m_value, getDefaultValue())) {
                m_folder.put(getDelegate().getPath(), this);
            } else {
                m_folder.remove(getDelegate().getPath());
            }
        }
    }

    private boolean equalValue(String a, String b) {
        return a == null ? b == null : a.equals(b);
    }
}
