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
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.site.user.UserTable;


public abstract class AddPhoneUser extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "AddPhoneUser"; 

    public abstract Phone getPhone();
    
    public abstract void setPhone(Phone phone);
    
    /** REQUIRED PROPERTY */
    public abstract Integer getPhoneId();
    
    public abstract void setPhoneId(Integer id);
    
    public abstract PhoneContext getPhoneContext();
    
    public abstract CoreContext getCoreContext();
    
    public void select(IRequestCycle cycle) {
        PhoneContext context = getPhoneContext();
        Phone phone = context.loadPhone(getPhoneId());
        
        UserTable table = (UserTable) getComponent("searchResults");
        SelectMap selections = table.getSelections();        
        Iterator usersIds = selections.getAllSelected().iterator();
        while (usersIds.hasNext()) {
            Integer userId = (Integer) usersIds.next();
            User user = getCoreContext().loadUser(userId.intValue());            
            Line line = phone.createLine();            
            line.getLineMetaData().setUser(user);
            line.getLineMetaData().setFolder(context.loadRootLineFolder());
            phone.addLine(line);
        }
        context.storePhone(phone);
        context.flush();

        PhoneLines page = (PhoneLines) cycle.getPage(PhoneLines.PAGE);
        page.setPhoneId(getPhoneId());
        cycle.activate(page);
    }
    
    public void cancel(IRequestCycle cycle) {
        PhoneLines page = (PhoneLines) cycle.getPage(PhoneLines.PAGE);
        page.setPhoneId(getPhoneId());
        cycle.activate(page);
    }
    
    public void pageBeginRender(PageEvent event_) {
        setPhone(getPhoneContext().loadPhone(getPhoneId()));
    }           
}
