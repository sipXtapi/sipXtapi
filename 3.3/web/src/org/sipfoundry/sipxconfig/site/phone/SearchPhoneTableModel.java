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

import org.apache.commons.collections.Transformer;
import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.sipfoundry.sipxconfig.components.ObjectSourceDataSqueezer;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.search.BeanAdaptor;
import org.sipfoundry.sipxconfig.search.SearchManager;

/**
 * Geared towards paging-based data models for tapestry TableViews that only load one page at a
 * time and do server-side sorting.
 */
public class SearchPhoneTableModel implements IBasicTableModel {
    private String m_queryText;
    private SearchManager m_searchManager;
    private ObjectSourceDataSqueezer m_squeezer;

    public SearchPhoneTableModel(SearchManager searchManager, String queryText,
            ObjectSourceDataSqueezer squeezer) {
        m_queryText = queryText;
        m_searchManager = searchManager;
        m_squeezer = squeezer;
    }

    public SearchPhoneTableModel() {
        // intentionally empty
    }

    public void setQueryText(String queryText) {
        m_queryText = queryText;
    }

    public void setSearchManager(SearchManager searchManager) {
        m_searchManager = searchManager;
    }

    public int getRowCount() {
        List phones = m_searchManager.search(Phone.class, m_queryText, null);
        return phones.size();
    }

    public Iterator getCurrentPageRows(int firstRow, int pageSize, ITableColumn objSortColumn,
            boolean orderAscending) {
        String[] orderBy = PhoneTableModel.orderByFromSortColum(objSortColumn);        
        List page = m_searchManager.search(Phone.class, m_queryText, firstRow, pageSize, orderBy,
                orderAscending, new IdToPhone());
        return page.iterator();
    }

    private class IdToPhone implements Transformer {
        public Object transform(Object input) {
            BeanAdaptor.Identity identity = (BeanAdaptor.Identity) input;
            return m_squeezer.getValue(identity.getBeanId());
        }
    }
}
