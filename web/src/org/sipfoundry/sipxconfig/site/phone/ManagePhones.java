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

import java.io.IOException;
import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.contrib.table.model.IPrimaryKeyConvertor;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
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
    
    public abstract void setCurrentLine(Line line);
    
    public abstract Line getCurrentLine();
    
    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selected);
    
    public abstract IPrimaryKeyConvertor getIdConverter();
    
    public abstract void setIdConverter(IPrimaryKeyConvertor cvt);

    /**
     * When user clicks on link to edit a phone/endpoint
     */
    public void editPhone(IRequestCycle cycle) {
        EditPhone page = (EditPhone) cycle.getPage(EditPhone.PAGE);
        Object[] params = cycle.getServiceParameters();
        Integer endpointId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        page.setEndpointId(endpointId.intValue());
        cycle.activate(page);
    }
    
    public void addPhone(IRequestCycle cycle) {
        NewPhone page = (NewPhone) cycle.getPage(NewPhone.PAGE);
        cycle.activate(page);
    }
    
    public void generateProfiles(IRequestCycle cycle) {
        // TODO: Should execute asychronously
        PhoneContext phoneContext = PhonePageUtils.getPhoneContext(cycle);
        
        SelectMap selections = getSelections();        
        Iterator endpointIds = selections.getAllSelected().iterator();
        while (endpointIds.hasNext()) {
            Integer endpointId = (Integer) endpointIds.next();
            Endpoint endpoint = phoneContext.loadEndpoint(endpointId.intValue());
            Phone phone = phoneContext.getPhone(endpoint);
            try {
                phone.generateProfiles(phoneContext, endpoint);
            } catch (IOException ioe) {
                throw new RuntimeException("Error generating profiles", ioe);
            }
        }
    }
    
    /**
     * called before page is drawn
     */
    public void pageBeginRender(PageEvent event) {
        PhoneContext phoneContext = PhonePageUtils.getPhoneContext(event.getRequestCycle());
        
        setIdConverter(new PhoneContextDataSqueezer(phoneContext, Endpoint.class));

        // Generate the list of phone items
        setPhones(phoneContext.loadPhoneSummaries());
        if (getSelections() == null) {
            setSelections(new SelectMap());
        }
    }
}
