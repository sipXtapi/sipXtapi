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

import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.UrlTransform;

/**
 * EmergencyRuleTest
 */
public class EmergencyRuleTest extends TestCase {
    private EmergencyRule m_rule;

    protected void setUp() throws Exception {
        m_rule = new EmergencyRule();
        m_rule.setEmergencyNumber("911");
        m_rule.setOptionalPrefix("9");

        Gateway g = new Gateway();
        g.setAddress("sosgateway.com");
        m_rule.setGateways(Collections.singletonList(g));
    }

    public void testGetPatterns() {
        String[] patterns = m_rule.getPatterns();
        assertEquals(3, patterns.length);
        assertEquals("sos", patterns[0]);
        assertEquals("911", patterns[1]);
        assertEquals("9911", patterns[2]);
    }

    public void testGetTransforms() {
        Transform[] transforms = m_rule.getTransforms();
        assertEquals(1, transforms.length);
        UrlTransform emergencyTransform = (UrlTransform) transforms[0];
        assertEquals("sip:911@sosgateway.com", emergencyTransform.getUrl());
    }

    public void testPermissions() {
        List permissions = m_rule.getPermissions();
        assertEquals(0, permissions.size());
    }
}
