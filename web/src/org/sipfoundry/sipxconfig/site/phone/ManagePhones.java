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
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneMetaData;
import org.sipfoundry.sipxconfig.site.line.EditLine;

/**
 * List all the phones/phones for management and details drill-down
 */
public abstract class ManagePhones extends BasePage 
        implements PageRenderListener {
    
    public static final String PAGE = "ManagePhones";
    
    /** model of the table */
    public abstract void setPhones(List phones);
    
    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selected);
    
    public abstract void setIdConverter(IPrimaryKeyConvertor cvt);

    public abstract PhoneContext getPhoneContext();

    /**
     * When user clicks on link to edit a phone/phone
     */
    public void editPhone(IRequestCycle cycle) {
        EditPhone page = (EditPhone) cycle.getPage(EditPhone.PAGE);
        Object[] params = cycle.getServiceParameters();
        Integer phoneId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        page.setPhoneId(phoneId);
        cycle.activate(page);
    }
    
    public void editLine(IRequestCycle cycle) {
        Object[] params = cycle.getServiceParameters();
        Integer lineId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        EditLine page = (EditLine) cycle.getPage(EditLine.PAGE);
        page.setLineId(lineId);
        cycle.activate(page);                
    }

    public void addPhone(IRequestCycle cycle) {
        NewPhone page = (NewPhone) cycle.getPage(NewPhone.PAGE);
        cycle.activate(page);
    }
    
    public void defaults(IRequestCycle cycle) {
        PhoneDefaults page = (PhoneDefaults) cycle.getPage(PhoneDefaults.PAGE);
        cycle.activate(page);
    }
    
    public void generateProfiles(IRequestCycle cycle_) {
        // TODO: Should execute asychronously and submit job
        // to job database table 
        PhoneContext phoneContext = getPhoneContext();
        
        SelectMap selections = getSelections();        
        Iterator phoneIds = selections.getAllSelected().iterator();
        while (phoneIds.hasNext()) {
            Integer phoneId = (Integer) phoneIds.next();
            Phone phone = phoneContext.loadPhone(phoneId);
            try {
                phone.generateProfiles();
            } catch (IOException ioe) {
                throw new RuntimeException("Error generating profiles", ioe);
            }
        }
    }
    
    /**
     * called before page is drawn
     */
    public void pageBeginRender(PageEvent event_) {
        PhoneContext phoneContext = getPhoneContext();
        
        setIdConverter(new PhoneDataSqueezer(phoneContext));

        // Generate the list of phone items
        setPhones(phoneContext.loadPhones());
        if (getSelections() == null) {
            setSelections(new SelectMap());
        }
    }
    
    /**
     * PhoneSummary is not a make up object contructed of and phone and a phone
     * object.  reconstruct it here from phone and phonecontext
     */
    static class PhoneDataSqueezer extends PhoneContextDataSqueezer {
        
        PhoneDataSqueezer(PhoneContext context) {
            super(context, PhoneMetaData.class);
        }

        public Object getPrimaryKey(Object objValue) {
            Object pk = null;
            if (objValue != null) {
                pk = ((Phone) objValue).getPhoneMetaData().getPrimaryKey();
            }
            
            return pk;
        }

        public Object getValue(Object objPrimaryKey) {           
            PhoneMetaData phoneMeta = (PhoneMetaData) super.getValue(objPrimaryKey);
            // reload object due to PhoneContext API (good) restriction
            Phone phone = getPhoneContext().loadPhone(phoneMeta.getId());
            
            return phone;
        }
    }
}
