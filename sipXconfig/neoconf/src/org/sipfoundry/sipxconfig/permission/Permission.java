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
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.PrimaryKeySource;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.type.BooleanSetting;

/**
 * Permission Copy of permission setting names exist in user-setting.xml
 */
public final class Permission implements Comparable<Permission>, PrimaryKeySource {
    public enum Type {
        APPLICATION("application"), CALL("call-handling");

        private String m_name;

        Type(String name) {
            m_name = name;
        }

        public String getName() {
            return m_name;
        }

        public Permission create(String name) {
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

    private Type m_type = Type.CALL;

    private String m_label;

    private String m_description;

    private String m_name;

    private boolean m_defaultValue;

    private final boolean m_builtIn;

    private Integer m_id = BeanWithId.UNSAVED_ID;

    public Permission(Type type, String name) {
        m_type = type;
        m_name = name;
        m_builtIn = true;
    }

    public Permission() {
        m_builtIn = false;
    }

    public Permission(Setting setting) {
        m_name = setting.getName();
        m_label = setting.getLabel();
        m_description = setting.getDescription();
        m_builtIn = true;
        m_defaultValue = isEnabled(setting.getValue());
    }

    void setId(Integer id) {
        m_id = id;
    }

    public Integer getId() {
        return m_id;
    }

    public static boolean isEnabled(String value) {
        return (Boolean) PermissionSetting.INSTANCE.convertToTypedValue(value);
    }

    public boolean isBuiltIn() {
        return m_builtIn;
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

    public void setDefaultValue(boolean defaultValue) {
        m_defaultValue = defaultValue;
    }

    public boolean getDefaultValue() {
        return m_defaultValue;
    }

    public String getName() {
        if (m_builtIn) {
            return m_name;
        }
        return "perm_" + getId();
    }

    public Object getPrimaryKey() {
        return m_builtIn ? m_name : m_id;
    }

    public Type getType() {
        return m_type;
    }

    public void setType(Type type) {
        m_type = type;
    }

    public boolean equals(Object obj) {
        if (!(obj instanceof Permission)) {
            return false;
        }
        if (this == obj) {
            return true;
        }
        Permission rhs = (Permission) obj;
        // only compare name and type - make sure that you compare names using getName (and not m_name)        
        return new EqualsBuilder().append(m_type, rhs.m_type).append(getName(), rhs.getName())
                .isEquals();

    }

    public int hashCode() {
        return new HashCodeBuilder().append(getName()).append(m_type).hashCode();
    }

    public int compareTo(Permission o) {
        return getName().compareTo(o.getName());
    }

    /**
     * Create setting that corresponds to this permission
     * 
     * @return newly created setting
     */
    Setting getSetting() {
        SettingImpl setting = new SettingImpl(getName());
        setting.setType(PermissionSetting.INSTANCE);
        setting.setDescription(m_description);
        setting.setLabel(m_label);
        setting.setTypedValue(m_defaultValue);
        return setting;
    }

    private static class PermissionSetting extends BooleanSetting {
        public static final PermissionSetting INSTANCE = new PermissionSetting();

        public PermissionSetting() {
            setTrueValue(ENABLE);
            setFalseValue(DISABLE);
        }
    }
}
