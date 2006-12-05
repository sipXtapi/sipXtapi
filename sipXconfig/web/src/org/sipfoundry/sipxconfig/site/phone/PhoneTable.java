/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.site.line.EditLine;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class PhoneTable extends BaseComponent {
    @Parameter(required = true)
    public abstract SelectMap getSelections();

    @Parameter(required = true)
    public abstract Object getSource();

    @InjectPage(value = EditPhone.PAGE)
    public abstract EditPhone getEditPhonePage();

    @InjectPage(value = EditLine.PAGE)
    public abstract EditLine getEditLinePage();

    public abstract Phone getPhone();

    public abstract Line getLine();

    /**
     * Called, when user clicks on link to edit a phone.
     */
    public IPage editPhone(Integer phoneId) {
        EditPhone page = getEditPhonePage();
        page.setPhoneId(phoneId);
        page.setReturnPage(getPage());
        return page;
    }

    /**
     * Called, when user clicks on link to edit a line.
     */
    public IPage editLine(Integer lineId) {
        EditLine page = getEditLinePage();
        page.setLineId(lineId);
        return page;
    }

    public String getVersion() {
        DeviceVersion ver = getPhone().getDeviceVersion();
        if (ver == null) {
            return null;
        }

        return "v" + ver.getVersionId();
    }
}
