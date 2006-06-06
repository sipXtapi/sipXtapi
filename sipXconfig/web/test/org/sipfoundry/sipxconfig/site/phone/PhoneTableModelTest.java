/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import junit.framework.TestCase;

import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;

public class PhoneTableModelTest extends TestCase {

    public void testOrderByFromSortColumEmpty() {
        String[] orderBy = PhoneTableModel.orderByFromSortColum(null);
        assertEquals(0, orderBy.length);
    }

    public void testOrderByFromSortColum() {
        IMocksControl columnCtrl = EasyMock.createControl();
        ITableColumn column = columnCtrl.createMock(ITableColumn.class);
        column.getColumnName();
        columnCtrl.andReturn("bongo");
        column.getColumnName();
        columnCtrl.andReturn("modelId");
        columnCtrl.replay();

        String[] orderBy = PhoneTableModel.orderByFromSortColum(column);
        assertEquals(1, orderBy.length);
        assertEquals("bongo", orderBy[0]);

        orderBy = PhoneTableModel.orderByFromSortColum(column);
        assertEquals(2, orderBy.length);
        assertEquals("beanId", orderBy[0]);
        assertEquals("modelId", orderBy[1]);

        columnCtrl.verify();
    }
}
