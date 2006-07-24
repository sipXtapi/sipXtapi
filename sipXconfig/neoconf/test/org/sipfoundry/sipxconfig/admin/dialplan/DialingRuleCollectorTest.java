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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;

public class DialingRuleCollectorTest extends TestCase {

    public void testGetDialingRuleProviders() {
        final int len = 10;
        final IMocksControl[] drpCtrl = new IMocksControl[len];
        final DialingRuleProvider[] drp = new DialingRuleProvider[len];

        // create a dummy dialing rule and put it in a list
        IDialingRule rule = new IntercomRule(true, "*88", "intercomCode");
        List<IDialingRule> rules = new ArrayList<IDialingRule>();
        rules.add(rule);

        for (int i = 0; i < len; i++) {
            drpCtrl[i] = EasyMock.createControl();
            drp[i] = drpCtrl[i].createMock(DialingRuleProvider.class);
            drp[i].getDialingRules();
            drpCtrl[i].andReturn(rules).anyTimes();
            drpCtrl[i].replay();
        }
        
        DialingRuleCollector collector = new DialingRuleCollector() {
            protected Collection getDialingRuleProviders() {
                return Arrays.asList(drp);
            }
        };

        rules = collector.getDialingRules();
        assertEquals(len, rules.size());
        for (Iterator i = rules.iterator(); i.hasNext();) {
            assertSame(rule, i.next());
        }

        for (int i = 0; i < len; i++) {
            drpCtrl[i].verify();
        }
    }
    
}
