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
package org.sipfoundry.sipxconfig.components;

import java.util.Arrays;
import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneDao;
import org.sipfoundry.sipxconfig.site.EditPhone;
import org.sipfoundry.sipxconfig.site.EditPhoneLines;
import org.sipfoundry.sipxconfig.site.EditSettings;

/**
 * Top portion of pages that show tabs, help box, intro text, etc
 */
public abstract class EditPhoneNavigation extends BaseComponent {
    
    public abstract Collection getSettingSet();

    public abstract void setSettingSet(Collection headings);
    
    public abstract void setPhone(Phone phone);
    
    public abstract Phone getPhone();
    
    public abstract void setCurrentSetting(String setting);

    public abstract String getCurrentSetting();
    
    public abstract PhoneContext getPhoneContext();

    public abstract void setPhoneContext(PhoneContext phoneContext);

    public void editPhone(IRequestCycle cycle) {
        EditPhone page = (EditPhone) cycle.getPage(EditPhone.PAGE);
        Object[] params = (Object[]) cycle.getServiceParameters();
        page.setPhone(getPhone(0, params));
        cycle.activate(page);
    }
    
    public void editLines(IRequestCycle cycle) {
        EditPhoneLines page = (EditPhoneLines) cycle.getPage(EditPhoneLines.PAGE);
        Object[] params = (Object[]) cycle.getServiceParameters();
        page.setPhone(getPhone(0, params));
        cycle.activate(page);
    }
    
    public void editSettings(IRequestCycle cycle) {
        EditSettings page = (EditSettings) cycle.getPage(EditSettings.PAGE);
        Object[] params = (Object[]) cycle.getServiceParameters();
        page.setPhone(getPhone(0, params));
        page.setSetting((String) params[1]);
        cycle.activate(page);        
    }

    private Phone getPhone(int index, Object[] params) {
        Integer endpointId = (Integer) params[index];
        PhoneDao dao = getPhoneContext().getPhoneDao();
        Endpoint endpoint = dao.loadEndpoint(endpointId.intValue());
        
        return getPhoneContext().getPhone(endpoint);
    }
    
    public void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        String[] headings = new String[] {
            "Outbound Proxy", "Network" 
        };        
        setSettingSet(Arrays.asList(headings));
    }        
    
}
