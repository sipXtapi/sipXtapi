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
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Collections;
import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Permission;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;

/**
 * LongDistanceRuleTest
 */
public class LongDistanceRuleTest extends TestCase {

    private LongDistanceRule m_rule;

    protected void setUp() throws Exception {
        m_rule = new LongDistanceRule();
        m_rule.setLongDistancePrefix("1");
        m_rule.setPstnPrefix("9");
        m_rule.setExternalLen(7);
        
        Gateway g = new Gateway();
        g.setAddress("longdistance.gateway.com");
        m_rule.setGateways(Collections.singletonList(g));        
    }

    public void testGetPatterns() {
        String[] patterns = m_rule.getPatterns();
        assertEquals(3, patterns.length);
        assertEquals("91xxxxxxx", patterns[0]);
        assertEquals("1xxxxxxx", patterns[1]);
        assertEquals("xxxxxxx", patterns[2]);
    }

    public void testGetTransforms() {
        Transform[] transforms = m_rule.getTransforms();
        assertEquals(1,transforms.length);
        FullTransform transform = (FullTransform) transforms[0];
        assertEquals("1{vdigits}", transform.getUser());
        assertEquals("longdistance.gateway.com", transform.getHost());
    }

    public void testGetPermissions() {
        List permissions = m_rule.getPermissions();
        assertEquals(1, permissions.size());
        assertEquals(Permission.LONG_DISTANCE_DIALING, permissions.get(0));
    }

}
