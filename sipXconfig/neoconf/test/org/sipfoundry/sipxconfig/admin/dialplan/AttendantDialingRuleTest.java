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
import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.UrlTransform;

public class AttendantDialingRuleTest extends TestCase {
    private static final String URL_PARAMS = ";play={voicemail}%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3D";
    private static final String OPERATOR_URL = "<sip:{digits}@{mediaserver}" + URL_PARAMS
            + "autoattendant%26name%3Daa_-1>";    
    
    public void testNotImplemented() {
        AttendantDialingRule rule = new AttendantDialingRule();
        assertNull(rule.getPatterns());
        assertNull(rule.getTransforms());
    }

    public void testAppendToGenerationRulesDisabled() {
        AttendantDialingRule rule = new AttendantDialingRule();
        List list = new ArrayList();
        rule.appendToGenerationRules(list);
        // do not add anything if rule is disabled and it is disabled by default
        assertTrue(list.isEmpty());
    }

    public void testAppendToGenerationRules() {
        AttendantDialingRule rule = new AttendantDialingRule();
        rule.setName("abc");
        rule.setExtension("100");
        rule.setAttendantAliases("0, operator");
        rule.setEnabled(true);

        List list = new ArrayList();
        rule.appendToGenerationRules(list);
        // do not add anything if rule is disabled and it is disabled by default
        assertFalse(list.isEmpty());

        Object firstRule = list.get(0);
        assertTrue(firstRule instanceof MappingRule.Operator);

        DialingRule dr = (DialingRule) firstRule;
        String[] patterns = dr.getPatterns();
        assertEquals(3, patterns.length);
        assertEquals("100", patterns[0]);
        assertEquals("0", patterns[1]);
        assertEquals("operator", patterns[2]);

        Transform[] transforms = dr.getTransforms();
        assertEquals(1, transforms.length);
        assertTrue(transforms[0] instanceof UrlTransform);
        UrlTransform urlTransform = (UrlTransform) transforms[0];

        assertEquals(OPERATOR_URL, urlTransform.getUrl());
    }

    public void testIsInternal() {
        AttendantDialingRule rule = new AttendantDialingRule();
        // this is internal rule
        assertTrue(rule.isInternal());
    }
}
