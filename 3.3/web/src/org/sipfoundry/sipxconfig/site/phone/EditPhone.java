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
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

/**
 * Tapestry Page support for editing and creating new phones
 */
public abstract class EditPhone extends BasePage implements PageBeginRenderListener {

    public static final String PAGE = "EditPhone";

    public abstract Phone getPhone();

    public abstract void setPhone(Phone phone);

    /** REQUIRED PROPERTY */
    public abstract Integer getPhoneId();

    public abstract void setPhoneId(Integer id);

    public abstract PhoneContext getPhoneContext();

    public abstract ICallback getCallback();

    public abstract void setCallback(ICallback callback);

    public void addLine(IRequestCycle cycle) {
        Object[] params = cycle.getListenerParameters();
        Integer phoneId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        AddPhoneUser page = (AddPhoneUser) cycle.getPage(AddPhoneUser.PAGE);
        page.setReturnToEditPhone(true);
        page.setPhoneId(phoneId);
        cycle.activate(page);
    }

    public void commit(IRequestCycle cycle_) {
        save();
    }

    private boolean save() {
        IValidationDelegate delegate = (IValidationDelegate) getBeans().getBean("validator");
        boolean save = !delegate.getHasErrors();
        if (save) {
            PhoneContext dao = getPhoneContext();
            dao.storePhone(getPhone());
        }

        return save;
    }

    public void pageBeginRender(PageEvent event_) {
        if (getPhone() != null) {
            return;
        }

        // Load the phone with the ID that was passed in
        PhoneContext context = getPhoneContext();
        setPhone(context.loadPhone(getPhoneId()));

        // If no callback has been given, then navigate back to Manage Phones on OK/Cancel
        if (getCallback() == null) {
            setCallback(new PageCallback(ManagePhones.PAGE));
        }
    }
}
