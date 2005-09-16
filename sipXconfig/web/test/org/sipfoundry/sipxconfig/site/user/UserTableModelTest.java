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
import java.util.Iterator;
import java.util.List;

import junit.framework.TestCase;

import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

public class UserTableModelTest extends TestCase {
    
    public void testGetCurrentPageRows() {
        User[] page1Array = new User[] { new User(), new User() };
        User[] page2Array = new User[] { new User(), new User() };
        
        List page1 = Arrays.asList(page1Array);
        List page2 = Arrays.asList(page2Array);
        
        MockControl coreContextCtrl = MockControl.createControl(CoreContext.class);
        CoreContext coreContext = (CoreContext) coreContextCtrl.getMock();
        coreContextCtrl.expectAndReturn(coreContext.loadUsersByPage(0, 1, "userName", true), page1); 
        coreContextCtrl.expectAndReturn(coreContext.loadUsersByPage(1, 1, "userName", true), page2); 
        coreContextCtrl.replay();

        UserTableModel model = new UserTableModel();
        model.setCoreContext(coreContext);
        ITableColumn col = new UserTableModel.UserTableColumn("User Name", "userName", "userName");
        Iterator i1 = model.getCurrentPageRows(0, 1, col, true);
        assertEquals(page1Array[0], i1.next());
        assertEquals(page1Array[1], i1.next());
        assertFalse(i1.hasNext());
        
        Iterator i2 = model.getCurrentPageRows(1, 1, col, true);
        assertEquals(page2Array[0], i2.next());
        assertEquals(page2Array[1], i2.next());
        assertFalse(i2.hasNext());

        coreContextCtrl.verify();
    }

}
