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
package org.sipfoundry.sipxconfig.site.phone;

import java.util.Iterator;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.User;
import org.sipfoundry.sipxconfig.site.user.UserTable;


public abstract class AddPhoneUser extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "AddPhoneUser"; 

    public abstract Endpoint getEndpoint();
    
    public abstract void setEndpoint(Endpoint endpoint);
    
    /** REQUIRED PROPERTY */
    public abstract int getEndpointId();
    
    public abstract void setEndpointId(int id);
    
    public void select(IRequestCycle cycle) {
        PhoneContext context = PhonePageUtils.getPhoneContext(cycle);
        Endpoint endpoint = context.loadEndpoint(getEndpointId());
        
        UserTable table = (UserTable) getComponent("searchResults");
        SelectMap selections = table.getSelections();        
        Iterator usersIds = selections.getAllSelected().iterator();
        while (usersIds.hasNext()) {
            Integer userId = (Integer) usersIds.next();
            User user = context.loadUser(userId.intValue());            
            Line line = new Line();            
            line.setUser(user);
            line.setFolder(context.loadRootLineFolder());
            endpoint.addLine(line);
        }
        context.storeEndpoint(endpoint);
        context.flush();

        PhoneLines page = (PhoneLines) cycle.getPage(PhoneLines.PAGE);
        page.setEndpointId(getEndpointId());
        cycle.activate(page);
    }
    
    public void cancel(IRequestCycle cycle) {
        PhoneLines page = (PhoneLines) cycle.getPage(PhoneLines.PAGE);
        page.setEndpointId(getEndpointId());
        cycle.activate(page);
    }
    
    public void pageBeginRender(PageEvent event) {
        PhoneContext context = PhonePageUtils.getPhoneContext(event.getRequestCycle());
        setEndpoint(context.loadEndpoint(getEndpointId()));
    }           
}
