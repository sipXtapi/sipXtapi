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

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.site.phone.ManagePhones;
import org.sipfoundry.sipxconfig.site.phone.PhonePageUtils;

/**
 * Create a new line
 */
public abstract class NewLine extends BasePage {
    
    public static final String PAGE = "NewLine";
    
    public abstract void setEndpoint(Endpoint endpoint);

    public abstract Endpoint getEndpoint();
    
    public abstract void setLine(Line line);
    
    public abstract Line getLine();
    
    public abstract int getEndpointId();
    
    public abstract void setEndpointId(int id);
    
    public abstract void setReturnPage(String returnPage);
    
    public abstract String getReturnPage();
    
    public void finish(IRequestCycle cycle) {
        PhoneContext phoneContext = PhonePageUtils.getPhoneContext(cycle);
        saveLine(phoneContext);
        if (isEndpointWizard()) {
            saveEndpoint(phoneContext);
        }
        cycle.activate(getReturnPage());
    }
    
    public boolean isEndpointWizard() {
        return getEndpoint() != null;
    }
    
    void saveLine(PhoneContext phoneContext) {
        phoneContext.storeLine(getLine());
    }
    
    void saveEndpoint(PhoneContext phoneContext_) {
        // TODO: may not have line interface in 2.8
        throw new RuntimeException("not implemented");
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }
}
