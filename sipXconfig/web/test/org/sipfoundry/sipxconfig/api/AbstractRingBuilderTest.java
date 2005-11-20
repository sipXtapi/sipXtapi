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

public class AbstractRingBuilderTest extends TestCase {
    private AbstractRingBuilder m_builder;
    private org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing m_myAbstractRing;
    private AbstractRing m_apiAbstractRing;
    
    protected void setUp() {
        m_builder = new AbstractRingBuilder();
        m_myAbstractRing = new org.sipfoundry.sipxconfig.admin.callgroup.UserRing();
        m_apiAbstractRing = new AbstractRing();
    }

    public void testFromApi() {
        m_apiAbstractRing.setType(AbstractRingBuilder.TYPE_IMMEDIATE);
        ApiBeanUtil.toMyObject(m_builder, m_myAbstractRing, m_apiAbstractRing);
        assertEquals(org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing.Type.IMMEDIATE,
                m_myAbstractRing.getType());
    }

    public void testToApi() {
        m_myAbstractRing.setType(org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing.Type.IMMEDIATE);
        ApiBeanUtil.toApiObject(m_builder, m_apiAbstractRing, m_myAbstractRing);
        assertEquals(AbstractRingBuilder.TYPE_IMMEDIATE, m_apiAbstractRing.getType());
    }
}
