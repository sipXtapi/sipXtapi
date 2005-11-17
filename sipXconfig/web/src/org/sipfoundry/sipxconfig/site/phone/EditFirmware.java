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

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.phone.Firmware;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public abstract class EditFirmware extends PageWithCallback implements PageRenderListener {
    
    public static final String PAGE = "EditFirmware";
    
    public abstract Firmware getFirmware();
    
    public abstract void setFirmware(Firmware firmware);
    
    public abstract Integer getFirmwareId();
    
    public abstract void setFirmwareId(Integer id);
    
    public abstract PhoneContext getPhoneContext();

    public void pageBeginRender(PageEvent event_) {
        Firmware firmware = getFirmware();
        if (firmware == null) {
            Integer id = getFirmwareId();
            if (id == null) {
                firmware = new Firmware();
            } else {
                firmware = getPhoneContext().loadFirmware(id);
            }
            setFirmware(firmware);
        }
    }
    
    public void onSave(IRequestCycle cycle_) {
        getPhoneContext().saveFirmware(getFirmware());
    }
    
    public void onDelete(IRequestCycle cycle_) {
        getPhoneContext().saveFirmware(getFirmware());
    }
}
