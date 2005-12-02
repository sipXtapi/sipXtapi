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
package org.sipfoundry.sipxconfig.site.setting;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.site.phone.PhoneModels;

public abstract class GroupLabel extends BaseComponent {
    
    public abstract Group getGroup();
    
    public void viewGroup(IRequestCycle cycle) {
        Integer groupId = (Integer) TapestryUtils.assertParameter(Integer.class, cycle.getServiceParameters(), 0);
        String resourceId = (String) TapestryUtils.assertParameter(String.class, cycle.getServiceParameters(), 1);
        // HACK: Enumerate all possible group types, cannot think a better way w/o
        // introducing interfaces
        if (resourceId.equals("phone")) {
            PhoneModels page = (PhoneModels) cycle.getPage(PhoneModels.PAGE);
            page.setGroupId(groupId);
            cycle.activate(page);
        }
    }
}
