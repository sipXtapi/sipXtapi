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
package org.sipfoundry.sipxconfig.site.user;

import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.sipfoundry.sipxconfig.common.CoreContext;

public class UserTableModel implements IBasicTableModel {
    private CoreContext m_coreContext;
    private Integer m_groupId;

    public UserTableModel(CoreContext coreContext, Integer groupId) {
        setCoreContext(coreContext);
        setGroupId(groupId);
    }

    public UserTableModel() {
        // intentionally empty
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public int getRowCount() {
        int count = m_coreContext.getUsersInGroupCount(m_groupId);
        return count;
    }

    public void setGroupId(Integer groupId) {
        m_groupId = groupId;
    }

    public Integer getGroupId() {
        return m_groupId;
    }

    public Iterator getCurrentPageRows(int firstRow, int pageSize, ITableColumn objSortColumn,
            boolean orderAscending) {
        String orderBy = objSortColumn != null ? objSortColumn.getColumnName() : null;
        List page = m_coreContext.loadUsersByPage(m_groupId, firstRow, pageSize, orderBy,
                orderAscending);
        return page.iterator();
    }
}
