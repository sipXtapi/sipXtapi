/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/app/shell/PHintPopup.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sipxphone.sys.app.shell ;

import java.awt.* ;
import java.util.* ;

import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.sys.startup.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.sys.* ;

/**
 * The HintPopup is a window that is displayed in the bottom half of
 * the screen and includes both a centered blurp of hint text along with
 * a left justified title.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PHintPopup extends PComponent implements Cloneable
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** Default Label Font */
    private static final Font FONT_HINT = SystemDefaults.getFont(SystemDefaults.FONTID_HINT) ;
    private static final Font FONT_TITLE = SystemDefaults.getFont(SystemDefaults.FONTID_HINT_TITLE) ;

    private static final int WIDTH = 160 ;
    private static final int HEIGHT = 78 ;
    private static final int LEFT_INDENT = 7 ;
    private static final int TITLE_RIGHT_INDENT = 8 ;
    private static final int HINT_RIGHT_INDENT = 12 ;
    private static final int TITLE_INDENT = 3 ;
    private static final int TITLE_HEIGHT = 15 ;
    private static final int HINT_INDENT = 21 ;
    private static final int HINT_HEIGHT = 45 ;

    private static final Rectangle RECT_TITLE = new Rectangle(LEFT_INDENT, TITLE_INDENT, WIDTH-TITLE_RIGHT_INDENT, TITLE_HEIGHT) ;
    private static final Rectangle RECT_HINT = new Rectangle(LEFT_INDENT, HINT_INDENT, WIDTH-HINT_RIGHT_INDENT, HINT_HEIGHT) ;



//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** image reference */
    private Image m_imageBackground ;
    /** The hint text */
    private String m_strHint ;
    /** The title text */
    private String m_strTitle ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * Default Constructor
     */
    public PHintPopup()
    {
        super() ;

        setOpaque(false) ;
        setForeground(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;
        setBackgroundImage(getImage("imgHintFrame")) ;
        m_strHint = null ;
        m_strTitle = null ;
    }


    /**
     * Constructor taking the initial hint text
     */
    public PHintPopup(String strHint)
    {
        this() ;
        m_strHint = strHint ;
    }


    /**
     * Constructor taking the initial hint and title text
     */
    public PHintPopup(String strHint, String strTitle)
    {
        this() ;

        m_strHint = strHint ;
        m_strTitle = strTitle ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Set the hint text that should display in this component.
     *
     * @param string The text string that should display in the component.
     */
    public void setHint(String strHint)
    {
        m_strHint = strHint ;

        if (isVisible())
            repaint() ;
    }

    /**
     * gets the hint text of this component.
     * @deprecated DO NOT EXPOSE
     */
    public String getHint(){
        return m_strHint;
    }

    /**
     * Set the hint title that should display in this component.
     *
     * @param string The title string that should display in the component.
     */
    public void setTitle(String strTitle)
    {
        m_strTitle = strTitle ;

        if (isVisible())
            repaint() ;
    }

    /**
     * gets the title of this component.
     * @deprecated DO NOT EXPOSE
     */
    public String getTitle(){
        return m_strTitle;
    }

    /**
     * @deprecated Hide
     */
    public void paint(Graphics g)
    {
        super.paint(g) ;

        // Display Title
        if (m_strTitle != null) {
            g.setFont(FONT_TITLE) ;
            g.setColor(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT_HINT_TITLE)) ;
            GUIUtils.drawWrappingString(g, m_strTitle, RECT_TITLE, GUIUtils.ALIGN_WEST) ;
        }

        // Display Hint
        if (m_strHint != null) {
            g.setFont(FONT_HINT) ;
            g.setColor(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;
            GUIUtils.drawWrappingString(g, m_strHint, RECT_HINT, GUIUtils.ALIGN_CENTER) ;
        }
    }


    /**
     * @deprecated do not expose
     */
    public Dimension getPreferredSize()
    {
        return new Dimension(WIDTH, HEIGHT) ;
    }


    /**
     * @deprecated do not expose
     */
    public Dimension getMinimumSize()
    {
        return getPreferredSize() ;
    }


    /**
     * @deprecated do not expose
     */
    public Dimension getMaximumSize()
    {
        return getPreferredSize() ;
    }
//////////////////////////////////////////////////////////////////////////////
// Implementation
////

}