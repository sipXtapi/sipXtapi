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

import java.awt.TextArea;

import junit.framework.TestCase;
import org.sipfoundry.sipxphone.awt.event.PButtonEvent;


/**
 *  test for use of the PFormOverlayRegion
 */
public class PTaskFormOverlayTest extends TestCase
{
    public void setUp()
    {
    }

    public void tearDown()
    {
    }


    /**
     * tests out the initialize method
     */
    public void testInitialize()
    {
        try
        {
        	PFormOverlay overlay = new PTaskFormOverlay();
        	overlay.initialize();
        	
        	assertEquals(overlay.getRegionCount(), 11);
        	assertEquals(overlay.getRegion(0).getButtonID(), PButtonEvent.BID_L1);
        	assertEquals(overlay.getRegion(7).getButtonID(), PButtonEvent.BID_R4);
        	
        	java.awt.event.MouseEvent e  =
        		new java.awt.event.MouseEvent(new TextArea(), 0, 0, 0, 79, 120, 1, false);
        	
        	assertEquals(overlay.invokeButton(e), PButtonEvent.BID_L4);
       	
        }
        catch (Exception e)
        {
            e.printStackTrace();
            fail("PFormOverlayRegionTest failed.");
        }
    }
}