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
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.Iterator;
import java.util.List;

import org.dom4j.Element;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingVisitor;

public class Permissions extends DataSetGenerator {

    /**
     * Adds <item> <identity>user_uri</identity> <permission>permission_name</permission>
     * </item>
     * 
     * to the list of items.
     */
    protected void addItems(Element items) {
        String domain = getSipDomain();
        CoreContext coreContext = getCoreContext();
        List list = coreContext.loadUsers();
        for (Iterator i = list.iterator(); i.hasNext();) {
            User user = (User) i.next();
            addUser(items, user, domain);
        }
    }

    void addUser(Element item, User user, String domain) {
        Setting permissions = user.getSettings().getSetting(Permission.CALL_PERMISSION_PATH);
        PermissionWriter writer = new PermissionWriter(user, domain, item);
        permissions.acceptVisitor(writer);
    }

    class PermissionWriter implements SettingVisitor {

        private User m_user;

        private Element m_items;

        private String m_domain;

        PermissionWriter(User user, String domain, Element items) {
            m_user = user;
            m_items = items;
            m_domain = domain;
        }

        public void visitSetting(Setting setting) {
            if (Permission.isEnabled(setting.getValue())) {
                Element item = addItem(m_items);
                item.addElement("identity").setText(m_user.getUri(m_domain));
                item.addElement("permission").setText(setting.getName());
            }
        }

        public boolean visitSettingGroup(Setting group_) {
            return true;
        }
    }

    protected DataSet getType() {
        return DataSet.PERMISSION;
    }
}
