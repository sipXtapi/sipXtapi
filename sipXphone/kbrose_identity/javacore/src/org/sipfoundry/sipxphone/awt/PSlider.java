/*
 * $Id$
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.awt ;

import java.awt.* ;
import java.awt.event.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.awt.event.* ;


/**
 * This class creates a horizontal slider on the screen.
 * <br><br>
 * The constructor can take the minimum and maximum values of the slider, plus
 * labels for the minimum and maximum values.
 *
 */
public class PSlider extends PComponent
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** size of our slider */
    protected static final Dimension DIM_SLIDER_SIZE = new Dimension(117,15) ;
    /** Size of slider unit area */
    protected static final Rectangle RECT_SLIDER_UNIT_SIZE = new Rectangle(0,0,114,12) ;
    /** Overall drawing area */
    protected static final Rectangle RECT_DRAWING_AREA   = new Rectangle(0, 0, 117, 25) ;
    /** Relative Y Position of text */
    protected static final int Y_TEXT_OFFSET = 10 ;
    /** Relative X Position of Slider */
    protected static final int X_SLIDER_OFFSET = 5 ;
    /** Relative Y Position of Slider */
    protected static final int Y_SLIDER_OFFSET = 15 ;
    /** Relative X Position of slider value i.e. the slider units */
    protected static final int X_SLIDER_UNIT_OFFSET = 7 ;
    /** Relative Y Position of slider value i.e. the slider units */
    protected static final int Y_SLIDER_UNIT_OFFSET = 2 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////

    /** Number of positions within slider */
    protected int  m_iNumPositions = 0 ;
    /** What level or position are we currently at? */
    protected int   m_iPosition = -1 ;
    /** Our slider image */
    protected Image m_imgSlider ;
    /** Labels for slider text */
    protected String m_strMin ;
    protected String m_strMax ;
    /** Range of possible values for slider */
    protected int   m_iLow ;
    protected int   m_iHigh ;
    /** Offsets */
    protected int   m_iMinOffset ;
    protected int   m_iMaxOffset ;
    /** Whether to show current value */
    protected boolean m_bHideCurrentValue = false ;

