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
package org.sipfoundry.sipxconfig.site.dialplan;

import java.util.Collection;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantInUseException;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.VxmlGenerator;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class ManageAttendants extends BasePage {
    
    public static final String PAGE = "ManageAttendants"; 
    
    public abstract DialPlanContext getDialPlanManager();
    
    public abstract VxmlGenerator getVxmlGenerator();

    public abstract AutoAttendant getCurrentRow();
    
    public abstract SelectMap getSelections();

    public void deleteSelected(IRequestCycle cycle_) {        
        Collection selectedRows = getSelections().getAllSelected();
        if (selectedRows != null) {
            DialPlanContext manager = getDialPlanManager();
            try {
                manager.deleteAutoAttendantsByIds(selectedRows, getVxmlGenerator().getScriptsDirectory());
            } catch (AttendantInUseException e) {
                IValidationDelegate validator = TapestryUtils.getValidator(this);
                validator.record(e.getMessage(), ValidationConstraint.CONSISTENCY);
            }
        }
    }
    
    public void edit(IRequestCycle cycle) {
        EditAutoAttendant page = (EditAutoAttendant) cycle.getPage(EditAutoAttendant.PAGE);
        
        Integer id = (Integer) TapestryUtils.assertParameter(Integer.class, cycle.getServiceParameters(), 0);
        AutoAttendant attendant = getDialPlanManager().getAutoAttendant(id);
        page.setAttendant(attendant);
        cycle.activate(page);
    }
    
    public void add(IRequestCycle cycle) {
        EditAutoAttendant page = (EditAutoAttendant) cycle.getPage(EditAutoAttendant.PAGE);
        page.setAttendant(null);
        cycle.activate(page);
    }
}
