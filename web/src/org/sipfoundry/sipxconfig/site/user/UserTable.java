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
package org.sipfoundry.sipxconfig.site.user;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.contrib.table.model.IPrimaryKeyConvertor;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.User;
import org.sipfoundry.sipxconfig.site.phone.PhonePageUtils;

public abstract class UserTable extends BaseComponent implements PageRenderListener {
    
    public static final String COMPONENT = "UserTable";
    
    private IPrimaryKeyConvertor m_idConverter;
    
    /** REQUIRED PROPERTY */
    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selected);
    
    public abstract List getUsers();
    
    /** 
     * REQUIRED PROPERTY, reference cannot be changed after page rewind. i.e. any
     * query results need to clear list reference then add new user objects 
     * */
    public abstract void setUsers(List users);
    
    public void pageBeginRender(PageEvent event) {
        if (getSelections() == null) {
            setSelections(new SelectMap());            
        }
        PhoneContext context = PhonePageUtils.getPhoneContext(event.getRequestCycle());
        m_idConverter = new UserByIdConverter(context);
    }
    
    public IPrimaryKeyConvertor getIdConverter() {
        return m_idConverter;
    }

    static class UserByIdConverter implements IPrimaryKeyConvertor  {
        
        private PhoneContext m_context;
        
        public UserByIdConverter(PhoneContext context) {
            m_context = context;
        }
        
        public Object getPrimaryKey(Object objValue) {
            return new Integer(((User) objValue).getId());
        }

        public Object getValue(Object objPrimaryKey) {
            Integer id = (Integer) objPrimaryKey;
            return m_context.loadUser(id.intValue());
        }            
    }
}

