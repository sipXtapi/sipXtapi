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

import java.util.Arrays;
import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;

/**
 * CustomDialingRuleTest
 */
public class CustomDialingRuleTest extends TestCase {
    private static final int PATTERN_COUNT = 10;
    private static final String[] GATEWAYS = {
        "10.2.3.4", "10.4.5.6"
    };
    private CustomDialingRule m_rule;
    private CallPattern m_callPattern;
    private List m_patternsList;

    protected void setUp() throws Exception {
        DialPattern[] dialPatterns = new DialPattern[PATTERN_COUNT];
        for (int i = 0; i < dialPatterns.length; i++) {
            DialPattern p = new DialPattern();
            p.setPrefix("91");
            p.setDigits(i + 2);
            dialPatterns[i] = p;
        }
        m_patternsList = Arrays.asList(dialPatterns);
        m_callPattern = new CallPattern();
        m_callPattern.setDigits(CallDigits.VARIABLE_DIGITS);
        m_callPattern.setPrefix("999");

        m_rule = new CustomDialingRule();
        m_rule.setDialPatterns(m_patternsList);

        for (int i = 0; i < GATEWAYS.length; i++) {
            Gateway gateway = new Gateway();
            gateway.setUniqueId();
            gateway.setAddress(GATEWAYS[i]);
            m_rule.addGateway(gateway);
        }

        m_rule.setEnabled(true);
        m_rule.setCallPattern(m_callPattern);
    }

    public void testGetPatterns() {
        String[] patterns = m_rule.getPatterns();
        assertEquals(PATTERN_COUNT, patterns.length);
        for (int i = 0; i < patterns.length; i++) {
            String p = patterns[i];
            assertTrue(p.startsWith("91"));
        }
    }

    public void testGetTransforms() {
        Transform[] transforms = m_rule.getTransforms();
        assertEquals(GATEWAYS.length, transforms.length);
        for (int i = 0; i < GATEWAYS.length; i++) {
            assertTrue(transforms[i] instanceof FullTransform);
            FullTransform full = (FullTransform) transforms[i];
            assertTrue(full.getFieldParams()[0].startsWith("q="));
            assertEquals(GATEWAYS[i], full.getHost());
            assertTrue(full.getUser().startsWith("999"));
        }
    }

    public void testNoGateways() {
        CustomDialingRule rule = new CustomDialingRule();
        rule.setDialPatterns(m_patternsList);
        rule.setEnabled(true);
        rule.setCallPattern(m_callPattern);

        String[] patterns = rule.getPatterns();
        assertEquals(PATTERN_COUNT, patterns.length);
        for (int i = 0; i < patterns.length; i++) {
            String p = patterns[i];
            assertTrue(p.startsWith("91"));
        }

        Transform[] transforms = rule.getTransforms();
        assertEquals(1, transforms.length);
        FullTransform tr = (FullTransform) transforms[0];
        assertEquals("999{vdigits}", tr.getUser());
        assertNull(tr.getHost());
    }
}
