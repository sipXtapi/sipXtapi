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

import java.util.List;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;


/**
 * Comments
 */
public abstract class PhoneLines extends BasePage implements PageRenderListener {

    public static final String PAGE = "PhoneLines";

    public abstract Phone getPhone();
    
    public abstract void setPhone(Phone phone);
    
    public abstract int getPhoneId();
    
    /** REQUIRED PROPERTY */
    public abstract void setPhoneId(int id);

    public abstract List getLines();
    
    public abstract void setLines(List lines);
    
    public abstract Line getCurrentRow();

    public abstract void setCurrentRow(Line line);
    
    public abstract SelectMap getSelections();
    
    public abstract void setSelections(SelectMap selections);
    
    public void pageBeginRender(PageEvent event) {
        PhoneContext context = PhonePageUtils.getPhoneContext(event.getRequestCycle());
        Phone phone = context.getPhone(getPhoneId()); 
        setPhone(phone);
        setLines(phone.getEndpoint().getLines());
        
        // Generate the list of phone items
        if (getSelections() == null) {
            setSelections(new SelectMap());
        }
    }
    
    public void addUser(IRequestCycle cycle) {
        Object[] params = cycle.getServiceParameters();
        Integer endpointId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        AddPhoneUser page = (AddPhoneUser) cycle.getPage(AddPhoneUser.PAGE);
        page.setPhoneId(endpointId.intValue());
        cycle.activate(page);        
    }

    public void ok(IRequestCycle cycle) {
        apply(cycle);
        cycle.activate(ManagePhones.PAGE);
    }

    public void apply(IRequestCycle cycle) {
        PhoneContext dao = PhonePageUtils.getPhoneContext(cycle); 
        dao.storeEndpoint(getPhone().getEndpoint());
        dao.flush();
    }
    
    public void cancel(IRequestCycle cycle) {
        setPhone(null);
        cycle.activate(ManagePhones.PAGE);
    }
}
