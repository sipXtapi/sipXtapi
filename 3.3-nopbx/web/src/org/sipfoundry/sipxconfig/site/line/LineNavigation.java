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
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Setting;

public abstract class LineNavigation extends BaseComponent {

    /** REQUIRED PARAMETER */
    public abstract void setLine(Line line);
    
    public abstract Line getLine();
    
    public Setting getSettings() {
        return getLine().getSettings();
    }

    public abstract void setCurrentSetting(Setting setting);

    public abstract Setting getCurrentSetting();
    
    public abstract PhoneContext getPhoneContext();

    public void editLine(IRequestCycle cycle) {
        EditLine page = (EditLine) cycle.getPage(EditLine.PAGE);

        Object[] params = cycle.getListenerParameters();
        Integer lineId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        page.setLineId(lineId);

        cycle.activate(page);
    }
    
    public void editSettings(IRequestCycle cycle) {
        LineSettings page = (LineSettings) cycle.getPage(LineSettings.PAGE);

        Object[] params = cycle.getListenerParameters();
        Integer lineId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        page.setLineId(lineId);

        String section = (String) TapestryUtils.assertParameter(String.class, params, 1);        
        page.setParentSettingName(section);
        
        cycle.activate(page);        
    }
    
    /**
     * Used for contructing parameters for EditSettings DirectLink
     */
    public Object[] getEditSettingListenerParameters() {
        return new Object[] { 
            getLine().getId(),
            getCurrentSetting().getName() 
        };
    }
}
