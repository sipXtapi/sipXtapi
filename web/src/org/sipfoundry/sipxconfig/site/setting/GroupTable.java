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

import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.contrib.table.model.IPrimaryKeyConvertor;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.components.ObjectSourceDataSqueezer;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class GroupTable extends BaseComponent {
    
    public abstract SettingDao getSettingContext();
    
    public abstract List getGroups();
    
    public abstract SelectMap getSelections();
    
    public IPrimaryKeyConvertor getIdConverter() {
        return new ObjectSourceDataSqueezer(getSettingContext(), Group.class);
    }

    public void deleteGroup(IRequestCycle cycle_) {
        SettingDao context = getSettingContext();
        Object[] groupIds = getSelections().getAllSelected().toArray();
        List groups = getGroups();
        Collection deletedGroups = DataCollectionUtil.findByPrimaryKey(groups, groupIds);
        Iterator i = deletedGroups.iterator();
        while (i.hasNext()) {
            Group group = (Group) i.next();
            context.deleteGroup(group);
        }
    }
    
    public void moveUp(IRequestCycle cycle_) {
        moveGroups(-1);
    }

    public void moveDown(IRequestCycle cycle_) {
        moveGroups(1);
    }
    
    void moveGroups(int step) {
        SettingDao context = getSettingContext();
        Object[] groupIds = getSelections().getAllSelected().toArray();
        List groups = getGroups();
        DataCollectionUtil.moveByPrimaryKey(groups, groupIds, step);
        Iterator i = groups.iterator();
        while (i.hasNext()) {
            Group group = (Group) i.next();
            context.storeGroup(group);
        }
    }
}
