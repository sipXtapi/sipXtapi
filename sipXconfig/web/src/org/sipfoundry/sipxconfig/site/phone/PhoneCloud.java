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

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class PhoneCloud extends BaseComponent {
    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    @InjectPage(value = EditPhone.PAGE)
    public abstract EditPhone getEditPhonePage();

    public abstract List getPhones();

    public abstract void setPhones(List phones);

    public abstract int getCount();

    public abstract void setCount(int count);

    public abstract Phone getPhone();

    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        if (getPhones() == null) {
            PhoneContext phoneContext = getPhoneContext();
            // TODO: replace by loading lightweight user substitute
            List<Phone> phones = phoneContext.loadPhonesByPage(null, 0, 25, new String[] {
                "serialNumber"
            }, true);
            setPhones(phones);
            setCount(phoneContext.getPhonesCount());
        }
    }

    public IPage edit(Integer phoneId) {
        EditPhone page = getEditPhonePage();
        page.setPhoneId(phoneId);
        page.setReturnPage(getPage());
        return page;
    }

    public String getTitle() {
        return getMessages().format("title", getCount());
    }
}
