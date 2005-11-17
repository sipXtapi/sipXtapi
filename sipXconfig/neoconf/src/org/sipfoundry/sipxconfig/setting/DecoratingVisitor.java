/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

/**
 * Special type of vistitor used to decorate the scalar (non-group) settings.
 * 
 * One has to provide decorate method implementation - it will be called for all settings in the
 * hierarchy. Decorated settings will replace the real one.
 */
public abstract class DecoratingVisitor extends AbstractSettingVisitor {

    private Setting m_root;

    public DecoratingVisitor(Setting root) {
        m_root = root;
    }

    public void visitSetting(Setting setting) {
        Setting decorated = decorate(setting);
        String parentPath = setting.getParentPath();
        if (parentPath == null) {
            m_root.addSetting(decorated);
        } else {
            SettingUtil.getSettingFromRoot(m_root, parentPath).addSetting(decorated);
        }
    }

    public abstract Setting decorate(Setting setting);
}
