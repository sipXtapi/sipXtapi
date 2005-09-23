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
package org.sipfoundry.sipxconfig.components;

import java.util.Arrays;

import org.apache.tapestry.IComponent;
import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.ITableColumnModel;
import org.apache.tapestry.contrib.table.model.ITableModel;
import org.apache.tapestry.contrib.table.model.common.BasicTableModelWrap;
import org.apache.tapestry.contrib.table.model.ognl.ExpressionTableColumn;
import org.apache.tapestry.contrib.table.model.simple.SimpleTableColumn;
import org.apache.tapestry.contrib.table.model.simple.SimpleTableColumnModel;

public abstract class AbstractTableModel implements IBasicTableModel {

    static final int DEFAULT_PAGE_SIZE = 10;
    private int m_pageSize = DEFAULT_PAGE_SIZE;
       
    /**
     * For some reason if table model is overridden, page size is ignored when
     * defined on TableFormView
     *  
     * @param pageSize default is 10
     */
    public void setPageSize(int pageSize) {
        m_pageSize = pageSize;
    }

    public ITableModel createTableModel(IComponent component, SimpleTableColumn[] columns) {
        for (int i = 0; i < columns.length; i++) {
            columns[i].loadSettings(component);
        }

        ITableColumnModel colModel = new SimpleTableColumnModel(Arrays.asList(columns));
        BasicTableModelWrap tableModel = new BasicTableModelWrap(this, colModel);
        tableModel.getPagingState().setPageSize(m_pageSize);

        return tableModel;        
    }
    
    public static class OrderByTableColumn extends ExpressionTableColumn {
        private String m_orderBy;
        
        public OrderByTableColumn(String name, String expression, String orderBy) {
            super(name, expression, true);
            m_orderBy = orderBy;            
        }
        
        public OrderByTableColumn(String name, String expression) {
            super(name, expression, false);
        }
        
        public String getOrderBy() {
            return m_orderBy;
        }
    }
}
