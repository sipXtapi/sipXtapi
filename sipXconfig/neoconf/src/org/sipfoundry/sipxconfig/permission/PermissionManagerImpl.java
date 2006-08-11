/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.permission;

import java.util.Collection;
import java.util.Map;
import java.util.TreeMap;

import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class PermissionManagerImpl implements PermissionManager, ApplicationListener {

    private Map<String, Permission> m_permissions = new TreeMap<String, Permission>();

    private CoreContext m_coreContext;

    public void addCallPermission(Permission permission) {
        m_permissions.put(permission.getName(), permission);
    }

    public Collection<Permission> getCallPermissions() {
        return m_permissions.values();
    }

    public void removeCallPermissions(Collection<String> permissionNames) {
        for (String name : permissionNames) {
            m_permissions.remove(name);
        }
    }

    public Permission getPermission(String name) {
        return m_permissions.get(name);
    }

    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof ApplicationInitializedEvent) {
            init();
        }
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void init() {
        Setting userSettingsModel = m_coreContext.getUserSettingsModel();
        Setting callHandlingGroup = userSettingsModel.getSetting(Permission.CALL_PERMISSION_PATH);
        callHandlingGroup.acceptVisitor(new PermissionCreator(this));
    }

    static class PermissionCreator extends AbstractSettingVisitor {
        private PermissionManager m_manager;

        public PermissionCreator(PermissionManager manager) {
            m_manager = manager;
        }

        public void visitSetting(Setting setting) {
            String name = setting.getName();
            Permission permission = m_manager.getPermission(name);
            if (permission == null) {
                permission = new Permission(Permission.Type.CALL, name);
                m_manager.addCallPermission(permission);
            }
            permission.setLabel(setting.getLabel());
            permission.setDescription(setting.getDescription());
        }
    }
}
