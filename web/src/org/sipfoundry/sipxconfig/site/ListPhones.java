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
package org.sipfoundry.sipxconfig.site;

import java.util.List;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.PhoneDao;
import org.sipfoundry.sipxconfig.phone.PhoneSummary;
import org.sipfoundry.sipxconfig.phone.PhoneSummaryFactory;

/**
 * List all the phones/endpoints for management and details drill-down
 */
public abstract class ListPhones extends AbstractPhonePage 
        implements PageRenderListener, PhoneSummaryFactory {
    
    public static final String PAGE = "ListPhones";

    public PhoneSummary createPhoneSummary() {
        return new PhoneListRow();
    }
    
    // Return the model of the table
    public abstract List getPhones();
    
    public abstract void setPhones(List phones);
    
    public abstract PhoneListRow getCurrentPhone();

    public abstract void setCurrentPhone(PhoneListRow currentPhone);
    
    /**
     * When user clicks on link to edit a phone/endpoint
     */
    public void editPhone(IRequestCycle cycle) {
        PhoneListRow phone = getCurrentPhone();
        EditPhone page = (EditPhone) cycle.getPage(EditPhone.PAGE);
        page.setPhone(phone.getPhone());
        cycle.activate(page);
    }
    
    public void addPhone(IRequestCycle cycle) {
        NewPhone page = (NewPhone) cycle.getPage(NewPhone.PAGE);
        page.setEndpoint(new Endpoint());
        cycle.activate(page);
    }
    
    /**
     * called before page is drawn
     */
    public void pageBeginRender(PageEvent eventTemp) {
        // Generate the list of phone items
        PhoneDao dao = getPhoneContext().getPhoneDao();
        setPhones(dao.loadPhoneSummaries(this));        
    }    
}
