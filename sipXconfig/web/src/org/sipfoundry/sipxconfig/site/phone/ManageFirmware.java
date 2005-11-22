/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import java.util.Collection;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.FirmwareManufacturer;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public abstract class ManageFirmware extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "ManageFirmware";

    public abstract void setFirmware(Collection firmware);

    public abstract Collection getFirmware();
    
    public abstract PhoneContext getPhoneContext();
    
    public abstract FirmwareManufacturer getSelectedManufacturer();
    
    public void editFirmware(IRequestCycle cycle) {
        Integer firmwareId = (Integer) TapestryUtils.assertParameter(Integer.class, 
                cycle.getServiceParameters(), 0);
        EditFirmware page = (EditFirmware) cycle.getPage(EditFirmware.PAGE);
        page.setFirmwareId(firmwareId);
        cycle.activate(page);
    }
    
    public void addFirmware(IRequestCycle cycle) {
        EditFirmware page = (EditFirmware) cycle.getPage(EditFirmware.PAGE);
        page.setFirmwareId(null);
        page.setManufacturer(getSelectedManufacturer());
        page.activatePageWithCallback(PAGE, cycle);
    }
    
    public void deleteFirmware(IRequestCycle cycle_) {
    }

    /** stub: side-effect of PageRenderListener */
    public void pageBeginRender(PageEvent event_) {
        if (getFirmware() == null) {
            setFirmware(getPhoneContext().getFirmware());
        }
    }
    
    public void pageEndRender(PageEvent event) { 
        if (getSelectedManufacturer() != null) {
            addFirmware(event.getRequestCycle());
        }
    }
}
