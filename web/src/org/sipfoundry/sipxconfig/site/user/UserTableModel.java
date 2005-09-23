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

import org.apache.tapestry.IComponent;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.contrib.table.model.ITableModel;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.AbstractTableModel;

public class UserTableModel extends AbstractTableModel {
    
    static final String USER_NAME_PROPERTY = "userName"; 
    static final String FIRST_NAME_PROPERTY = "firstName"; 
    static final String LAST_NAME_PROPERTY = "lastName";
    
    private CoreContext m_coreContext;
    private Integer m_groupId;

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
    
    public int getRowCount() {
        int count = m_coreContext.getUserCount();
        return count;
    }
    
    public void setGroupId(Integer groupId) {
        m_groupId = groupId;
    }

    public Iterator getCurrentPageRows(int firstRow, int pageSize, ITableColumn objSortColumn,
            boolean orderAscending) {
        
        OrderByTableColumn userCol = (OrderByTableColumn) objSortColumn;
        String orderBy = userCol != null ? userCol.getOrderBy() : USER_NAME_PROPERTY;
        List page = m_coreContext.loadUsersByPage(m_groupId, firstRow, pageSize, orderBy, orderAscending);
        return page.iterator();
    }
    
    public ITableModel createTableModel(IComponent component, Integer groupId) {
        setGroupId(groupId);
        OrderByTableColumn[] columns = new OrderByTableColumn[] {
            new OrderByTableColumn(USER_NAME_PROPERTY, USER_NAME_PROPERTY, USER_NAME_PROPERTY),
            new OrderByTableColumn(FIRST_NAME_PROPERTY, FIRST_NAME_PROPERTY, FIRST_NAME_PROPERTY),
            new OrderByTableColumn(LAST_NAME_PROPERTY, LAST_NAME_PROPERTY, LAST_NAME_PROPERTY),
            new OrderByTableColumn("aliases", "aliasesString")
        };

        return createTableModel(component, columns);
    }
}