//////////////////////////////////////////////////////////////////////////////
// Constructor
////
    /**
     * Default constructor.
     */
    public PSlider()
    {
     this("min","max", 0, 10, 10) ;
    }

    /**
     * Creates a slider component.
     * @param strMin The label for the left-hand slider label
     * @param strMax The label for the right-hand slider label
     * @param iLow The lowest possible value
     * @param iHigh The highest possible value
     * @param iCurrent The initial value (position) of the slider
     */
    public PSlider(String strMin, String strMax, int iLow, int iHigh, int iInitial)
    {
        // Snag our images
        m_imgSlider = getImage("imgSlider") ;

        setValues(strMin, strMax, iLow, iHigh, iInitial) ;
        enableDoubleBuffering(true) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public methods
////

    /**
     * Call by framework when we are supposed to render a view of ourselves.
     *
     * @param g Graphics context
     */
    public void paint(Graphics g)
    {
        super.paint(g) ;
        Dimension dimSize = getSize() ;
        undrawSlider(g, m_iPosition) ;
        drawSlider(g, m_iPosition) ;
    }

   /**
    * Set values for the slider - this can optionally be done as part of the constructor
    *
    */
    public void setValues(String strMin, String strMax, int iLow, int iHigh, int iInitial)
    {

        // Set values to our data
        m_strMin    = strMin ;
        m_strMax    = strMax ;
        m_iLow      = iLow ;
        m_iHigh     = iHigh ;

        /* This could be extended to allow a specific number of values within the slider.
            For example, only 5 jumps from the min to the max. */
        m_iNumPositions = m_iHigh - m_iLow ;

        // Set the initial position of the slider
        setPosition(iInitial) ;

    }

    /**
     * Increase the slider by 1 unit.
     * Note currently there is on unit for every integer between the high and low values
     */

    public void setUp()
    {
        if (m_iPosition >= m_iHigh)
            m_iPosition = m_iHigh ;
        else
            m_iPosition++ ;

        repaint() ;
//System.out.println("PSlider::setUp() min: " + String.valueOf(m_iLow) + " max: " + String.valueOf(m_iHigh) + " current: " + m_iPosition) ;
    }


   /**
    * Decrease the slider by 1 unit.
    */

    public void setDown()
    {
        if (m_iPosition <= m_iLow)
            m_iPosition = m_iLow ;
        else
            m_iPosition-- ;

        repaint() ;
    }


    /**
     * Set the position of the slider.
     * @iPosition The slider position
     */
    public void setPosition(int iPosition)
    {
        if (iPosition > m_iHigh)
            m_iPosition = m_iHigh ;
        else if (iPosition < m_iLow)
            m_iPosition = m_iLow ;
        else
            m_iPosition = iPosition ;

//System.out.println("setPosition: " + iPosition) ;

        repaint() ;
    }


   /**
    * Class to hide the value shown on top of the slider, with the slider's
    * current value. For some cases, the actual value of the slider may be confusing
    * to show to the user.
    *
    * @param bHide Whether to show/hide the slider value.
    */
    public void hideCurrentValue(boolean bHide)
    {
        m_bHideCurrentValue = bHide ;
    }

    /**
     * Gets the text for the min position.
     * @return text used for the min position.
     * @deprecated DO NOT EXPOSE YET
     */
    public String getMinText(){
        return m_strMin;
    }

    /**
     * Gets the text for the max position.
     * @return text used for the max position.
     * @deprecated DO NOT EXPOSE YET
     */
    public String getMaxText(){
        return m_strMax;
    }

    /**
     * gets the position of the slider.
     * @return the position of the slider
     * @deprecated DO NOT EXPOSE YET
     */
    public int getPosition(){
        return m_iPosition;
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Draw the slider.
     * @param g Reference to our graphics context
     * @param iPosition Current position of slider
     */
    public void drawSlider(Graphics g, int iPosition)
    {
            g.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT)) ;

            // Set Offset ie the distance from NW and NE corners where we should start to draw text
            int iMinOffset = GUIUtils.calcXOffset(m_strMin, g, RECT_DRAWING_AREA, GUIUtils.ALIGN_WEST | GUIUtils.ALIGN_NORTH) ;
            int iMaxOffset = GUIUtils.calcXOffset(m_strMin, g, RECT_DRAWING_AREA, GUIUtils.ALIGN_EAST | GUIUtils.ALIGN_NORTH) ;

//System.out.println("PSlider::drawSlider() iMinOffset: " + iMinOffset + " iMaxOffset: " + iMaxOffset) ;

            // Draw labels
            g.drawString(m_strMin, iMinOffset + X_SLIDER_OFFSET, RECT_DRAWING_AREA.y + Y_TEXT_OFFSET) ;
            g.drawString(m_strMax, iMaxOffset, RECT_DRAWING_AREA.y + Y_TEXT_OFFSET) ;

            // Can optionally draw the current value of the slider in the middle of the slider component
            if (!m_bHideCurrentValue)
                g.drawString(String.valueOf(m_iPosition), (int) RECT_SLIDER_UNIT_SIZE.width/2, RECT_DRAWING_AREA.y + Y_TEXT_OFFSET) ;

            // Draw Slider background
            g.drawImage(m_imgSlider, RECT_DRAWING_AREA.x + X_SLIDER_OFFSET, RECT_DRAWING_AREA.y + Y_SLIDER_OFFSET, this) ;

            // Draw Slider Units
            int iRange = RECT_DRAWING_AREA.width - 4;

            float dec_percent = (float) (m_iPosition-m_iLow) / (float) (m_iHigh-m_iLow);
            int iCurrPosition = (int) (dec_percent * (float) iRange) ;

            for (int i =0; i< iCurrPosition; i++) {
                int iSpacing = (int) iRange/10 ; // How often we have a gap in display of units
                if (i % iSpacing != 0)
                    g.drawLine(RECT_DRAWING_AREA.x + X_SLIDER_UNIT_OFFSET + i, RECT_DRAWING_AREA.y + Y_SLIDER_OFFSET + Y_SLIDER_UNIT_OFFSET , RECT_DRAWING_AREA.x + 2 + 5 + i, RECT_DRAWING_AREA.y+27) ;
            }
    }


    /**
     * Undraw the slider.
     * @param g Reference to our graphics context
     * @param iPosition Current position of slider
     */
    public void undrawSlider(Graphics g, int iPosition)
    {
            // Just redraw the entire scrollbar...
            g.drawImage(m_imgSlider, RECT_DRAWING_AREA.x + X_SLIDER_OFFSET, RECT_DRAWING_AREA.y + Y_SLIDER_OFFSET, this) ;
    }
}

