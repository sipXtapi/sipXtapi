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

import org.sipfoundry.sipxphone.awt.event.PButtonEvent;

/**
 * Container for a collection of rectangular areas 
 * specific to the PTaskForm
 */
public class PTaskFormOverlay extends PFormOverlay
{
//////////////////////////////////////////////////////////////////////////////
//	 Construction
////
	/**
	 *  Constructor.
	 */
	public PTaskFormOverlay()
	{
		super();
	}
	
//////////////////////////////////////////////////////////////////////////////
//	 Public Methods
////
	/**
	 * Creates regions for this overlay.  Overrides base class implementation.
	 */
	public void initialize()
	{
		// add regions corresponding to the left menu
		addRegion(25, 0, 27, 80, PButtonEvent.BID_L1);
		addRegion(52, 0, 27, 80, PButtonEvent.BID_L2);
		addRegion(79, 0, 27, 80, PButtonEvent.BID_L3);
		addRegion(106, 0, 27, 80, PButtonEvent.BID_L4);
		
		// add regions corresponding to the right menu
		addRegion(25, 80, 27, 80, PButtonEvent.BID_R1);
		addRegion(52, 80, 27, 80, PButtonEvent.BID_R2);
		addRegion(79, 80, 27, 80, PButtonEvent.BID_R3);
		addRegion(106, 80, 27, 80, PButtonEvent.BID_R4);
		
		// add regions corresponding to the bottom menu
		addRegion(133, 0, 27, 53, PButtonEvent.BID_B1);
		addRegion(133, 53, 27, 53, PButtonEvent.BID_B2);
		addRegion(133, 106, 27, 53, PButtonEvent.BID_B3);
	}

}
