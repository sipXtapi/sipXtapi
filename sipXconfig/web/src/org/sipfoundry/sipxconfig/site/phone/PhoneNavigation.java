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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Top portion of pages that show tabs, help box, intro text, etc
 */
public abstract class PhoneNavigation extends BaseComponent {

    /** REQUIRED PARAMETER */
    public abstract void setPhone(Phone phone);

    public abstract Phone getPhone();

    public abstract void setSettings(Setting settings);

    public abstract Setting getSettings();

    public abstract void setCurrentSetting(Setting setting);

    public abstract Setting getCurrentSetting();

    public abstract PhoneContext getPhoneContext();

    public IPage editPhone(IRequestCycle cycle, Integer phoneId) {
        EditPhone page = (EditPhone) cycle.getPage(EditPhone.PAGE);
        page.setPhoneId(phoneId);
        return page;
    }

    public IPage editLines(IRequestCycle cycle, Integer phoneId) {
        PhoneLines page = (PhoneLines) cycle.getPage(PhoneLines.PAGE);
        page.setPhoneId(phoneId);
        return page;
    }

    public IPage editSettings(IRequestCycle cycle, Integer phoneId, String section) {
        PhoneSettings page = (PhoneSettings) cycle.getPage(PhoneSettings.PAGE);
        page.setPhoneId(phoneId);
        page.setParentSettingName(section);
        return page;
    }

    /**
     * Used for contructing parameters for EditSettings DirectLink
     */
    public Object[] getEditSettingListenerParameters() {
        return new Object[] {
            getPhone().getId(), getCurrentSetting().getName()
        };
    }

    public void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        setSettings(getPhone().getSettings());
    }
}
