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

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
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
        m_rule.setEnabled(true);
        m_rule.setLongDistancePrefix("1");
        m_rule.setPstnPrefix("9");
        m_rule.setExternalLen(7);
        
        Gateway g = new Gateway();
        g.setAddress("longdistance.gateway.com");
        m_rule.setGateways(Collections.singletonList(g));        
    }
    
    private DialingRule getGenerationRule(DialingRule rule) {
        List list = new ArrayList();
        rule.appendToGenerationRules(list);
        return (DialingRule) list.get(0);
    }

    public void testGetPatterns() {
        DialingRule rule = getGenerationRule(m_rule);
        String[] patterns = rule.getPatterns();
        assertEquals(3, patterns.length);
        assertEquals("91xxxxxxx", patterns[0]);
        assertEquals("1xxxxxxx", patterns[1]);
        assertEquals("xxxxxxx", patterns[2]);
    }

    public void testGetTransforms() {
        DialingRule rule = getGenerationRule(m_rule);
        Transform[] transforms = rule.getTransforms();
        assertEquals(1,transforms.length);
        FullTransform transform = (FullTransform) transforms[0];
        assertEquals("1{vdigits}", transform.getUser());
        assertEquals("longdistance.gateway.com", transform.getHost());
    }

    public void testGetPermissions() {
        DialingRule rule = getGenerationRule(m_rule);
        List permissions = rule.getPermissions();
        assertEquals(1, permissions.size());
        assertEquals(Permission.LONG_DISTANCE_DIALING, permissions.get(0));
    }
    
    public void testCalculateDialPatterns() {
        List list = m_rule.calculateDialPatterns("305");
        assertEquals(3, list.size());
        DialPattern pattern = (DialPattern) list.get(0);
        assertEquals("91305xxxx", pattern.calculatePattern());
        pattern = (DialPattern) list.get(1);
        assertEquals("1305xxxx", pattern.calculatePattern());
        pattern = (DialPattern) list.get(2);
        assertEquals("305xxxx", pattern.calculatePattern());
    }

    public void testCalculateCallPattern() {
        CallPattern callPattern = m_rule.calculateCallPattern("503");
        assertEquals("1503{vdigits}", callPattern.calculatePattern());
    }
    
    public void testAreaCodes() {
        m_rule.setAreaCodes("  305 411,222");
        List list = new ArrayList();
        m_rule.appendToGenerationRules(list);
        assertEquals(3, list.size());
        for (Iterator i = list.iterator(); i.hasNext();) {
            CustomDialingRule r = (CustomDialingRule) i.next();
            String[] patterns = r.getPatterns();
            assertEquals(3, patterns.length);
            assertTrue(patterns[0].endsWith("xxxx"));
            assertTrue(patterns[1].endsWith("xxxx"));
            assertTrue(patterns[2].endsWith("xxxx"));
            List permissions = r.getPermissions();
            assertEquals(1, permissions.size());
            assertEquals(Permission.LONG_DISTANCE_DIALING, permissions.get(0));
            Transform[] transforms = r.getTransforms();
            assertEquals(1,transforms.length);
            FullTransform transform = (FullTransform) transforms[0];
            assertTrue(transform.getUser().endsWith("{vdigits}"));
            assertEquals("longdistance.gateway.com", transform.getHost());
        }
    }

    public void testTollFreeDialing() {
        m_rule.setPermission(Permission.TOLL_FREE_DIALING);
        DialingRule generationRule = getGenerationRule(m_rule);
        List permissions = generationRule.getPermissions();
        assertSame(Permission.TOLL_FREE_DIALING,permissions.get(0));
    }   
}
