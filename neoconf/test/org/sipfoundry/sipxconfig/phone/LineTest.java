/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import junit.framework.TestCase;

/**
 * @author dhubler
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class LineTest extends TestCase {

    public void testMoveLine() {
        Endpoint e = new Endpoint();
        Line l0 = new Line();
        e.addLine(l0);
        Line l1 = new Line();
        e.addLine(l1);        
        Line l2 = new Line();
        e.addLine(l2);
        e.moveLine(1, -1);
        assertEquals(l0, e.getLines().get(1));
        assertEquals(1, l0.getPosition());
        assertEquals(l1, e.getLines().get(0));
        assertEquals(0, l1.getPosition());
        assertEquals(l2, e.getLines().get(2));
        assertEquals(2, l2.getPosition());
    }
}
