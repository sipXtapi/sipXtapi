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
import org.sipfoundry.sipxconfig.phone.LineData;
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
    
    /** 
     * made no attempts to abstract this to return to generic page
     * because return page needs to get state (phone id) returned
     * This should be replaced with generic BreadCrumbs or PageFlowGraph
     * utility class when it's invented.
     */
    public abstract void setReturnToEditPhone(boolean rtn);
    
    public abstract boolean getReturnToEditPhone();
    
    public void select(IRequestCycle cycle) {
        PhoneContext context = getPhoneContext();
        Phone phone = context.loadPhone(getPhoneId());
        
        UserTable table = (UserTable) getComponent("searchResults");
        SelectMap selections = table.getSelections();        
        Iterator usersIds = selections.getAllSelected().iterator();
        while (usersIds.hasNext()) {
            Integer userId = (Integer) usersIds.next();
            User user = getCoreContext().loadUser(userId.intValue());            
            Line line = phone.createLine(new LineData());            
            line.getLineData().setUser(user);
            line.getLineData().setFolder(context.loadRootLineFolder());
            phone.addLine(line);
        }
        context.storePhone(phone);
        context.flush();

        PhoneLines page = (PhoneLines) cycle.getPage(PhoneLines.PAGE);
        page.setPhoneId(getPhoneId());
        cycle.activate(page);
    }
    
    public void cancel(IRequestCycle cycle) {
        if (getReturnToEditPhone()) {
            EditPhone page = (EditPhone) cycle.getPage(EditPhone.PAGE);
            page.setPhoneId(getPhoneId());
            cycle.activate(page);        
        } else {
            PhoneLines page = (PhoneLines) cycle.getPage(PhoneLines.PAGE);
            page.setPhoneId(getPhoneId());
            cycle.activate(page);
        }
    }
    
    public void pageBeginRender(PageEvent event_) {
        setPhone(getPhoneContext().loadPhone(getPhoneId()));
    }           
}
