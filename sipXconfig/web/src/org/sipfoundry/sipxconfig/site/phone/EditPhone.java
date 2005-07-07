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

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

/**
 * Tapestry Page support for editing and creating new phone phones
 */
public abstract class EditPhone extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "EditPhone"; 

    public abstract Phone getPhone();
    
    public abstract void setPhone(Phone phone);
    
    /** REQUIRED PROPERTY */
    public abstract Integer getPhoneId();
    
    public abstract void setPhoneId(Integer id);
    
    public abstract PhoneContext getPhoneContext();
    
    public void ok(IRequestCycle cycle) {
        if (save()) {
            cycle.activate(ManagePhones.PAGE);
        }
    }

    public void addLine(IRequestCycle cycle) {
        Object[] params = cycle.getServiceParameters();
        Integer phoneId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        AddPhoneUser page = (AddPhoneUser) cycle.getPage(AddPhoneUser.PAGE);
        page.setReturnToEditPhone(true);
        page.setPhoneId(phoneId);
        cycle.activate(page);        
    }
    
    public void apply(IRequestCycle cycle_) {
        save();
    }
    
    private boolean save() {
        IValidationDelegate delegate = (IValidationDelegate) getBeans().getBean("validator");
        boolean save = !delegate.getHasErrors();
        if (save) {
            PhoneContext dao = getPhoneContext();
            dao.storePhone(getPhone());
        }
        
        return save;
    }
    
    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }
    
    public void pageBeginRender(PageEvent event_) {
        PhoneContext context = getPhoneContext();
        setPhone(context.loadPhone(getPhoneId()));
    }       
}
