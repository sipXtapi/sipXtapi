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
package org.sipfoundry.sipxconfig.admin;

import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.common.Patch;
import org.springframework.beans.factory.ListableBeanFactory;

public class PatchRunnerTest extends TestCase {
    
    PatchRunner patchRunner;
    
    protected void setUp() {
        patchRunner = new PatchRunner();
    }
    
    public void testGetPatchName() {
        assertEquals("name", patchRunner.getPatchName(new String[] { "name" }));
        try {
            patchRunner.getPatchName(null);
            fail();
        } catch (IllegalArgumentException e) {
            assertTrue(true);
        }
    }
    
    public void testApplyPatch() {        
        MockControl adminControl = MockControl.createStrictControl(AdminContext.class);        
        AdminContext admin = (AdminContext) adminControl.getMock();
        admin.setPatchApplied("unittest");
        adminControl.replay();
        
        MockControl patchControl = MockControl.createStrictControl(Patch.class);        
        Patch patch = (Patch) patchControl.getMock();
        patchControl.expectAndReturn(patch.applyPatch("unittest"), true);
        patchControl.replay();
        
        Map patches = new HashMap();
        patches.put("patchBean", patch);
        
        MockControl lbfControl = MockControl.createStrictControl(ListableBeanFactory.class);        
        ListableBeanFactory lbf = (ListableBeanFactory) lbfControl.getMock();
        lbfControl.expectAndReturn(lbf.getBean("adminContext"), admin);
        lbfControl.expectAndReturn(lbf.getBeansOfType(Patch.class), patches);        
        lbfControl.replay();

        patchRunner.applyPatch("unittest", lbf);
        
        adminControl.verify();
        lbfControl.verify();
        patchControl.verify();
    }
}
