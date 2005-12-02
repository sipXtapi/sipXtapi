/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/awt/BorderedLabel.java#2 $
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
import org.sipfoundry.util.GUIUtils ;
import org.sipfoundry.sipxphone.sys.SystemDefaults ;


/**
 * An extension to a PLabel, where a 3D border is displayed around the label
 * and some label text is displayed in the upper left hand corner.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class BorderedLabel extends PLabel
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    Insets m_insetsOriginal = null ;    // Insets passed to us
    String m_strBorderLabel = null ;    // Label displayed in upper left corner
    int    m_iTopInset = 0 ;            // additional top inset


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructs a bordered label with the specified border label text (this
     * is the text that will be displayed in the upper left hand corner).
     *
     * @param strBorderLabel The new border label
     */
    public BorderedLabel(String strLabel, String strBorderLabel)
    {
        super(strLabel) ;
        setInsets(new Insets(0, 0, 0, 0)) ;

        setBorderLabel(strBorderLabel) ;
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

        Insets adjInsets = new Insets(
                insets.top+m_iTopInset,
                insets.left+2,
                insets.bottom+2,
                insets.right+2) ;

        super.setInsets(adjInsets) ;
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
            rectBounds.x += m_insetsOriginal.left ;
            rectBounds.y += m_insetsOriginal.top ;
            rectBounds.width -= (m_insetsOriginal.right + m_insetsOriginal.left) ;
            rectBounds.height -= (m_insetsOriginal.top + m_insetsOriginal.bottom) ;
        }
        else
        {
            int iCenter = m_iTopInset / 2 ;
            int iRemainder = m_iTopInset - iCenter ;

            rectBounds.x += m_insetsOriginal.left ;
            rectBounds.y += m_insetsOriginal.top + iCenter;
            rectBounds.width -= (m_insetsOriginal.right + m_insetsOriginal.left) ;
            rectBounds.height -= (m_insetsOriginal.top + m_insetsOriginal.bottom + iRemainder) ;

            System.out.println("RECTANGLE bounds: " + rectBounds) ;
        }


        // Only continue if we have a valid drawing surface
        if ((rectBounds.width > 0) && (rectBounds.height > 0))
        {
            // Draw border
            GUIUtils.draw3dHiliteFrame(g, rectBounds) ;

            // Add Label
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
