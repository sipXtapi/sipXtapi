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
package org.sipfoundry.sipxconfig.common.event;

import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.springframework.beans.BeansException;
import org.springframework.context.ApplicationContext;
import org.springframework.context.event.ContextRefreshedEvent;

public class DaoEventPublisherImplTest extends TestCase {

    public void testPublishDelete() {
        Object entity = new Object();

        MockControl listenerCtrl = MockControl.createControl(DaoEventListener.class);
        DaoEventListener listener = (DaoEventListener) listenerCtrl.getMock();
        listener.onDelete(entity);
        listener.onDelete(entity);
        listenerCtrl.replay();

        Map beans = new HashMap();
        beans.put("1", listener);
        beans.put("2", listener);

        MockControl appContextCtrl = MockControl.createControl(ApplicationContext.class);
        ApplicationContext appContext = (ApplicationContext) appContextCtrl.getMock();
        appContext.getBeansOfType(DaoEventListener.class, true, true);
        appContextCtrl.setReturnValue(beans);
        appContextCtrl.replay();

        ContextRefreshedEvent event = new ContextRefreshedEvent(appContext);

        DaoEventPublisherImpl impl = new DaoEventPublisherImpl();
        impl.onApplicationEvent(event);
        impl.publishDelete(entity);

        appContextCtrl.verify();
        listenerCtrl.verify();
    }

    public void testPublishSave() {
        Object entity = new Object();

        MockControl listenerCtrl = MockControl.createControl(DaoEventListener.class);
        DaoEventListener listener = (DaoEventListener) listenerCtrl.getMock();
        listener.onSave(entity);
        listener.onSave(entity);
        listenerCtrl.replay();

        Map beans = new HashMap();
        beans.put("1", listener);
        beans.put("2", listener);

        MockControl appContextCtrl = MockControl.createControl(ApplicationContext.class);
        ApplicationContext appContext = (ApplicationContext) appContextCtrl.getMock();
        appContext.getBeansOfType(DaoEventListener.class, true, true);
        appContextCtrl.setReturnValue(beans);
        appContextCtrl.replay();

        ContextRefreshedEvent event = new ContextRefreshedEvent(appContext);

        DaoEventPublisherImpl impl = new DaoEventPublisherImpl();
        impl.onApplicationEvent(event);
        impl.publishSave(entity);

        appContextCtrl.verify();
        listenerCtrl.verify();
    }

    public void testInitialization() {
        DaoEventPublisherImpl impl = new DaoEventPublisherImpl();
        impl.onApplicationEvent(null);

        try {
            impl.publishSave(new Object());
            fail("Should have raised BeansException");
        } catch (BeansException e) {
            // this is expected
        }
    }
}
