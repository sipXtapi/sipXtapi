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
package org.sipfoundry.sipxconfig.admin.parkorbit;

import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbit;

import junit.framework.TestCase;

public class ParkOrbitTest extends TestCase {
    public void testCreateAlias() {
        ParkOrbit orbit = new ParkOrbit();
        orbit.setExtension("1002");
        AliasMapping alias = orbit.generateAlias("from.com", "to.com");
        assertEquals("1002@from.com", alias.getIdentity());
        assertEquals("1002@to.com", alias.getContact());
    }
}
