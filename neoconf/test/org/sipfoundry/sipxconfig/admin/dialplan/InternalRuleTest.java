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

import org.sipfoundry.sipxconfig.admin.dialplan.config.Permission;
import org.sipfoundry.sipxconfig.admin.dialplan.config.UrlTransform;

/**
 * InternalRuleTest
 */
public class InternalRuleTest extends TestCase {

    public void testAppendToGenerationRules() throws Exception {
        InternalRule ir = new InternalRule();
        ir.setLocalExtensionLen(5);
        ir.setAutoAttendant("20003");
        ir.setVoiceMail("20004");
        ir.setVoiceMailPrefix("7");
        ir.setEnabled(true);

        List rules = new ArrayList();
        ir.appendToGenerationRules(rules);

        assertEquals(4, rules.size());

        MappingRule o = (MappingRule) rules.get(0);
        MappingRule v = (MappingRule) rules.get(1);
        MappingRule vt = (MappingRule) rules.get(2);
        MappingRule vf = (MappingRule) rules.get(3);

        assertEquals("20003", o.getPatterns()[1]);
        assertEquals(0, o.getPermissions().size());
        UrlTransform to = (UrlTransform) o.getTransforms()[0];
        assertEquals(MappingRule.OPERATOR_URL, to.getUrl());
        
        assertEquals("20004", v.getPatterns()[0]);
        assertEquals(0, v.getPermissions().size());
        UrlTransform tv = (UrlTransform) v.getTransforms()[0];
        assertEquals(MappingRule.VOICEMAIL_URL, tv.getUrl());

        assertEquals("7xxxxx", vt.getPatterns()[0]);
        assertEquals(0, vt.getPermissions().size());
        UrlTransform tvt = (UrlTransform) vt.getTransforms()[0];
        assertEquals(MappingRule.VOICEMAIL_TRANSFER_URL, tvt.getUrl());
        
        assertEquals("xxxxx", vf.getPatterns()[0]);
        assertEquals(Permission.VOICEMAIL, vf.getPermissions().get(0));
        UrlTransform tvf = (UrlTransform) vf.getTransforms()[0];
        assertEquals(MappingRule.VOICEMAIL_FALLBACK_URL, tvf.getUrl());
    }

}
