/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.apache.tapestry.html.BasePage;

/**
 * ActivateDialPlan
 */
public abstract class ActivateDialPlan extends BasePage {
    public static final String PAGE = "ActivateDialPlan";
    
    public static final IPropertySelectionModel XML_FILES_MODEL = 
        new StringPropertySelectionModel(new String[] {"mapping_rules.xml", "fallback_rules.xml", "authrules.xml"}); 

    public abstract String getSelected();
    
    public String getXml() {
        return "Display: " + getSelected();     
    }

    public void setXml(String xml_) {
        // ignore xml - read only field
    }
    
    public void activate(IRequestCycle cycle_) {
        // activate dial plan
    }
}
