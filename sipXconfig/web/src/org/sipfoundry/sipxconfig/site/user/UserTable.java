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
package org.sipfoundry.sipxconfig.site.user;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.IPrimaryKeyConvertor;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.common.CoreManager;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.ObjectSourceDataSqueezer;
import org.sipfoundry.sipxconfig.components.SelectMap;

public abstract class UserTable extends BaseComponent implements PageRenderListener {

    public static final String COMPONENT = "UserTable";

    /** REQUIRED PROPERTY */
    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selected);

    public abstract CoreManager getCoreContext();

    public abstract Integer getGroupId();
    
    public abstract String getSearchString();

    public IBasicTableModel getTableModel() {
        return new UserTableModel(getCoreContext(), getGroupId(), getSearchString());
    }

    public void pageBeginRender(PageEvent event_) {
        if (getSelections() == null) {
            setSelections(new SelectMap());
        }
    }

    public IPrimaryKeyConvertor getIdConverter() {
        CoreManager context = getCoreContext();
        return new ObjectSourceDataSqueezer(context, User.class);
    }
}
