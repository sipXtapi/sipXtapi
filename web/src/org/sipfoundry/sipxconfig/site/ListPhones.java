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
import org.sipfoundry.sipxconfig.phone.PhoneDao;
import org.sipfoundry.sipxconfig.phone.PhoneSummary;
import org.sipfoundry.sipxconfig.phone.PhoneSummaryFactory;

/**
 * List all the phones/endpoints for management and details drill-down
 */
public abstract class ListPhones extends AbstractPhonePage 
        implements PageRenderListener, PhoneSummaryFactory {
    
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
        EditPhone page = (EditPhone) cycle.getPage(PAGE_EDIT_PHONE);
        page.setEndpoint(phone.getEndpoint());
        cycle.activate(page);
    }
    
    public void assignUser(IRequestCycle cycle) {
        PhoneListRow phone = getCurrentPhone();
        AssignUser page = (AssignUser) cycle.getPage(PAGE_ASSIGN_USER);
        if (phone.isAssigned()) {
            page.setAssignment(phone.getAssignment());
        } else {
            page.setEndpoint(phone.getEndpoint());
        }
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
