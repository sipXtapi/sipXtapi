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


public interface SettingVisitor {
    
    public void visitSetting(Setting setting);
    
    /**
     * @return false if you do not want to visit children
     */
    public boolean visitSettingGroup(Setting group);

}
