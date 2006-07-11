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

/**
 * PFormOverlayRegion objects contain rectangular 
 * coordinates and an associated button ID
  */
public class PFormOverlayRegion
{
//////////////////////////////////////////////////////////////////////////////
//	 Attributes
////
	private int m_top = 0;
	private int m_left = 0;
	private int m_height = 0;
	private int m_width = 0;
	private int m_buttonID = 0;
	
//////////////////////////////////////////////////////////////////////////////
//	 Construction
////
	/**
	 * Constructor.
	 */
	public PFormOverlayRegion(int top, int left, int height, int width, int buttonID)
	{
		m_top = top;
		m_left = left;
		m_height = height;
		m_width = width;
		m_buttonID = buttonID;
	}
	
//////////////////////////////////////////////////////////////////////////////
//	 Public Methods
////
	/**
	 * the x-coordinate of the top left corner of the rectangle
	 */
	public int getTop()
	{
		return m_top;
	}
	
	/**
	 * the y-coordinate of the top left corner of the rectangle
	 */
	public int getLeft()
	{
		return m_left;
	}
	
	/**
	 * the height of the rectangle in pixels
	 */
	public int getHeight()
	{
		return m_height;
	}
	
	/**
	 * width of the rectangle in pixels 
	 */
	public int getWidth()
	{
		return m_width;
	}
	
	/**
	 * ID of the button to invoke when this region is clicked into 
	 */
	public int getButtonID()
	{
		return m_buttonID;
	}
}
