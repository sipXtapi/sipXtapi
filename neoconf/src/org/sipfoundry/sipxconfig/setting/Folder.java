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

import java.util.Iterator;
import java.util.Map;

/**
 * Settings meta information user overrides is stored in this collection
 */
public class Folder extends AbstractStorage implements SettingVisitor {

    private static final long serialVersionUID = 1L;

    private String m_label;

    private String m_resource;

    private Folder m_parent;

    public Map getFolderSettings() {
        return getDelegate();
    }

    public void setFolderSettings(Map delegate) {
        setDelegate(delegate);
    }

    public Setting decorate(Setting setting) {
        setting.acceptVisitor(this);
        return setting;
    }

    public Folder getParent() {
        return m_parent;
    }

    public void setParent(Folder parent) {
        m_parent = parent;
    }

    public void visitSetting(Setting setting) {
        FolderSetting meta = (FolderSetting) get(setting.getPath());
        if (meta == null) {
            meta = new FolderSetting(this, setting);
        } else {
            meta.setDelegate(setting);
        }

        setting.getParent().addSetting(meta);
    }

    public void visitSettingGroup(Setting group) {
        Iterator i = group.getValues().iterator();
        while (i.hasNext()) {
            ((Setting) i.next()).acceptVisitor(this);
        }
    }

    public String getLabel() {
        return m_label;
    }

    public void setLabel(String label) {
        m_label = label;
    }

    public String getResource() {
        return m_resource;
    }

    public void setResource(String resource) {
        m_resource = resource;
    }
}

