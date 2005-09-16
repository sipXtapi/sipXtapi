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

import org.apache.tapestry.IComponent;
import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.contrib.table.model.ITableColumnModel;
import org.apache.tapestry.contrib.table.model.ITableModel;
import org.apache.tapestry.contrib.table.model.common.BasicTableModelWrap;
import org.apache.tapestry.contrib.table.model.ognl.ExpressionTableColumn;
import org.apache.tapestry.contrib.table.model.simple.SimpleTableColumnModel;
import org.sipfoundry.sipxconfig.common.CoreContext;

public class UserTableModel implements IBasicTableModel {
    
    static final String USER_NAME_PROPERTY = "userName"; 
    static final String FIRST_NAME_PROPERTY = "firstName"; 
    static final String LAST_NAME_PROPERTY = "lastName";
    static final int DEFAULT_PAGE_SIZE = 10;
    
    private int m_pageSize = DEFAULT_PAGE_SIZE;
    
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
    
    /**
     * For some reason if table model is overridden, page size is ignored when
     * defined on TableFormView
     *  
     * @param pageSize default is 10
     */
    public void setPageSize(int pageSize) {
        m_pageSize = pageSize;
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
    
    public ITableModel createTableModel(IComponent component) {
        UserTableColumn[] columns = new UserTableColumn[] {
            new UserTableColumn(USER_NAME_PROPERTY, USER_NAME_PROPERTY, USER_NAME_PROPERTY),
            new UserTableColumn(FIRST_NAME_PROPERTY, FIRST_NAME_PROPERTY, FIRST_NAME_PROPERTY),
            new UserTableColumn(LAST_NAME_PROPERTY, LAST_NAME_PROPERTY, LAST_NAME_PROPERTY),
            new UserTableColumn("aliases", "aliasesString")
        };
        for (int i = 0; i < columns.length; i++) {
            columns[i].loadSettings(component);
        }

        ITableColumnModel colModel = new SimpleTableColumnModel(Arrays.asList(columns));
        BasicTableModelWrap tableModel = new BasicTableModelWrap(this, colModel);
        tableModel.getPagingState().setPageSize(m_pageSize);

        return tableModel;
    }
    
    static class UserTableColumn extends ExpressionTableColumn {
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

