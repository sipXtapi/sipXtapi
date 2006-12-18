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

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.common.NavigationWithSettings;

public abstract class LineNavigation extends NavigationWithSettings {

    /** REQUIRED PARAMETER */
    public abstract void setLine(Line line);

    public abstract Line getLine();

    public abstract PhoneContext getPhoneContext();

    public void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        Setting settings = getLine().getSettings();
        setSettings(settings);
        setMessageSource(settings.getMessageSource());
    }

    public IPage editLine(IRequestCycle cycle, Integer lineId) {
        EditLine page = (EditLine) cycle.getPage(EditLine.PAGE);
        page.setLineId(lineId);
        return page;
    }

    public IPage editSettings(IRequestCycle cycle, Integer lineId, String section) {
        LineSettings page = (LineSettings) cycle.getPage(LineSettings.PAGE);
        page.setLineId(lineId);
        page.setParentSettingName(section);
        return page;
    }

    /**
     * Used for contructing parameters for EditSettings DirectLink
     */
    public Object[] getEditSettingListenerParameters() {
        return new Object[] {
            getLine().getId(), getCurrentSetting().getName()
        };
    }
}
