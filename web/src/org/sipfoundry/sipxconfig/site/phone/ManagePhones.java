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
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneSummary;

/**
 * List all the phones/endpoints for management and details drill-down
 */
public abstract class ManagePhones extends BasePage 
        implements PageRenderListener {
    
    public static final String PAGE = "ManagePhones";
    
    // Return the model of the table
    public abstract List getPhones();
    
    public abstract void setPhones(List phones);
    
    public abstract PhoneSummary getCurrentRow();

    public abstract void setCurrentRow(PhoneSummary currentPhone);
    
    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selected);
    
    /**
     * When user clicks on link to edit a phone/endpoint
     */
    public void editPhone(IRequestCycle cycle) {
        EditPhone page = (EditPhone) cycle.getPage(EditPhone.PAGE);
        Object[] params = cycle.getServiceParameters();
        Integer endpointId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        page.setPhoneId(endpointId.intValue());
        cycle.activate(page);
    }
    
    public void addPhone(IRequestCycle cycle) {
        NewPhone page = (NewPhone) cycle.getPage(NewPhone.PAGE);
        cycle.activate(page);
    }
    
    /**
     * called before page is drawn
     */
    public void pageBeginRender(PageEvent event) {
        PhoneContext phoneContext = PhonePageUtils.getPhoneContext(event.getRequestCycle());

        // Generate the list of phone items
        setPhones(phoneContext.loadPhoneSummaries());
        if (getSelections() == null) {
            setSelections(new SelectMap());
        }
    }
}
