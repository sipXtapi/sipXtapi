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
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.PhonePageUtils;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneDao;

/**
 * Create a new line
 */
public abstract class NewLine extends BasePage {
    
    public static final String PAGE = "NewLine";
    
    public abstract void setPhone(Phone phone);

    public abstract Phone getPhone();
    
    public abstract void setLine(Line line);
    
    public abstract Line getLine();
    
    public abstract int getPhoneId();
    
    public abstract void setPhoneId(int id);
    
    public abstract void setReturnPage(String returnPage);
    
    public abstract String getReturnPage();
    
    public void finish(IRequestCycle cycle) {
        PhoneContext phoneContext = PhonePageUtils.getPhoneContext(cycle);
        saveLine(phoneContext);
        if (isEndpointLineWizard()) {
            saveEndpointLine(phoneContext);
        }
        cycle.activate(getReturnPage());
    }
    
    public boolean isEndpointLineWizard() {
        return getPhone() != null;
    }
    
    void saveLine(PhoneContext phoneContext) {
        PhoneDao dao = phoneContext.getPhoneDao();
        dao.storeLine(getLine());
    }
    
    void saveEndpointLine(PhoneContext phoneContext) {
        PhoneDao dao = phoneContext.getPhoneDao();
        dao.storeLine(getLine());
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ListPhones.PAGE);
    }
}
