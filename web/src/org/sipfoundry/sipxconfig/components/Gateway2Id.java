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
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.contrib.table.model.IPrimaryKeyConvertor;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;


public class Gateway2Id implements IPrimaryKeyConvertor {
    private DialPlanContext m_manager;
    
    public Object getPrimaryKey(Object objValue) {
        Gateway g = (Gateway) objValue;
        return g.getId();
    }
    
    public Object getValue(Object objPrimaryKey) {
        Integer id = (Integer) objPrimaryKey;
        return m_manager.getGateway(id);
    }
    
    public DialPlanContext getManager() {
        return m_manager;
    }
    public void setManager(DialPlanContext manager) {
        m_manager = manager;
    }
}