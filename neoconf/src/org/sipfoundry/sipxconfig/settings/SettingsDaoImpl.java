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
package org.sipfoundry.sipxconfig.settings;

import java.util.Iterator;

import org.springframework.orm.hibernate.support.HibernateDaoSupport;

public class SettingsDaoImpl extends HibernateDaoSupport implements SettingsDao {

    public void storeSetting(Setting setting, int depth) {        
        getHibernateTemplate().saveOrUpdate(setting);
        if (depth > 0 || depth == CASCADE && setting.getSettings().size() > 0) {
            Iterator children = setting.getSettings().values().iterator();
            while (children.hasNext()) {
                Setting child = (Setting) children.next();
                storeSetting(child, depth - 1);                    
            }
        }
    }
    
    public void storeSetting(Setting setting) {
        storeSetting(setting, 0);
    }

    public SettingSet loadSettings(int id) {
        return (SettingSet) getHibernateTemplate().load(SettingSet.class, new Integer(id));
    }
}
