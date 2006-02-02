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
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;

public class AliasCollectorTest extends TestCase {

    public void testGetAliasMappings() {
        final int len = 10;
        final MockControl[] apCtrl = new MockControl[len];
        final AliasProvider[] ap = new AliasProvider[len];

        AliasMapping alias = new AliasMapping();

        for (int i = 0; i < len; i++) {
            apCtrl[i] = MockControl.createControl(AliasProvider.class);
            ap[i] = (AliasProvider) apCtrl[i].getMock();
            ap[i].getAliasMappings();
            apCtrl[i].setDefaultReturnValue(Collections.singleton(alias));
            apCtrl[i].replay();
        }
        
        AliasCollector collector = new AliasCollector() {
            protected Collection getAliasProviders() {
                return Arrays.asList(ap);
            }
        };

        Collection aliasMappings = collector.getAliasMappings();
        assertEquals(len, aliasMappings.size());
        for (Iterator i = aliasMappings.iterator(); i.hasNext();) {
            assertSame(alias, i.next());
        }

        for (int i = 0; i < len; i++) {
            apCtrl[i].verify();
        }
    }
}
