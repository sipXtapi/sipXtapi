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
package org.sipfoundry.sipxconfig.admin.dialplan;

import junit.framework.TestCase;

public class AttendantMenuActionTest extends TestCase {
    
    public void testVxmlParameter() {
        assertEquals("operatoraddr", AttendantMenuAction.OPERATOR.vxmlParameter(null));
        assertEquals("operatoraddr", AttendantMenuAction.OPERATOR.vxmlParameter("anything"));
        assertEquals("none", AttendantMenuAction.GOTO_EXTENSION.vxmlParameter(null));
        assertEquals("none", AttendantMenuAction.GOTO_EXTENSION.vxmlParameter("   "));
        assertEquals("voltar", AttendantMenuAction.GOTO_EXTENSION.vxmlParameter("voltar"));        
    }

}
