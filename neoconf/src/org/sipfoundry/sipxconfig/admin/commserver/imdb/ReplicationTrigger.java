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

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.setting.Group;

public class ReplicationTrigger implements DaoEventListener {

    private SipxReplicationContext m_replicationContext;

    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    public SipxReplicationContext getReplicationContext() {
        return m_replicationContext;
    }

    public void onSave(Object entity) {
        onSaveOrDelete(entity);
    }

    public void onDelete(Object entity) {
        onSaveOrDelete(entity);
    }

    void onSaveOrDelete(Object entity) {
        Class c = entity.getClass();
        if (Group.class.equals(c)) {
            Group group = (Group) entity;
            if ("user".equals(group.getResource())) {
                m_replicationContext.generate(DataSet.PERMISSION);
            }
        } else if (User.class.equals(c)) {
            m_replicationContext.generateAll();
        }
    }
}
