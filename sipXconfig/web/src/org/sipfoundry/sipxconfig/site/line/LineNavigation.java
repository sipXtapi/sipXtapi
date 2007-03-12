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
import org.apache.tapestry.annotations.InjectPage;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.site.common.BeanNavigation;

public abstract class LineNavigation extends BeanNavigation<Line> {

    @InjectPage(value = LineSettings.PAGE)
    public abstract LineSettings getLineSettingsPage();

    @InjectPage(value = EditLine.PAGE)
    public abstract EditLine getEditLinePage();

    public boolean isIdentificationTabActive() {
        return EditLine.PAGE.equals(getPage().getPageName());
    }

    public IPage editLine(Integer lineId) {
        EditLine page = getEditLinePage();
        page.setLineId(lineId);
        return page;
    }

    public IPage editSettings(Integer lineId, String section) {
        LineSettings page = getLineSettingsPage();
        page.setLineId(lineId);
        page.setParentSettingName(section);
        return page;
    }
}
