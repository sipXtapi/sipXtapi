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
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneMetaData;


/**
 * First page of wizard-like UI for creating a new phone
 */
public abstract class NewPhone extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "NewPhone"; 

    public abstract PhoneMetaData getPhoneMetaData();
    
    public abstract void setPhoneMetaData(PhoneMetaData phone);
    
    public abstract PhoneContext getPhoneContext();

    public void finish(IRequestCycle cycle) {
        Phone phone = getPhoneContext().newPhone(getPhoneMetaData().getFactoryId());
        phone.setPhoneMetaData(getPhoneMetaData());
        getPhoneContext().storePhone(phone);
        cycle.activate(ManagePhones.PAGE);
    }

    public void next(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }
    
    public void pageBeginRender(PageEvent event_) {
        // always fresh object
        PhoneMetaData meta = new PhoneMetaData();
        meta.setFolder(getPhoneContext().loadRootPhoneFolder());
        setPhoneMetaData(meta);
    }
}
