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
package org.sipfoundry.sipxconfig.common;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.enums.EnumUtils;
import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Permission
 * Copy of permission setting names exist in user-setting.xml
 */
public final class Permission extends Enum {    
    public static final String ENABLE = "ENABLE";    
    public static final String DISABLE = "DISABLE";
    
    public static final Permission APPLICATION = new Permission("application");
    public static final Permission SUPERADMIN = new Permission(APPLICATION, "superadmin");
    public static final Permission TUI_CHANGE_PIN = new Permission(APPLICATION, "tui-change-pin");
    public static final Permission CALL_HANDLING = new Permission("call-handling");
    public static final Permission NO_ACCESS = new Permission(CALL_HANDLING, "NoAccess");
    public static final Permission VOICEMAIL = new Permission(CALL_HANDLING, "Voicemail");
    public static final Permission RESTRICTED_DIALING = new Permission(CALL_HANDLING, "900Dialing");
    public static final Permission TOLL_FREE_DIALING = new Permission(CALL_HANDLING, "TollFree");
    public static final Permission LONG_DISTANCE_DIALING = new Permission(CALL_HANDLING, "LongDistanceDialing");
    public static final Permission INTERNATIONAL_DIALING = new Permission(CALL_HANDLING, "InternationalDialing");
    public static final Permission LOCAL_DIALING = new Permission(CALL_HANDLING, "LocalDialing");
    public static final Permission AUTO_ATTENDANT = new Permission(CALL_HANDLING, "AutoAttendant");
    public static final Permission FORWARD_CALLS_EXTERNAL = new Permission(CALL_HANDLING, "ForwardCallsExternal");
    public static final Permission RECORD_SYSTEM_PROMPTS = new Permission(CALL_HANDLING, "RecordSystemPrompts");
    public static final Permission VALID_USER = new Permission(CALL_HANDLING, "ValidUser");
    
    private Permission m_parent;

    private Permission(String permision) {
        super(permision);
    }

    private Permission(Permission parent, String permision) {        
        super(permision);
        m_parent = parent;
    }

    /**
     * Used for Hibernate type translation
     */
    public static class UserType extends EnumUserType {
        public UserType() {
            super(Permission.class);
        }
    }

    public static Permission getEnum(String permission) {
        return (Permission) getEnum(Permission.class, permission);
    }
    
    public Permission getParent() {
        return m_parent;
    }
    
    public static boolean isEnabled(String value) {
        return ENABLE.equals(value);
    }
    
    /**
     * Returns the path in user settings to this permission group or setting
     */
    public String getSettingPath() {
        StringBuffer sb = new StringBuffer("permission/");
        if (m_parent != null) {
            sb.append(m_parent.getName()).append(Setting.PATH_DELIM);
        }
        sb.append(getName());
        
        return sb.toString();
    }
    
    public void setEnabled(Group g, boolean enable) {
        String path = Setting.PATH_DELIM + getSettingPath();
        g.getValues().put(path, enable ? ENABLE : DISABLE);
    }
    
    public Permission[] getChildren() {
        List list = EnumUtils.getEnumList(Permission.class);
        Iterator i = list.iterator();
        List children = new ArrayList();
        while (i.hasNext()) {
            Permission p = (Permission) i.next();
            if (p.getParent() == this) {
                children.add(p);
            }
        }
        
        return (Permission[]) children.toArray(new Permission[children.size()]);        
    }
}
