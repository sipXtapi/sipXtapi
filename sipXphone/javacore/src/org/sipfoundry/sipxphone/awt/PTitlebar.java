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

import java.text.* ;
import java.awt.* ;
import java.util.* ;

import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.util.* ;


/**
 * Titlebar component
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PTitlebar extends PComponent
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** Dialog frame style: bold titlebar */
    public static final int TAB_FRAME = 0 ;
    /** Dialog frame style: bolder/darker titlebar */
    public static final int MESSAGEBOX_FRAME = 1 ;
    /* gray flat style */
    public static final int APP_FRAME = 2;

    private static Rectangle RECT_ICON = new Rectangle(3, 2, 3+18+2, 21) ;
    private static Rectangle RECT_TEXT = new Rectangle(3+18+4, 3, 152, 21) ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** Titlebar title */
    protected String m_strTitle = null ;
    /** Title bar image; should be 16x16 */
    protected Image  m_imgIcon  = null ;

    protected Color  m_textColor = null ;

    /** What style is this titlebar? */
    public    int    m_iStyle ;

    /** background image as suggested by iStyle */
    protected Image  m_imgBackground ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     *
     */
    public PTitlebar()
    {
        setStyle(TAB_FRAME) ;
        initializeTitlebar() ;
    }


    public PTitlebar(int iStyle)
    {
        setStyle(iStyle) ;
        initializeTitlebar() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * set the titlebar title
     *
     * @param strTitle
     */
    public void setTitle(String strTitle)
    {
        m_strTitle = strTitle ;
        repaint(RECT_TEXT.x, RECT_TEXT.y, RECT_TEXT.width, RECT_TEXT.height) ;
    }


    /**
     * set the title to be the current time and date
     */
    public void setTimeDate()
    {
        Date time = Calendar.getInstance().getTime() ;

        DateFormat dfTime = DateFormat.getTimeInstance(DateFormat.SHORT) ;
        DateFormat dfDate = DateFormat.getDateInstance(DateFormat.SHORT) ;

        setTitle(dfTime.format(time) + " " + dfDate.format(time)) ;
    }



    /**
     * get the titlebar title
     *
     * @return String
     */
    public String getTitle()
    {
        return m_strTitle ;
    }


    /**
     * set the title bar icon
     *
     * @param imgIcon
     */
    public void setIcon(Image imgIcon)
    {
        m_imgIcon = imgIcon ;
        repaint(RECT_ICON.x, RECT_ICON.y, RECT_ICON.width, RECT_ICON.height) ;
    }


    /**
     * get the title bar icon
     *
     * @return Image
     */
    public Image getIcon()
    {
        return m_imgIcon ;
    }


    /**
     * set the title bar style
     *
     * @param iStyle
     */
    public void setStyle(int iStyle)
    {
        m_iStyle = iStyle ;
        initializeTitlebar() ;
    }


    /**
     * get the title bar style
     *
     * @param int
     */
    public int getStyle()
    {
        return m_iStyle ;
    }



//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    public void forcePaint()
    {
        repaint() ;
    }


    /**
     *
     */
    public void paint(Graphics g)
    {
        // Store clip region
        Rectangle rectClip = g.getClipBounds() ;

        // Place background
        g.drawImage(m_imgBackground, 0, 0, this) ;

        // Draw Icon
        if (m_imgIcon != null)
        {
            g.setClip(RECT_ICON) ;
            int xOffset = GUIUtils.calcXImageOffset(m_imgIcon, RECT_ICON, 0) ;
            int yOffset = GUIUtils.calcYImageOffset(m_imgIcon, RECT_ICON, 0) ;

            g.drawImage(m_imgIcon, xOffset, yOffset, this) ;
        }

        // Draw Text
        if (m_strTitle != null)
        {
            g.setClip(RECT_TEXT) ;
            g.setColor(m_textColor) ;
            g.setFont(SystemDefaults.getFont(SystemDefaults.FONTID_TITLEBAR)) ;
            g.drawString(m_strTitle, 5+19+2, 5+12) ;
        }

        // Restore original clip region.
        g.setClip(rectClip) ;
    }


    /**
     * load the appropriate background image and text color given the current style
     */
    protected void initializeTitlebar()
    {
        AppResourceManager resMgr = AppResourceManager.getInstance() ;

        switch (m_iStyle) {
            case TAB_FRAME:
                m_textColor = SystemDefaults.getColor(SystemDefaults.COLORID_TEXT) ;
                m_imgBackground = resMgr.getImage("imgTabFrameTitlebar") ;
                break ;
            case MESSAGEBOX_FRAME:
                m_textColor = SystemDefaults.getColor(SystemDefaults.COLORID_BORDER_LIGHT) ;
                m_imgBackground = resMgr.getImage("imgDialogFrameTitlebar") ;
                break ;
            case APP_FRAME:
                m_textColor = SystemDefaults.getColor(SystemDefaults.COLORID_TEXT) ;
                m_imgBackground = resMgr.getImage("imgAppFrameTitlebar") ;
                break ;
        }
    }
}
