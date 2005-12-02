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
import java.util.* ;

import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.* ;

import java.awt.* ;
import java.util.* ;

/**
 * PItemRender is a basic text render that can display text aligned anywhere
 * within a 3x3 grid.  NORTH, SOUTH, EAST, WEST and CENTER constants have
 * been defined and can be combined, e.g. (NORTH | EAST).
 * <br><br>
 * Selected and non-selected Foreground and Background colors can be specified
 * by using setColor, setSelColor, setBkColor and setSelBkColor.
 * <br><br>
 */
public class PDefaultItemRenderer extends PComponent implements PItemRenderer, Cloneable
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** align the component to the east along the horizonal axis */
    public static final int   EAST      = 0x0010 ;
    /** align the component to the west along the horizonal axis */
    public static final int   WEST      = 0x0001 ;
    /** align the component to the center of both the horizonal and vertical
        axis.  This is the default if a horizontal or vertical alignment is
        not specified */
    public static final int   CENTER    = 0x0000 ;
    /** align the component to the north along the vertical axis */
    public static final int   NORTH     = 0x1000 ;
    /** align the component to the south along the vertical axis */
    public static final int   SOUTH     = 0x0100 ;

    /** Screen width:160 - PCommandBar width */
    private static final int MIN_ALIGN_WEST_WIDTH = 130;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** item that we are displaying */
    protected Object      m_objData ;
    /** is this item selected? */
    protected boolean        m_bSelected ;
    /** dimensions of this item (calculated once and retrieved many times) */
    protected Dimension   m_dimSize ;

    /** how should we align our text? */
    protected int         m_iAlignment ;

    /** unless explicitly set, auto align based on hueristics */
    private boolean     m_isAutoAlign = true;

    /** font that will be used when rendering*/
    protected Font        m_fontText ;
    /** non-selected text color used when rendering */
    protected Color       m_colorText ;
    /** selected text color used when rendering */
    protected Color       m_colorSelText ;
    /** non-selected background color used when rendering */
    protected Color       m_colorBackground ;
    /** selected background color used when rendering */
    protected Color       m_colorSelBackground ;


    /**
     * Stock no argument constructor
     */
    public PDefaultItemRenderer()
    {
        this(WEST) ;
        m_isAutoAlign = true;
    }


    /**
     * Constructor taking an initial alignment
     */
    public PDefaultItemRenderer(int iAlignment)
    {
        m_fontText = SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_LARGE) ;

        m_iAlignment = iAlignment ;
        m_isAutoAlign = false;

        m_colorText          = SystemDefaults.getColor(SystemDefaults.COLORID_TEXT) ;
        m_colorSelText       = SystemDefaults.getColor(SystemDefaults.COLORID_TEXT_SELECTED) ;

        m_colorBackground    = SystemDefaults.getColor(SystemDefaults.COLORID_BACKGROUND) ;
        m_colorSelBackground = SystemDefaults.getColor(SystemDefaults.COLORID_BACKGROUND_SELECTED) ;

        m_bSelected = false ;

        setOpaque(true) ;
    }



    /**
     * return the actual component
     */
    public Component getComponent(Object objSource, Object objValue, boolean bSelected)
    {
        PDefaultItemRenderer renderer = createInstance() ;

        renderer.setData(objValue) ;
        renderer.setState(bSelected) ;

        return renderer  ;
    }


    /**
     * sets the alignment of text.
     *
     * @param iAlignment NORTH, SOUTH, EAST, WEST, and/or CENTER constants
     *        used to describe the alignment of the text.  Constants can be
     *        combined.
     */
    public void setAlignment(int iAlignment)
    {
        m_iAlignment = iAlignment ;
        m_isAutoAlign = false;
    }


    /**
     * get the alignment of text.
     *
     * @return alignment of text.  This is a bitmask and should be checked with
     *         NORTH, SOUTH, EAST, WEST constants, e.g. if
     *        (getAlignment & NORTH) ...
     */
    public int getAlignment()
    {
        if (m_isAutoAlign)
        {
            // NOTE:In an effort to fix existing applications that use
            // a PList that spans entire width of phone screen and
            // align text to the left when select buttons are on the
            // right.  Dumb users click buttons on wrong side. This is
            // not an issue when there is a PCommandBar down the left
            // or right side that helps user gravitate to correct
            // buttons irrelevant of PList text alignment.
            PList plist = getParentPList();
            if (plist != null)
            {
                Rectangle bounds = plist.getBounds();
                // Spans more than 3 quarters of screen
                return bounds.width > MIN_ALIGN_WEST_WIDTH ? EAST : WEST;
            }
        }

        return m_iAlignment ;
    }

    public boolean isAutoAlign()
    {
        return m_isAutoAlign;
    }

    private PList getParentPList()
    {
        Component p = getParent();
        while (p != null && !(p instanceof PList)) 
        {
            p = p.getParent();
        }

        return (PList)p;
    }


    /**
     * set the non-selected text color
     */
    public void setColor(Color color)
    {
        m_colorText = color ;
    }


    /**
     * set the selected text color
     */
    public void setSelColor(Color color)
    {
        m_colorSelText = color ;
    }


    /**
     * set the non-selected background color
     */
    public void setBkColor(Color color)
    {
        m_colorBackground = color ;
    }

    /**
     * get the non-selected background color
     */
    public Color getBkColor()
    {
        return m_colorBackground;
    }


    /**
     * set the selected background color
     */
    public void setSelBkColor(Color color)
    {
        m_colorSelBackground = color ;
    }

    /**
     * get the selected background color
     */
    public Color getSelBkColor()
    {
        return m_colorSelBackground;
    }


    public void doLayout()
    {
        m_dimSize = getSize() ;
        super.doLayout() ;
    }


    /**
     * set the state of the item.  Is it selected?
     *
     * @param bSelected boolean true if selected otherwise set to false
     */
    public void setState(boolean bSelected)
    {
        m_bSelected = bSelected ;
        if (m_bSelected) {
           setColor(m_colorSelText) ;
           setBackground(m_colorSelBackground) ;
        } else {
           setColor(m_colorText) ;
           setBackground(m_colorBackground) ;
        }
    }


    /**
     * set the object that we would like to display.  the toString()
     * representation will be displayed.
     */
    public void setData(Object objData)
    {
        m_objData = objData ;
    }


    /**
     * undraws the text by re-drawing the text using the background color
     */
    public void undrawItemText(Graphics g)
    {
        if (m_objData != null) {
            // Draw the background
            if (m_bSelected && (m_objData != null)) {
                g.setColor(m_colorSelBackground) ;
            } else {
                g.setColor(m_colorBackground) ;
            }

            drawItemText(g) ;
        }
    }


    /**
     * draw text at position x,y
     */
    public void drawItemText(Graphics g)
    {
        if (m_objData != null) {
            String strText = m_objData.toString() ;
            int xOffset = calcXOffset(g, strText, m_fontText, getAlignment()) ;
            int yOffset = calcYOffset(g, strText, m_fontText, getAlignment()) ;

            g.drawString(strText, xOffset, yOffset) ;
        }
    }


    /**
     * draw the background
     */
    public void drawItemBackground(Graphics g)
    {
        Dimension dim = getSize() ;
        g.fillRect(0, 0, dim.width, dim.height) ;
    }



    /**
     * draw a item at position x,y
     */
    public void drawItem(Graphics g)
    {
        prepareForBackgroundRendering(g) ;
        drawItemBackground(g) ;
        prepareForForegroundRendering(g) ;
        drawItemText(g) ;
    }


    /**
     * undraw a item at position x,y
     */
    public void undrawItem(Graphics g)
    {
        undrawItemText(g) ;
    }



    /**
     * set the desire font used in rendering
     */
    public void setFont(Font font)
    {
        m_fontText = font ;
    }


    /**
     * create a copy/clone of this renderer
     */
    public PDefaultItemRenderer createInstance()
    {
        PDefaultItemRenderer renderer = null ;
        try {
            renderer = (PDefaultItemRenderer) clone() ;
        } catch (CloneNotSupportedException cnse) {
            System.out.println(cnse) ;
        }
        return renderer ;
    }


    /**
     * Calculate the desired X Offset given the passed graphics context, string,
     * font, and alignment constraint.
     */
    public int calcXOffset(Graphics g, String strText, Font fontText, int iAlignment)
    {
        FontMetrics fm = g.getFontMetrics(fontText) ;
        Rectangle   rectBounds = getBounds() ;
        int         xOffset = 0 ;
        int         iWidth ;

        if ((fm != null) && (strText != null)) {
            iWidth = fm.stringWidth(strText) ;

            // Align Flush Right
            if ((iAlignment & EAST) == EAST) {
                xOffset = Math.max(rectBounds.width - iWidth, 0) ;
            }
            // Align Flush Left
            else if ((iAlignment & WEST) == WEST) {
                xOffset = 0 ;
            }
            // Center
            else {
                xOffset = Math.max((rectBounds.width - iWidth)/2, 0) ;
            }
        }

        return xOffset ;
    }


    /**
     * Calculate the desired Y Offset given the passed graphics context, string,
     * font, and alignment constraint.
     */
    public int calcYOffset(Graphics g, String strText, Font fontText, int iAlignment)
    {
        FontMetrics fm = g.getFontMetrics(fontText) ;
        Rectangle   rectBounds = getBounds() ;
        int         yOffset = 0 ;
        int         iHeight = 0 ;

        if ((fm != null) && (strText != null)) {
            iHeight = fm.getAscent() + fm.getDescent();

            // Align on bottom
            if ((iAlignment & SOUTH) == SOUTH) {
                yOffset = Math.max((rectBounds.height - iHeight) - 2, 0) ;
            }
            // Align on Top
            else if ((iAlignment & NORTH) == NORTH) {
                yOffset = 0 ;
            }
            // Align Center
            else {
                yOffset = Math.max((rectBounds.height - iHeight)/2, 0) ;
            }
        }
        return yOffset + iHeight ;
    }


    /**
     * called by framework when time to paint.
     */
    public void paint(Graphics g)
    {
        drawItem(g) ;
    }

    /**
     * get the object that we would like to display.
     */
    public Object getData(){
        return m_objData;
    }

    /**
     * get unselected text color.
     */
    public Color getTextColor(){
       return m_colorText;
    }

    /**
     * set unselected text color
     */
    public void setTextColor(Color color){
       m_colorText = color;
    }

    /**
     * Is the text selected?
     */
    public boolean isSelected(){
        return m_bSelected;
    }
//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    /**
     * prepares the graphics context for background rendering.  This includes
     * setting the background color
     */
    protected void prepareForBackgroundRendering(Graphics g)
    {
        if (m_bSelected && (m_objData != null)) {
            g.setColor(m_colorSelBackground) ;
        } else {
            g.setColor(m_colorBackground) ;
        }
    }


    /**
     * prepares the graphics contexts for foreground rendering.  This includes
     * setting font and foreground colors
     */
    protected void prepareForForegroundRendering(Graphics g)
    {
        // Set Text Color
        if (m_bSelected)
            g.setColor(m_colorSelText) ;
        else
            g.setColor(m_colorText) ;

        // Draw the text
        g.setFont(m_fontText) ;
    }
}
