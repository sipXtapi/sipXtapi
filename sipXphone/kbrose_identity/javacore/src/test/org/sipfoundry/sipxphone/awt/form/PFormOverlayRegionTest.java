/*
 * $Id:  $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package org.sipfoundry.sipxphone.awt.form;

import junit.framework.TestCase;


/**
 *  test for use of the PFormOverlay
 */
public class PFormOverlayRegionTest extends TestCase
{

    public void setUp()
    {
    }

    public void tearDown()
    {
    }


  /**
     * tests the PFormOverlayRegion
     */
    public void testPFormOvelayRegion()
    {
        try
        {
        	PFormOverlayRegion region;
        	region = new PFormOverlayRegion(1, 2, 3, 4, 999);
        	
        	assertEquals(region.getTop(), 1);
        	assertEquals(region.getLeft(), 2);
        	assertEquals(region.getHeight(), 3);
        	assertEquals(region.getWidth(), 4);
        	assertEquals(region.getButtonID(), 999);
        }
        catch (Exception e)
        {
            e.printStackTrace();
            fail("PFormOverlayRegionTest failed.");
        }
    }
}
