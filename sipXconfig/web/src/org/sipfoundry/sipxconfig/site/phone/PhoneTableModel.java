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
package org.sipfoundry.sipxconfig.site.phone;

import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.IComponent;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.contrib.table.model.ITableModel;
import org.sipfoundry.sipxconfig.components.AbstractTableModel;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

/**
 * Geared towards paging-based data models for tapestry TableViews that only
 * load one page at a time and do server-side sorting.  
 */
public class PhoneTableModel extends AbstractTableModel {    
    private static final String SERIAL_NUM_PROPERTY = "serialNumber";
    private static final String DESCRIPTION_PROPERTY = "description";
    private static final String MODEL_PROPERTY = "model.label";
    
    private PhoneContext m_phoneContext;
    private Integer m_groupId;

    public void setGroupId(Integer groupId) {
        m_groupId = groupId;
    }

    public void setPhoneContext(PhoneContext context) {
        m_phoneContext = context;
    }

    public ITableModel createTableModel(IComponent component) {
        OrderByTableColumn[] columns = new OrderByTableColumn[] {
            new OrderByTableColumn("Phone", SERIAL_NUM_PROPERTY, SERIAL_NUM_PROPERTY),
            new OrderByTableColumn("Lines", "lines", "beanId"),
            new OrderByTableColumn("Model", MODEL_PROPERTY, "modelId"),
            new OrderByTableColumn("Description", DESCRIPTION_PROPERTY, DESCRIPTION_PROPERTY)
        };

        return createTableModel(component, columns);
    }

    public int getRowCount() {
        return m_phoneContext.getPhoneCount();
    }

    public Iterator getCurrentPageRows(int firstRow, int pageSize, ITableColumn objSortColumn, boolean orderAscending) {
        OrderByTableColumn userCol = (OrderByTableColumn) objSortColumn;
        String orderBy = userCol != null ? userCol.getOrderBy() : SERIAL_NUM_PROPERTY;
        List page = m_phoneContext.loadPhonesByPage(m_groupId, firstRow, pageSize, orderBy, orderAscending);
        return page.iterator();
    }

}
