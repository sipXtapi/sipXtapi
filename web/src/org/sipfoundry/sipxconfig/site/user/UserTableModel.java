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

import java.util.Collection;
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

    // FIXME: will go away when m_user collection goes away
    private Collection m_users;

    // FIXME: will go away when m_user collection goes away
    public void setUsers(Collection users) {
        m_users = users;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
    
    public int getRowCount() {

        // FIXME: will go away when m_user collection goes away
        if (m_users != null) {
            return m_users.size();
        }

        int count = m_coreContext.getUserCount();
        return count;
    }

    public Iterator getCurrentPageRows(int firstRow, int pageSize, ITableColumn objSortColumn,
            boolean orderAscending) {
        
        // FIXME: will go away when m_user collection goes away
        if (m_users != null) {
            return m_users.iterator();
        }
        
        OrderByTableColumn userCol = (OrderByTableColumn) objSortColumn;
        String orderBy = userCol != null ? userCol.getOrderBy() : USER_NAME_PROPERTY;
        List page = m_coreContext.loadUsersByPage(firstRow, pageSize, orderBy, orderAscending);
        return page.iterator();
    }
    
    public ITableModel createTableModel(IComponent component) {
        OrderByTableColumn[] columns = new OrderByTableColumn[] {
            new OrderByTableColumn(USER_NAME_PROPERTY, USER_NAME_PROPERTY, USER_NAME_PROPERTY),
            new OrderByTableColumn(FIRST_NAME_PROPERTY, FIRST_NAME_PROPERTY, FIRST_NAME_PROPERTY),
            new OrderByTableColumn(LAST_NAME_PROPERTY, LAST_NAME_PROPERTY, LAST_NAME_PROPERTY),
            new OrderByTableColumn("aliases", "aliasesString")
        };

        return createTableModel(component, columns);
    }
}

