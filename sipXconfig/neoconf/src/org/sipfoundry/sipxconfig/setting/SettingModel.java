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

public class SettingModel extends SettingDecorator {

    private String m_resource;

    public SettingModel() {
        super();
    }

    public void setResource(String resource) {
        m_resource = resource;
    }

    public void setModelFilesContext(ModelFilesContext modelFileContext) {
        Setting model = modelFileContext.loadModelFile(m_resource);
        setDelegate(model);
    }

    public String getDefaultValue() {
        // not really used, but delegate anyway
        return getDelegate().getDefaultValue();
    }
}
