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
package org.sipfoundry.sipxconfig.site;

import java.util.Arrays;
import java.util.List;

import org.apache.tapestry.contrib.table.model.ITableColumnModel;
import org.apache.tapestry.contrib.table.model.ITableModel;
import org.apache.tapestry.contrib.table.model.ITableSortingState;
import org.apache.tapestry.contrib.table.model.simple.SimpleTableColumnModel;
import org.apache.tapestry.contrib.table.model.simple.SimpleTableModel;
import org.sipfoundry.sipxconfig.phone.PhoneContext;


/**
 * List all the phones/endpoints for management and details drill-down
 */
public class ListPhonesPage extends AbstractPhonePage {

    private ITableModel m_phones;
    
    public void setPhoneContext(PhoneContext context) {
        super.setPhoneContext(context);

        List columnLabels = Arrays.asList(new String[] { "a", "b", "c" });        
        ITableColumnModel columns = new SimpleTableColumnModel(columnLabels);
        Object[] data = new Object[] {
                "apple", "orange", "banana"
        };        
        m_phones = new SimpleTableModel(data, columns);
        m_phones.getPagingState().setPageSize(15);
        m_phones.getPagingState().setCurrentPage(0);
        m_phones.getSortingState().setSortColumn("a", ITableSortingState.SORT_ASCENDING);        
    }
    
    public ITableModel getPhones() {
        return m_phones;
    }    
}
