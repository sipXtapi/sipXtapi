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

import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Phone;

/**
 * List all the phones/endpoints for management and details drill-down
 */
public abstract class ListPhones extends AbstractPhonePage 
		implements PageRenderListener {

    /** Serial number column */
    public static final String SERIAL_NUMBER = "Serial Number";
    
    // Return the model of the table
    public abstract List getPhones();
    
    public abstract void setPhones(List phones);
    
    public abstract PhoneItem getCurrentPhone();

    public abstract void setCurrentPhone(PhoneItem currentPhone);  
    
    /**
     * called before page is drawn
     */
    public void pageBeginRender(PageEvent event) {
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
