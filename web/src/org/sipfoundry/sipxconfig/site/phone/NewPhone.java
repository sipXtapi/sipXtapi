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
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneData;


/**
 * First page of wizard-like UI for creating a new phone
 */
public abstract class NewPhone extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "NewPhone"; 

    public abstract PhoneData getPhoneData();
    
    public abstract void setPhoneData(PhoneData phone);
    
    public abstract PhoneContext getPhoneContext();
    
    public abstract boolean isStay();

    public void finish(IRequestCycle cycle) {
        IValidationDelegate delegate = (IValidationDelegate) getBeans().getBean("validator");
        boolean save = !delegate.getHasErrors();
        if (save) {
            Phone phone = getPhoneContext().newPhone(getPhoneData().getFactoryId());
            phone.setPhoneData(getPhoneData());
            getPhoneContext().storePhone(phone);        
            if (isStay()) {
                // triggers form to clear
                cycle.activate(PAGE);                    
            } else {
                cycle.activate(ManagePhones.PAGE);
            }
        }
    }
    
    public void next(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }
    
    public void pageBeginRender(PageEvent event_) {
        // always fresh object
        PhoneData meta = new PhoneData();
        meta.setFolder(getPhoneContext().loadRootPhoneFolder());
        setPhoneData(meta);
    }
}
