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
package org.sipfoundry.sipxconfig.api;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.common.CoreContext;

public class UserBuilderTest extends TestCase {    
    private UserBuilder m_builder;
    private org.sipfoundry.sipxconfig.common.User m_other;
    private User m_api;
    
    protected void setUp() {
        m_builder = new UserBuilder();
        m_other = new org.sipfoundry.sipxconfig.common.User();
        m_api = new User();
        MockControl coreContextCtrl = MockControl.createNiceControl(CoreContext.class);
        CoreContext coreContext = (CoreContext) coreContextCtrl.getMock();
        coreContextCtrl.expectAndReturn(coreContext.getAuthorizationRealm(), "lampshade");
        coreContextCtrl.replay();
        m_builder.setCoreContext(coreContext);
    }

    public void testFromApi() {        
        m_api.setAliases("one, two");
        m_builder.fromApi(m_api, m_other);
        assertEquals("one, two", m_other.getAliasesString());
    }

    public void testToApi() {
        m_other.setAliasesString("one, two");
        m_builder.toApi(m_api, m_other);
        assertEquals("one, two", m_api.getAliases());
    }

    public void testGetGroupsByString() {
    }
}
