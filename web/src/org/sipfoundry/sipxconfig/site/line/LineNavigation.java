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
package org.sipfoundry.sipxconfig.site.line;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Setting;

public abstract class LineNavigation extends BaseComponent {

    /** REQUIRED PARAMETER */
    public abstract void setLine(Line line);
    
    public abstract Line getLine();
    
    public abstract void setSettings(Setting settings);
    
    public abstract Setting getSettings();

    public abstract void setCurrentSetting(Setting setting);

    public abstract Setting getCurrentSetting();
    
    public abstract PhoneContext getPhoneContext();

    public void editLine(IRequestCycle cycle) {
        EditLine page = (EditLine) cycle.getPage(EditLine.PAGE);

        Object[] params = cycle.getServiceParameters();
        Integer lineId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        page.setLineId(lineId.intValue());

        cycle.activate(page);
    }
    
    public void editSettings(IRequestCycle cycle) {
        LineSettings page = (LineSettings) cycle.getPage(LineSettings.PAGE);

        Object[] params = cycle.getServiceParameters();
        Integer lineId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        page.setLineId(lineId.intValue());

        String section = (String) TapestryUtils.assertParameter(String.class, params, 1);        
        page.setParentSettingName(section);
        
        cycle.activate(page);        
    }
    
    /**
     * Used for contructing parameters for EditSettings DirectLink
     */
    public Object[] getEditSettingListenerParameters() {
        return new Object[] { 
            new Integer(getLine().getId()),
            getCurrentSetting().getName() 
        };
    }
    
    public void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        
        PhoneContext context = getPhoneContext();
        Phone phone = context.getPhone(getLine().getEndpoint());
        setSettings(getLine().getSettings(phone));
    }        
}
