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

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneDao;

/**
 * List all the phones/endpoints for management and details drill-down
 */
public abstract class ListPhones extends AbstractPhonePage 
        implements PageRenderListener {

    // Return the model of the table
    public abstract List getPhones();
    
    public abstract void setPhones(List phones);
    
    public abstract PhoneItem getCurrentPhone();

    public abstract void setCurrentPhone(PhoneItem currentPhone);
    
    /**
     * When user clicks on link to edit a phone/endpoint
     */
    public void editPhone(IRequestCycle cycle) {
        EditPhone page = (EditPhone) cycle.getPage(PAGE_EDIT_PHONE);
        page.setEndpoint(getEndpoint(cycle));
        cycle.activate(page);
    }
    
    public void assignUser(IRequestCycle cycle) {
        AssignUser page = (AssignUser) cycle.getPage(PAGE_ASSIGN_USER);
        page.setEndpoint(getEndpoint(cycle));
        cycle.activate(page);
    }
    
    /**
     * load from dao, but optionally could find it in list
     */
    private Endpoint getEndpoint(IRequestCycle cycle) {
        Object[] params = cycle.getServiceParameters();
        Integer endpointId = (Integer) params[0];
        PhoneDao dao = getPhoneContext().getPhoneDao();
        return dao.loadEndpoint(endpointId.intValue());        
    }
    
    /**
     * called before page is drawn
     */
    public void pageBeginRender(PageEvent eventTemp) {
        // Generate the list of phone items
        List endpointList = getPhoneContext().getPhoneDao().loadEndpoints();
        List phones = new ArrayList(endpointList.size());
        Iterator i =  endpointList.iterator();
        while (i.hasNext()) {
            Endpoint endpoint = (Endpoint) i.next();
            Phone phone = getPhoneContext().getPhone(endpoint);
            phones.add(new PhoneItem(phone));
        }
        setPhones(phones);
    }    
}
