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

import java.util.Arrays;
import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.site.line.LineSettings;

/**
 * Top portion of pages that show tabs, help box, intro text, etc
 */
public abstract class PhoneNavigation extends BaseComponent {
    
    public abstract Collection getSettingSet();

    public abstract void setSettingSet(Collection headings);
    
    public abstract void setPhone(Phone phone);
    
    public abstract Phone getPhone();
    
    public abstract void setCurrentSetting(String setting);

    public abstract String getCurrentSetting();
    
    public void editPhone(IRequestCycle cycle) {
        EditPhone page = (EditPhone) cycle.getPage(EditPhone.PAGE);
        page.setPhone(getPhone(cycle));
        cycle.activate(page);
    }
    
    public void editLines(IRequestCycle cycle) {
        PhoneLines page = (PhoneLines) cycle.getPage(PhoneLines.PAGE);
        page.setPhone(getPhone(cycle));
        cycle.activate(page);
    }
    
    public void editSettings(IRequestCycle cycle) {
        LineSettings page = (LineSettings) cycle.getPage(LineSettings.PAGE);
        page.setPhone(getPhone(cycle));
        Object[] params = cycle.getServiceParameters();
        String setting = (String) TapestryUtils.assertParameter(String.class, params, 1);
        page.setSetting(setting);
        cycle.activate(page);        
    }
    
    private Phone getPhone(IRequestCycle cycle) {
        Object[] params = cycle.getServiceParameters();
        Integer endpointId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        PhoneContext phoneContext = PhonePageUtils.getPhoneContext(cycle);
        return phoneContext.getPhone(endpointId.intValue());
    }
    
    /**
     * Used for contructing parameters for EditSettings DirectLink
     */
    public Object[] getEditSettingListenerParameters() {
        return new Object[] { 
            new Integer(getPhone().getEndpoint().getId()),
            getCurrentSetting() 
        };
    }
    
    public void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        // TODO: Build these from Endpoints, SettingsSet heirarchy
        String[] headings = new String[] {
            "Outbound Proxy", "Network" 
        };        
        setSettingSet(Arrays.asList(headings));
    }        
    
}
