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
import org.easymock.MockControl;

public class PhoneTableModelTest extends TestCase {

    public void testOrderByFromSortColumEmpty() {
        String[] orderBy = PhoneTableModel.orderByFromSortColum(null);
        assertEquals(0, orderBy.length);
    }

    public void testOrderByFromSortColum() {
        MockControl columnCtrl = MockControl.createControl(ITableColumn.class);
        ITableColumn column = (ITableColumn) columnCtrl.getMock();
        columnCtrl.expectAndReturn(column.getColumnName(), "bongo");
        columnCtrl.expectAndReturn(column.getColumnName(), "modelId");
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
