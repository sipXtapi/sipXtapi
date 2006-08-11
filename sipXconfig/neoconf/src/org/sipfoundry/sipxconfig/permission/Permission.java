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
package org.sipfoundry.sipxconfig.permission;

import java.util.List;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.apache.commons.lang.enums.EnumUtils;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Permission Copy of permission setting names exist in user-setting.xml
 */
public final class Permission {
    enum Type {
        APPLICATION("application"), CALL("call-handling");

        private String m_name;

        Type(String name) {
            m_name = name;
        }

        public String getName() {
            return m_name;
        }

        Permission create(String name) {
            return new Permission(this, name);
        }
    }

    public static final String ENABLE = "ENABLE";
    public static final String DISABLE = "DISABLE";

    public static final String PATH_PREFIX = "permission/";
    public static final String CALL_PERMISSION_PATH = PATH_PREFIX + Type.CALL.getName();

    public static final Permission SUPERADMIN = Type.APPLICATION.create("superadmin");
    public static final Permission TUI_CHANGE_PIN = Type.APPLICATION.create("tui-change-pin");

    public static final Permission NO_ACCESS = Type.CALL.create("NoAccess");
    public static final Permission VOICEMAIL = Type.CALL.create("Voicemail");
    public static final Permission RESTRICTED_DIALING = Type.CALL.create("900Dialing");
    public static final Permission TOLL_FREE_DIALING = Type.CALL.create("TollFree");
    public static final Permission LONG_DISTANCE_DIALING = Type.CALL
            .create("LongDistanceDialing");
    public static final Permission INTERNATIONAL_DIALING = Type.CALL
            .create("InternationalDialing");
    public static final Permission LOCAL_DIALING = Type.CALL.create("LocalDialing");
    public static final Permission FORWARD_CALLS_EXTERNAL = Type.CALL
            .create("ForwardCallsExternal");

    private Type m_type;

    private String m_label;

    private String m_description;

    private String m_name;

    Permission(Type type, String name) {
        m_type = type;
        m_name = name;
    }

    public static boolean isEnabled(String value) {
        return ENABLE.equals(value);
    }

    /**
     * Returns the path in user settings to this permission group or setting
     */
    public String getSettingPath() {
        StringBuilder sb = new StringBuilder(PATH_PREFIX);
        sb.append(m_type.getName()).append(Setting.PATH_DELIM);
        sb.append(getName());

        return sb.toString();
    }

    public void setEnabled(Group g, boolean enable) {
        String path = getSettingPath();
        g.getValues().put(path, enable ? ENABLE : DISABLE);
    }

    public static List<Permission> getEnumList() {
        return EnumUtils.getEnumList(Permission.class);
    }

    public void setLabel(String label) {
        m_label = label;
    }

    public String getLabel() {
        return m_label;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getDescription() {
        return m_description;
    }

    public String getName() {
        return m_name;
    }

    public boolean equals(Object obj) {
        if (!(obj instanceof Permission)) {
            return false;
        }
        if (this == obj) {
            return true;
        }
        Permission rhs = (Permission) obj;
        // only compare name and type
        return new EqualsBuilder().append(m_type, rhs.m_type).append(m_name, rhs.m_name)
                .isEquals();

    }

    public int hashCode() {
        return new HashCodeBuilder().append(m_name).append(m_type).hashCode();
    }
}
