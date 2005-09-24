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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.contrib.table.model.IPrimaryKeyConvertor;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.ObjectSourceDataSqueezer;
import org.sipfoundry.sipxconfig.components.SelectMap;

public abstract class UserTable extends BaseComponent implements PageRenderListener {
    
    public static final String COMPONENT = "UserTable";
    
    /** REQUIRED PROPERTY */
    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selected);
    
    public abstract CoreContext getCoreContext();
    
    public void setGroupId(Integer groupId) {
        UserTableModel model = getTableModel();
        model.setGroupId(groupId);                
    }
    
    // not sure why this is called
    public Integer getGroupId() {
        UserTableModel model = getTableModel();
        return model.getGroupId();                
    }
    
    public abstract UserTableModel getTableModel();
    
    public void pageBeginRender(PageEvent event_) {
        if (getSelections() == null) {
            setSelections(new SelectMap());            
        }        
    }

    /**
     * I cannot find out how to get this in ognl so I shamelessly defined a this getter
     * @return this
     */
    public IComponent getThis() {
        return this;
    }
    
    public IPrimaryKeyConvertor getIdConverter() {
        CoreContext context = getCoreContext();
        return new ObjectSourceDataSqueezer(context, User.class);
    }
}

