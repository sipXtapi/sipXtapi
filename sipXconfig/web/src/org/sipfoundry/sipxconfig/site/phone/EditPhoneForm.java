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

import java.util.Collection;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class EditPhoneForm extends BaseComponent {
    public abstract Phone getPhone();

    public abstract String getGroupsString();

    public abstract void setGroupsString(String groupsString);

    public abstract PhoneContext getPhoneContext();

    public abstract SettingDao getSettingDao();

    public abstract Collection getGroupsCandidates();

    public abstract void setGroupCandidates(Collection groupsList);

    public void buildGroupCandidates(String groupsString) {
        List allGroups = getPhoneContext().getGroups();
        Collection candidates = TapestryUtils.getAutoCompleteCandidates(allGroups, groupsString);
        setGroupCandidates(candidates);
    }

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {

        if (!TapestryUtils.isRewinding(cycle, this)) {
            if (getGroupsString() == null) {
                Phone phone = getPhone();
                setGroupsString(phone.getGroupsNames());
            }
        }

        super.renderComponent(writer, cycle);

        if (TapestryUtils.isRewinding(cycle, this) && TapestryUtils.isValid(this)) {
            String groupsString = getGroupsString();
            if (groupsString != null) {
                List groups = getSettingDao().getGroupsByString(Phone.GROUP_RESOURCE_ID,
                        groupsString, false);
                Phone phone = getPhone();
                phone.setGroupsAsList(groups);
            }
        }
    }
}
