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
import java.util.List;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.User;


public abstract class AddPhoneUser extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "AddPhoneUser"; 

    public abstract Phone getPhone();
    
    public abstract void setPhone(Phone phone);
    
    /** REQUIRED PROPERTY */
    public abstract int getPhoneId();
    
    public abstract void setPhoneId(int id);
    
    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selected);

    public void select(IRequestCycle cycle) {
        PhoneLines page = (PhoneLines) cycle.getPage(PhoneLines.PAGE);
        page.setPhoneId(getPhoneId());
    
        PhoneContext context = PhonePageUtils.getPhoneContext(cycle);
        Endpoint endpoint = context.loadEndpoint(getPhoneId());
        List lines = endpoint.getLines();
        
        Iterator users = getSelections().getAllSelected().iterator();
        while (users.hasNext()) {
            User user = (User) users.next();
            Line line = new Line();
            line.setUser(user);
            line.setCredential(user.getCredential());
            lines.add(line);
        }
        context.storeEndpoint(endpoint);

        cycle.activate(page);
    }

    public void cancel(IRequestCycle cycle) {
        PhoneLines page = (PhoneLines) cycle.getPage(PhoneLines.PAGE);
        page.setPhoneId(getPhoneId());
        cycle.activate(page);
    }
    
    public void pageBeginRender(PageEvent event) {
        PhoneContext context = PhonePageUtils.getPhoneContext(event.getRequestCycle());
        setPhone(context.getPhone(getPhoneId()));
    }           
}
