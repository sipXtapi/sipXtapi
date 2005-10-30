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
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;


public class Gateway2Id implements IPrimaryKeyConvertor {
    private GatewayContext m_context;
    
    public Object getPrimaryKey(Object objValue) {
        Gateway g = (Gateway) objValue;
        return g.getId();
    }
    
    public Object getValue(Object objPrimaryKey) {
        Integer id = (Integer) objPrimaryKey;
        return m_context.getGateway(id);
    }
    
    public void setContext(GatewayContext context) {
        m_context = context;
    }
}
