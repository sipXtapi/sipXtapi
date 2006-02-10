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

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.site.user.UserTable;

public abstract class AddPhoneUser extends BasePage implements PageBeginRenderListener {

    public static final String PAGE = "AddPhoneUser";

    public abstract Phone getPhone();

    public abstract void setPhone(Phone phone);

    /** REQUIRED PROPERTY */
    public abstract Integer getPhoneId();

    public abstract void setPhoneId(Integer id);

    public abstract PhoneContext getPhoneContext();

    public abstract CoreContext getCoreContext();

    /**
     * made no attempts to abstract this to return to generic page because return page needs to
     * get state (phone id) returned This should be replaced with generic BreadCrumbs or
     * PageFlowGraph utility class when it's invented.
     */
    public abstract void setReturnToEditPhone(boolean rtn);

    public abstract boolean getReturnToEditPhone();

    public void select(IRequestCycle cycle) {
        PhoneContext context = getPhoneContext();

        UserTable table = (UserTable) getComponent("searchResults");
        SelectMap selections = table.getSelections();
        context.addUsersToPhone(getPhoneId(), selections.getAllSelected());

        PhoneLines page = (PhoneLines) cycle.getPage(PhoneLines.PAGE);
        page.setPhoneId(getPhoneId());
        cycle.activate(page);
    }

    public void cancel(IRequestCycle cycle) {
        if (getReturnToEditPhone()) {
            EditPhone page = (EditPhone) cycle.getPage(EditPhone.PAGE);
            page.setPhoneId(getPhoneId());
            cycle.activate(page);
        } else {
            PhoneLines page = (PhoneLines) cycle.getPage(PhoneLines.PAGE);
            page.setPhoneId(getPhoneId());
            cycle.activate(page);
        }
    }

    public void pageBeginRender(PageEvent event_) {
        Phone phone = getPhone();
        if (phone == null) {
            phone = getPhoneContext().loadPhone(getPhoneId());
            setPhone(phone);
        }
    }
}
