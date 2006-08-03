/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import org.sipfoundry.sipxconfig.admin.dialplan.config.FullTransform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;

import junit.framework.TestCase;

public class IntercomRuleTest extends TestCase {
    private IntercomRule m_rule;
    
    protected void setUp() {
        m_rule = new IntercomRule(true, "*78", "Ipek");
    }
    
    public void testGetPatterns() {
        String[] patterns = m_rule.getPatterns();
        assertEquals(1, patterns.length);
        assertEquals("*78" + ".", patterns[0]);
    }

    public void testGetTransforms() {
        Transform[] transforms = m_rule.getTransforms();
        assertEquals(1, transforms.length);
        FullTransform transform = (FullTransform) transforms[0];
        assertEquals("{vdigits}", transform.getUser());
        String[] headerparams = transform.getHeaderParams();
        assertEquals(1, headerparams.length);
        assertEquals("Alert-info=Ipek", headerparams[0]);
        assertNull(transform.getHost());
    }    
}
