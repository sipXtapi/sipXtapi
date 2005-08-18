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
package org.sipfoundry.sipxconfig.common;

import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.springframework.beans.factory.ListableBeanFactory;

public class DaoEventDispatcherTest extends TestCase {

    public void testOnSaveAndOnDelete() {
        Object entity = new Object();

        MockControl listenerControl = MockControl.createStrictControl(DaoEventListener.class);
        DaoEventListener listener = (DaoEventListener) listenerControl.getMock();
        listenerControl.expectAndReturn(listener.onSave(entity, null, null, null, null), false);
        listener.onDelete(entity, null, null, null, null);
        listenerControl.setDefaultVoidCallable();
        listenerControl.replay();
        
        MockControl bfControl = MockControl.createStrictControl(ListableBeanFactory.class);
        ListableBeanFactory bf = (ListableBeanFactory) bfControl.getMock();
        Map beanMap = new HashMap();
        beanMap.put(listener.getClass().getName(), listener);
        bfControl.expectAndReturn(bf.getBeansOfType(DaoEventListener.class, true, true), beanMap, 2);
        bfControl.replay();
        
        DaoEventDispatcher dispatcher = new DaoEventDispatcher();
        dispatcher.setBeanFactory(bf);
        dispatcher.onSave(entity, null, null, null, null);
        dispatcher.onDelete(entity, null, null, null, null);
        
        listenerControl.verify();
        bfControl.verify();
    }
}


