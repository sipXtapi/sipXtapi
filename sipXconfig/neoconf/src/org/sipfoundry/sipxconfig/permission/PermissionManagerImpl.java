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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;

import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.setting.Setting;

public class PermissionManagerImpl extends SipxHibernateDaoSupport implements PermissionManager {

    private ModelFilesContext m_modelFilesContext;

    public void addCallPermission(Permission permission) {
        getHibernateTemplate().saveOrUpdate(permission);
    }

    public Collection<Permission> getCallPermissions() {
        return getAllPermissions();
    }

    public void removeCallPermissions(Collection<String> permissionNames) {
        Map<String, Permission> builtInPermissions = getBuiltInPermissions();
        Collection toBeRemoved = new ArrayList(permissionNames.size());
        for (String name : permissionNames) {
            if (builtInPermissions.containsKey(name)) {
                // ignore build in permissions
                continue;
            }
            Object permission = getHibernateTemplate().load(Permission.class, name);
            toBeRemoved.add(permission);
        }
        getHibernateTemplate().deleteAll(toBeRemoved);
    }

    public Permission getPermission(String name) {
        Set<Permission> allPermissions = getAllPermissions();
        for (Permission permission : allPermissions) {
            if (permission.getName().equals(name)) {
                return permission;
            }
        }
        return null;
    }

    public void setModelFilesContext(ModelFilesContext modelFilesContext) {
        m_modelFilesContext = modelFilesContext;
    }

    /**
     * Combine built-in and custom permissions into a single permission model
     */
    public Setting getPermissionModel() {
        Setting settings = loadSettings();
        Setting callHandlingGroup = settings.getSetting(Permission.CALL_PERMISSION_PATH);
        Collection<Permission> callPermissions = loadCustomPermissions();
        for (Permission permission : callPermissions) {
            callHandlingGroup.addSetting(permission.getSetting());
        }
        return settings;
    }

    /**
     * Loads permission settings and creates Permissions collection
     */
    private Map<String, Permission> getBuiltInPermissions() {
        Setting userSettingsModel = loadSettings();
        Map<String, Permission> permissions = new TreeMap<String, Permission>();
        Setting callHandlingGroup = userSettingsModel.getSetting(Permission.CALL_PERMISSION_PATH);
        for (Setting setting : callHandlingGroup.getValues()) {
            permissions.put(setting.getName(), new Permission(setting));
        }
        return permissions;
    }

    /**
     * Load all permissions from the DB
     * 
     * HACK: In order to make this function work in non-DB test we return empty set of permissions in
     * case hibernate session factory is not set.
     */
    private Collection<Permission> loadCustomPermissions() {
        if (getSessionFactory() != null) {
            return getHibernateTemplate().loadAll(Permission.class);
        }
        return Collections.emptyList();
    }

    private Set<Permission> getAllPermissions() {
        Set<Permission> permissions = new TreeSet<Permission>();
        permissions.addAll(getBuiltInPermissions().values());
        permissions.addAll(loadCustomPermissions());
        return permissions;
    }

    private Setting loadSettings() {
        return m_modelFilesContext.loadModelFile("user-settings.xml");
    }
}
