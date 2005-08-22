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

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.context.ApplicationContext;

public class DaoEventDispatcherTest extends TestCase {
	
	public void testOnDelete() {
    	User u = new User();
    	u.setUserName("testme");

    	MockControl listenerCtrl = MockControl.createControl(DaoEventListener.class);
		DaoEventListener listener = (DaoEventListener) listenerCtrl.getMock();
		listener.onDelete(u);
		listenerCtrl.replay();
		
		DaoEventDispatcher dispatcher = DaoEventDispatcher.createDeleteDispatcher();
		dispatcher.onEvent(new Object[] {u}, Collections.singleton(listener));
		
		listenerCtrl.verify();		
	}

    public void _testOnSaveAspect() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
    	User u = new User();
    	u.setUserName("testme");
    	
    	MockControl listenerCtrl = MockControl.createControl(DaoEventListener.class);
		DaoEventListener listener = (DaoEventListener) listenerCtrl.getMock();
		listener.onSave(u);
		listenerCtrl.replay();
    	
    	MockControl control = MockControl.createStrictControl(ListableBeanFactory.class);
    	ListableBeanFactory beanFactory = (ListableBeanFactory) control.getMock();
    	Map beanMap = new HashMap();
    	beanMap.put("mockEventListener", listener);
    	control.expectAndReturn(beanFactory.getBeansOfType(DaoEventListener.class, true, true), beanMap);
    	control.replay();
    	
    	ApplicationContext app = TestHelper.getApplicationContext();
    	CoreContext core = (CoreContext) app.getBean(CoreContext.CONTEXT_BEAN_NAME);
    	DaoEventDispatcher dispatcher = (DaoEventDispatcher)app.getBean("onSaveEventDispatcher");
    	BeanFactory oldFactory = dispatcher.getBeanFactory();
    	try {
    		dispatcher.setBeanFactory(beanFactory);
    		// event
    		core.saveUser(u);
    		
    		// no event
    		core.clear();
    	
    		listenerCtrl.verify();
    		control.verify();
    	} finally {
    		dispatcher.setBeanFactory(oldFactory);
    	}
    }
}


