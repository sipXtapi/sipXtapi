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
import org.sipfoundry.sipxphone.service.Timer;
import org.sipfoundry.sipxphone.sys.app.shell.PingtelEventQueue;

/**
 * Container for a collection of rectangular areas and 
 * invocation actions that are associated with a form.
 * The PFormOverlay is used in order to determine the desired action resulting
 * from a mouse-click event.
 */
public class PFormOverlay
{
//////////////////////////////////////////////////////////////////////////////
//	 Constants
////
	protected static final int MAX_REGIONS_IN_FORM = 40;
	
//////////////////////////////////////////////////////////////////////////////
//	 Attributes
////
	protected PFormOverlayRegion[] m_regions;
	protected int m_regionCount = 0;
	
//////////////////////////////////////////////////////////////////////////////
//	 Construction
////
	/**
	 * Constructor.
	 */
	public PFormOverlay()
	{
		m_regions = new PFormOverlayRegion[MAX_REGIONS_IN_FORM];
	}
	
//////////////////////////////////////////////////////////////////////////////
//	 Public Methods
////
	/**
	 * Creates regions for this overlay.  Meant to be overridden by
	 * derived classes.
	 */
	public void initialize()
	{
		// add regions corresponding to the left menu
		addRegion(25, 0, 27, 27, PButtonEvent.BID_L1);
		addRegion(52, 0, 27, 27, PButtonEvent.BID_L2);
		addRegion(79, 0, 27, 27, PButtonEvent.BID_L3);
		addRegion(106, 0, 27, 27, PButtonEvent.BID_L4);
		
		// add regions corresponding to the right menu
		addRegion(25, 28, 27, 132, PButtonEvent.BID_R1);
		addRegion(52, 28, 27, 132, PButtonEvent.BID_R2);
		addRegion(79, 28, 27, 132, PButtonEvent.BID_R3);
		addRegion(106, 28, 27, 132, PButtonEvent.BID_R4);
		
		// add regions corresponding to the bottom menu
		addRegion(133, 0, 27, 53, PButtonEvent.BID_B1);
		addRegion(133, 53, 27, 53, PButtonEvent.BID_B2);
		addRegion(133, 106, 27, 53, PButtonEvent.BID_B3);
	}
	
	/**
	 * Creates a new region, adding it to the tail of the regions array
	 * @param top y-coordinate of upper left corner
	 * @param left x-coordinate of upper left corner
	 * @param height height of regions in pixels
	 * @param width width of region in pixels
	 * @param buttonID the button ID to associate with this region
	 */
	public void addRegion(int top, int left, int height, int width, int buttonID)
	{
		m_regions[m_regionCount] = new PFormOverlayRegion(top, left, height, width, buttonID);
		m_regionCount++;
	}
	
	/**
	 * @return the number or regions in the collection
	 */
	public int getRegionCount()
	{
		return m_regionCount;
	}
	
	/**
	 * Accessor for individual regions in the collection
	 * @param index zero based index into the array
	 * @return the indicated PFormOverlayRegion object
	 */
	public PFormOverlayRegion getRegion(int index)
	{
		return m_regions[index];
	}
	
	/**
	 * Posts a button event, with the appropriate button id.
	 * Button id is determined based on the xy coordinate of
	 * the MouseEvent
	 * @param e MouseEvent to examine to determine the desired button id
	 */
	public int invokeButton(java.awt.event.MouseEvent e)
	{
		PFormOverlayRegion currentRegion;
		int invokedButton = 0;
		for (int i=0; i < m_regionCount; i++)
		{
			currentRegion = m_regions[i];
			if (e.getX() >= currentRegion.getLeft() && 
				e.getX() < (currentRegion.getLeft() + currentRegion.getWidth()) )
			{
				if (e.getY() >= currentRegion.getTop() && 
						e.getY() < (currentRegion.getTop() + currentRegion.getHeight()) )
					{
						invokedButton = currentRegion.getButtonID();
						postButtonEvent(currentRegion.getButtonID());
					}
			}
		}
		return invokedButton;
	}
	
	/**
	 * Creates a PButtonEvent for the given buttonID
	 * @param buttonID ID of the button
	 */
	private void postButtonEvent(int buttonID)
	{
		PButtonEvent newEvent = new PButtonEvent(this, PButtonEvent.BUTTON_UP, buttonID) ;
		PingtelEventQueue.postEvent(newEvent);	
	}
    
}
