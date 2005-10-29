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
import org.sipfoundry.sipxconfig.common.CoreManager;

public class UserTableModel implements IBasicTableModel {
    private CoreManager m_coreContext;
    private Integer m_groupId;
    private String m_searchString;

    public UserTableModel(CoreManager coreContext, Integer groupId, String searchString) {
        setCoreContext(coreContext);
        setGroupId(groupId);
        setSearchString(searchString);
    }

    public UserTableModel() {
        // intentionally empty
    }

    public void setCoreContext(CoreManager coreContext) {
        m_coreContext = coreContext;
    }

    public int getRowCount() {
        int count = m_coreContext.getUsersInGroupWithSearchCount(m_groupId, m_searchString);
        return count;
    }

    public Integer getGroupId() {
        return m_groupId;
    }

    public void setGroupId(Integer groupId) {
        m_groupId = groupId;
    }

    public String getSearchString() {
        return m_searchString;
    }

    public void setSearchString(String searchString) {
        m_searchString = searchString;
    }

    public Iterator getCurrentPageRows(int firstRow, int pageSize, ITableColumn objSortColumn,
            boolean orderAscending) {
        String orderBy = objSortColumn != null ? objSortColumn.getColumnName() : null;
        List page = m_coreContext.loadUsersByPage(m_searchString, m_groupId, firstRow, pageSize, orderBy,
                orderAscending);
        return page.iterator();
    }
}
