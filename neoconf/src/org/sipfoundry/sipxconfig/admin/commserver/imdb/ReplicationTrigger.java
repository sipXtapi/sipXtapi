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
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.io.Serializable;

import org.hibernate.type.Type;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.common.DaoEventListener;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;

public class ReplicationTrigger implements DaoEventListener {

    private SipxProcessContext m_processContext;

    public void setProcessContext(SipxProcessContext processContext) {
        m_processContext = processContext;
    }
    
    public SipxProcessContext getProcessContext() {
        return m_processContext;
    }

    public boolean onSave(Object entity, Serializable id, Object[] state, String[] propertyNames,
            Type[] types) {
        onSaveOrDelete(entity, id, state, propertyNames, types);
        return false;
    }

    public void onDelete(Object entity, Serializable id, Object[] state, String[] propertyNames, 
            Type[] types) {
        onSaveOrDelete(entity, id, state, propertyNames, types);
    }
    
    void onSaveOrDelete(Object entity, Serializable id_, Object[] state_, String[] propertyNames_, 
            Type[] types_) {

        Class c = entity.getClass();
        if (Group.class.equals(c)) {
            Group group = (Group) entity;
            if ("user".equals(group.getResource())) {
                m_processContext.generate(DataSet.PERMISSION);
            }
        } else if (User.class.equals(c)) {
            m_processContext.generateAll();
        }                
    }
}
