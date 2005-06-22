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
import org.apache.tapestry.contrib.table.model.IPrimaryKeyConvertor;
import org.sipfoundry.sipxconfig.components.HibernateDaoDataSqueezer;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class GroupTable extends BaseComponent {
    
    public abstract SettingDao getSettingContext();

    public IPrimaryKeyConvertor getIdConverter() {
        return new HibernateDaoDataSqueezer(getSettingContext(), Group.class);
    }

    public void deleteGroup(IRequestCycle cycle_) {        
    }
}
