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

public class AbstractSettingVisitor implements SettingVisitor {

    public void visitSetting(Setting setting_) {
        // do nothing
    }

    public boolean visitSettingGroup(SettingSet group_) {
        return true;
    }

    public boolean visitSettingArray(SettingArray array) {
        return true;
    }
}
