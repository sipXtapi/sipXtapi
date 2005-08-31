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
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.ObjectSourceDataSqueezer;
import org.sipfoundry.sipxconfig.components.SelectMap;

public abstract class UserTable extends BaseComponent implements PageRenderListener {
    
    public static final String COMPONENT = "UserTable";
    
    /** REQUIRED PROPERTY */
    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selected);
    
    public abstract List getUsers();
    
    /** 
     * REQUIRED PROPERTY, reference cannot be changed after page rewind. i.e. any
     * query results need to clear list reference then add new user objects 
     * */
    public abstract void setUsers(List users);
    
    public abstract CoreContext getCoreContext();
    
    public void pageBeginRender(PageEvent event_) {
        if (getSelections() == null) {
            setSelections(new SelectMap());            
        }
    }
    
    public IPrimaryKeyConvertor getIdConverter() {
        CoreContext context = getCoreContext();
        return new ObjectSourceDataSqueezer(context, User.class);
    }
}

