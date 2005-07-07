/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/awt/ComponentBorder.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.awt;

import java.awt.* ;
import org.sipfoundry.util.GUIUtils ;

import org.sipfoundry.sipxphone.sys.SystemDefaults ;


/**
 * This is a container that displays a 3d border around an specified component.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class ComponentBorder extends PContainer
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    Insets     m_insetsOriginal ;   // Any specified insets
    Insets     m_insetsAdjusted ;   // Adjust insets (for placing the componet)
    String     m_strBorderLabel ;   // Border label placed in upper left corner
    int        m_iTopInset ;        // Additional top inset adjustment
    Component  m_component ;        // The component to wrap


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructs a ComponentBorder given a border label and component
     *
     * @param strBorderLabel The border label to display in the upper left
     *         corner.
     * @param component The component to wrap with a border.
     */
    public ComponentBorder(String strBorderLabel, Component component)
    {
        super() ;

        setInsets(new Insets(0, 0, 0, 0)) ;

        setLayout(null) ;
        setBorderLabel(strBorderLabel) ;
        m_component = component ;
        add(m_component) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Accessor Methods
////

    /**
     * Sets the insets for this label
     */
    public void setInsets(Insets insets)
    {
        m_insetsOriginal = insets ;

        m_insetsAdjusted = new Insets( insets.top+m_iTopInset,
                insets.left+2,
                insets.bottom+2,
                insets.right+2) ;

        super.setInsets(m_insetsAdjusted) ;
    }


    /**
     * Resets the border label.
     *
     * @param strBorderLabel The new border label
     */
    public void setBorderLabel(String strLabel)
    {
        m_strBorderLabel = strLabel ;
        if (m_strBorderLabel != null)
        {
            m_iTopInset = 12 ;
        }
        else
        {
            m_iTopInset = 2 ;
        }

        setInsets(m_insetsOriginal) ;
    }


    /**
     * Sets the bounds for this component
     */
    public void setBounds(Rectangle rect)
    {
        setBounds(rect.x, rect.y, rect.width, rect.height) ;
    }


    /**
     * Sets the bounds for this component
     */
    public void setBounds(int x, int y, int width, int height)
    {
        super.setBounds(x, y, width, height);

        System.out.println("setBounds: " + x + ", " + y + ", " + width + ", " + height) ;

        if (m_component != null)
        {
            m_component.setBounds(  m_insetsAdjusted.left,
                                    m_insetsAdjusted.top,
                                    width - (m_insetsAdjusted.right + m_insetsAdjusted.left),
                                    height - (m_insetsAdjusted.top + m_insetsAdjusted.bottom)) ;
        }
    }

    /**
     * This paint method allows the parent class to draw before it draws the 3d
     * border
     *
     * @depreciated do not expose
     */
    public void paint(Graphics g)
    {
        // Let parent class draw whatever it needs/wants
        super.paint(g) ;

        // Set the initial font and color
        Font fontOld = g.getFont() ;
        Font fontSmall = SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL) ;
        g.setFont(fontSmall) ;
        g.setColor(Color.black);

        // Figure out where to draw
        Rectangle rectBounds = getBounds() ;
        if (m_strBorderLabel == null)
        {
            rectBounds.x = m_insetsOriginal.left ;
            rectBounds.y = m_insetsOriginal.top ;
            rectBounds.width -= (m_insetsOriginal.right + m_insetsOriginal.left) ;
            rectBounds.height -= (m_insetsOriginal.top + m_insetsOriginal.bottom) ;
        }
        else
        {
            int iCenter = m_iTopInset / 2 ;
            int iRemainder = m_iTopInset - iCenter ;

            rectBounds.x = m_insetsOriginal.left ;
            rectBounds.y = m_insetsOriginal.top + iCenter;
            rectBounds.width -= (m_insetsOriginal.right + m_insetsOriginal.left) ;
            rectBounds.height -= (m_insetsOriginal.top + m_insetsOriginal.bottom + iRemainder) ;
        }

        // Only continue if we have a valid drawing surface
        if ((rectBounds.width > 0) && (rectBounds.height > 0))
        {
            // Draw Border
            GUIUtils.draw3dHiliteFrame(g, rectBounds) ;

            // Draw Label
            if (m_strBorderLabel != null)
            {
                Rectangle rectText = new Rectangle(
                        rectBounds.x + 8,
                        rectBounds.y - (m_iTopInset / 2),
                        rectBounds.width - 16,
                        m_iTopInset) ;

                FontMetrics fm = g.getFontMetrics() ;
                int iWidth = fm.stringWidth(m_strBorderLabel) ;
                if (iWidth < rectText.width)
                {
                    rectText.width = iWidth ;
                }

                g.clearRect(rectText.x, rectText.y, rectText.width, rectText.height) ;
                Rectangle rectClip = g.getClipBounds() ;

                GUIUtils.drawTruncatedString(g, m_strBorderLabel, rectText, GUIUtils.ALIGN_WEST) ;
            }
        }

        // Reset old font
        if (fontOld != null)
            g.setFont(fontOld) ;

    }
}
