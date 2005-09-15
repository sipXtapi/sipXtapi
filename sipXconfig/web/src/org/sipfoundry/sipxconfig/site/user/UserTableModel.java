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

import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.contrib.table.model.ITableModel;
import org.apache.tapestry.contrib.table.model.common.BasicTableModelWrap;
import org.apache.tapestry.contrib.table.model.ognl.ExpressionTableColumn;
import org.apache.tapestry.contrib.table.model.simple.SimpleTableColumnModel;
import org.sipfoundry.sipxconfig.common.CoreContext;

public class UserTableModel implements IBasicTableModel {
    
    static final String USER_NAME_PROPERTY = "userName"; 
    static final String FIRST_NAME_PROPERTY = "firstName"; 
    static final String LAST_NAME_PROPERTY = "lastName"; 

    static final ITableColumn[] COLUMNS = new ITableColumn[] {
        new UserTableColumn("User Name", USER_NAME_PROPERTY, USER_NAME_PROPERTY),
        new UserTableColumn("First Name", FIRST_NAME_PROPERTY, FIRST_NAME_PROPERTY),
        new UserTableColumn("Last Name", LAST_NAME_PROPERTY, LAST_NAME_PROPERTY),
        new UserTableColumn("Aliases", "aliasesString")
    };

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
        
        UserTableColumn userCol = (UserTableColumn) objSortColumn;
        String orderBy = userCol != null ? userCol.getOrderBy() : USER_NAME_PROPERTY;
        List page = m_coreContext.loadUsersByPage(firstRow, pageSize, orderBy, orderAscending);
        return page.iterator();
    }
        
    public ITableModel getTableModel() {
        return new BasicTableModelWrap(this, new SimpleTableColumnModel(Arrays.asList(COLUMNS)));
    }
    
    static  class UserTableColumn extends ExpressionTableColumn {
        private String m_orderBy;
        
        UserTableColumn(String name, String expression, String orderBy) {
            super(name, expression, true);
            m_orderBy = orderBy;
        }
        
        UserTableColumn(String name, String expression) {
            super(name, expression, false);
        }
        
        public String getOrderBy() {
            return m_orderBy;
        }
    }
}

