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
package org.sipfoundry.sipxconfig.vendor;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.core.Phone;
import org.sipfoundry.sipxconfig.core.SipxConfig;
import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.beans.factory.access.SingletonBeanFactoryLocator;

/**
 * Comments
 */
public class SipxPhoneTest extends TestCase {
    
    private Phone m_softphone;

    /*
     * @see TestCase#setUp()
     */
    protected void setUp() throws Exception {
        super.setUp();

        BeanFactoryLocator bfl = SingletonBeanFactoryLocator.getInstance();
        BeanFactoryReference bf = bfl.useBeanFactory("unittest");
        // now use some bean from factory 
        SipxConfig sipx = (SipxConfig) bf.getFactory().getBean("sipxconfig");
        assertNotNull(sipx);

		m_softphone = (Phone)sipx.getPhoneFactory().getPhoneById(SipxPhone.SOFTPHONE);
		assertTrue(m_softphone != null);
    }

    /**
     * Doesn't really check values, just that they're not null. This 
     * may want to change
     */
    public void testGetters() {
        // TODO
    }
}
