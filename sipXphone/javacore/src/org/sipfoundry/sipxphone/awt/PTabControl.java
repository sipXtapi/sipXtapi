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
import java.util.* ;

import org.sipfoundry.sipxphone.awt.event.* ;

import org.sipfoundry.sipxphone.service. * ;
import org.sipfoundry.sipxphone.sys. * ;

import org.sipfoundry.util.* ;


/**
 * This is a basic tab control that will display 3 tabs.  You can
 * specify label text or a label image.  Please see the constants
 * DIM_LABEL_DRAWING_AREA for label dimensions.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PTabControl extends PContainer
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** size of our tab control */
    protected static final Dimension DIM_TAB_CONTROL_SIZE   = new Dimension(160, 24) ;
    /** amount of screen area allocated to the tab label */
    public static final    Dimension DIM_LABEL_DRAWING_AREA = new Dimension(43, 16) ;
    /** drawing origin of left most tab */
    public static final    Point     POINT_LABEL_LEFT_ORIGIN = new Point(9,5) ;
    /** drawing origin of middle tab */
    public static final    Point     POINT_LABEL_MIDDLE_ORIGIN = new Point(58,5) ;
    /** drawing origin of right most tab */
    public static final    Point     POINT_LABEL_RIGHT_ORIGIN = new Point(111,5) ;

    /** the left most tab */
    public static final int LEFT     = 0 ;
    /** the middle tab */
    public static final int MIDDLE   = 1 ;
    /** the right most tab */
    public static final int RIGHT    = 2 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** different rendered views of our tabs.  */
    protected Image  m_imgTabRenderings[] ;
    /** labels for each tab */
    protected PLabel m_labels[] ;
    /** which tab is selected?  LEFT, MIDDLE, RIGHT */
    protected int   m_iSelectedTab ;

    /** list of PTabListeners awaiting tab notifications */
    protected Vector m_vListeners = new Vector() ;


//////////////////////////////////////////////////////////////////////////////
// Constructors
////
    public PTabControl()
    {
        super() ;

        setOpaque(false) ;

        AppResourceManager resMgr = AppResourceManager.getInstance() ;

        m_iSelectedTab = -1 ;
        m_imgTabRenderings = new Image[3] ;
        m_labels = new PLabel[3] ;
        m_imgTabRenderings[LEFT] = resMgr.getImage("imgTabFrameLeftSelected") ;
        m_imgTabRenderings[MIDDLE] = resMgr.getImage("imgTabFrameMiddleSelected") ;
        m_imgTabRenderings[RIGHT] = resMgr.getImage("imgTabFrameRightSelected") ;
    }

//////////////////////////////////////////////////////////////////////////////
// Accessor Methods
////

    /**
     * reset the background image for the specified position
     *
     * @image the position (LEFT, MIDDLE, RIGHT)
     * @image the background image
     */
    public void setActiveTabImage(int position, Image image)
    {
        // Validate the position parameter
        if ((position == LEFT) || (position == MIDDLE) || (position == RIGHT))
        {
            m_imgTabRenderings[position] = image ;
        }
        else
        {
            throw new IllegalArgumentException() ;
        }
    }


    /**
     * set the label of the specified tab
     *
     * @param objLabel our label object
     * @param iTab which tab? LEFT, MIDDLE, or RIGHT?
     */
    public void setLabel(PLabel label, int iTab)
    {
        // Remove label is it already exists
        if (m_labels[iTab] != null) {
            remove(m_labels[iTab]) ;
        }

        // set label as requested
        m_labels[iTab] = label ;

        // add new label
        if (label != null) {
            add(label) ;

            switch (iTab) {
                case LEFT:
                    label.setBounds(POINT_LABEL_LEFT_ORIGIN.x, POINT_LABEL_LEFT_ORIGIN.y, DIM_LABEL_DRAWING_AREA.width, DIM_LABEL_DRAWING_AREA.height) ;
                    break ;
                case MIDDLE:
                    label.setBounds(POINT_LABEL_MIDDLE_ORIGIN.x, POINT_LABEL_MIDDLE_ORIGIN.y, DIM_LABEL_DRAWING_AREA.width, DIM_LABEL_DRAWING_AREA.height) ;
                    break ;
                case RIGHT:
                    label.setBounds(POINT_LABEL_RIGHT_ORIGIN.x, POINT_LABEL_RIGHT_ORIGIN.y, DIM_LABEL_DRAWING_AREA.width, DIM_LABEL_DRAWING_AREA.height) ;
                    break ;
            }
        }
    }


    /**
     * set the tab selection
     *
     * @param iTab which tab? LEFT, MIDDLE, or RIGHT?
     */
    public void setTab(int iTab)
    {
        if ( (iTab == LEFT) || (iTab == MIDDLE) || (iTab == RIGHT) )
        {
            // Only change if needed AND if we have a valid rendering
            if ((iTab != m_iSelectedTab) && (m_imgTabRenderings[iTab] != null))
            {
                m_iSelectedTab = iTab ;
                repaint() ;
                fireTabChanged(iTab) ;
            }
        }
        else
        {
            throw new IllegalStateException("Invalid tab setting") ;
        }
    }


    /**
     * get the tab selection
     *
     * @return tab selection (LEFT, MIDDLE, or RIGHT)
     */
    public int getTab()
    {
        return m_iSelectedTab ;
    }


    /**
     *
     */
    public void addTabListener(PTabListener listener)
    {
        m_vListeners.addElement(listener) ;
    }


    /**
     *
     */
    public void removeTabListener(PTabListener listener)
    {
        m_vListeners.removeElement(listener) ;
    }




//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Paint the component
     */
    public void paint(Graphics g)
    {
        /** Draw selected background image */
        switch (m_iSelectedTab)
        {
            case LEFT:
            case MIDDLE:
            case RIGHT:
                if (m_imgTabRenderings[m_iSelectedTab] != null)
                {
                    g.drawImage(m_imgTabRenderings[m_iSelectedTab],
                            0, 0,
                            SystemDefaults.getColor(SystemDefaults.COLORID_BACKGROUND_TAB),
                            this) ;
                }
                break ;
            default:
                throw new IllegalStateException("Illegal tab selection: " + Integer.toString(m_iSelectedTab)) ;
        }

        super.paint(g) ;
    }

    /**
     *
     */
    protected void fireTabChanged(int iNewTab)
    {
        PTabListener listener ;
        PTabEvent    changeEvent = new PTabEvent(this, iNewTab) ;

        Enumeration traverse = m_vListeners.elements() ;
        while (traverse.hasMoreElements()) {
            listener = (PTabListener) traverse.nextElement() ;
            if (listener != null) {
                try {
                    listener.tabChanged(changeEvent) ;
                } catch (Throwable t) {
                    Shell.getInstance().showUnhandledException(t, true) ;
                }
            }
        }
    }
}
