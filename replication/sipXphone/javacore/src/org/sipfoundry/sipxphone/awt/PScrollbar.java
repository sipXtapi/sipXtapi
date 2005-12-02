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

import org.sipfoundry.sipxphone.awt.event.* ;


/**
 *
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PScrollbar extends PComponent
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** size of our scrollbar bar */
    protected static final Dimension DIM_BAR_SIZE = new Dimension(7, 96) ;
    protected static final Dimension DIM_BAR_3_OPTION_SIZE = new Dimension(7,72);
    /** size of our scrollbar thumb */
    protected static final Dimension DIM_THUMB_SIZE_TOP = new Dimension(5, 2) ;
    protected static final Dimension DIM_THUMB_SIZE_MIDDLE = new Dimension(5, 1) ;
    protected static final Dimension DIM_THUMB_SIZE_BOTTOM = new Dimension(5, 2) ;
    /** minimum number of pixels of middle of thumb */
    protected static final int I_MIN_THUMB_MIDDLE_SIZE = 7 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** reference to the scroll bar background image */
    protected Image m_imageScrollbar ;
    protected Image m_imageScrollbar3Option ;
    /** references to the scroll thumb image - composed of a top, middle and bottom of the thumb*/
    protected Image m_imageScrollthumbTop ;
    protected Image m_imageScrollthumbMiddle ;
    protected Image m_imageScrollthumbBottom ;

    protected Image m_imageScrollbarTop ;
    protected Image m_imageScrollbarMiddle ;
    protected Image m_imageScrollbarBottom ;

    /** How many options are there in the list? */
    protected int   m_iNumOptions  = 0 ;
    /** What level or position are we currently at? */
    protected int   m_iPosition = -1 ;
    /** number of pixels in thumb middle */
    protected int   m_iSizeThumbMiddle = 0;
    /** number of options displayed on the screen at one time */
    protected int   m_iOptionsPerScreen = 4; //keep default to 4, otherwise 3?

    /** Is the scrollbar visible? */
    protected boolean m_bVisible ;
    /* Can only 3 options be displayed on the screen at once? */
    protected boolean m_b3OptionsPerScreen;

    /** area within scroll bar where we can draw the thumb */
    protected Rectangle m_rRectBarDrawingArea;



//////////////////////////////////////////////////////////////////////////////
// Constructor
////
    /**
     *
     */
    public PScrollbar()
    {
        // Snag our images
        m_imageScrollbar = getImage("imgScrollbarBar") ;
        m_imageScrollbar3Option = getImage("imgScrollbarBar3Option") ;

        m_imageScrollthumbTop = getImage("imgScrollbarThumbTop") ;
        m_imageScrollthumbMiddle = getImage("imgScrollbarThumbMiddle") ;
        m_imageScrollthumbBottom = getImage("imgScrollbarThumbBottom") ;

        m_imageScrollbarTop = getImage("imgScrollbarTop") ;
        m_imageScrollbarMiddle = getImage("imgScrollbarMiddle") ;
        m_imageScrollbarBottom = getImage("imgScrollbarBottom") ;

        m_bVisible = false ;
        m_b3OptionsPerScreen = false;
        m_rRectBarDrawingArea = new Rectangle(1,2, 5, 90) ;
        m_iOptionsPerScreen = 4;

        enableDoubleBuffering(true) ;
    }


    public PScrollbar(boolean bThreeOptionsPerScreen)
    {
        this();
        if (bThreeOptionsPerScreen){
            m_iOptionsPerScreen = 3;
//            m_rRectBarDrawingArea.setSize(5,68);

        }


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

        // Paint Background

        Dimension dim = getSize() ;
        m_rRectBarDrawingArea.setSize(dim);

        if (m_bVisible) {
            if (m_iPosition < 0)
                m_iPosition = 0 ;

            undrawThumb(g, m_iPosition) ;
            drawThumb(g, m_iPosition) ;
        }
    }


    public void update(Graphics g)
    {
        Component comp = getParent() ;
        if (comp != null) {
            Dimension dimParent = comp.getSize() ;
            if ((dimParent.width <= 0) || (dimParent.height <= 0))
                return ;

            Graphics parentG = comp.getGraphics() ;
            Rectangle bounds = getBounds() ;
            Image imgOffscreen = comp.createImage(dimParent.width, dimParent.height) ;
            Graphics og = imgOffscreen.getGraphics() ;
            og.setClip(bounds.x, bounds.y, bounds.width, bounds.height) ;
            comp.update(og) ;

            g.drawImage(imgOffscreen, bounds.x, bounds.y, bounds.width, bounds.height, null) ;

            og.dispose() ;
            parentG.dispose() ;
        }
    }

    protected void redrawScrollbar()
    {
        Graphics g = getGraphics() ;

        if ((m_bVisible) && (g != null)) {
            if (m_iPosition < 0)
                m_iPosition = 0 ;

            undrawThumb(g, m_iPosition) ;
            drawThumb(g, m_iPosition) ;
        }
    }




    /**
     *
     */
    protected void drawThumb(Graphics g, int iPosition)
    {
        if (m_bVisible) {

            int iRange = m_rRectBarDrawingArea.height-(m_iSizeThumbMiddle +
                    (DIM_THUMB_SIZE_TOP.height+DIM_THUMB_SIZE_BOTTOM.height)+3);

            int iCurrPosition = (int) (((float) iPosition / (float) m_iNumOptions) * (float) iRange) ;
/*
            System.out.println("------------------------------");
            System.out.println("m_rRectBarDrawingArea.height" + m_rRectBarDrawingArea.height);
            System.out.println("iPostiion" + iPosition);
            System.out.println("m_iNumOptions" + m_iNumOptions);
            System.out.println("m_iOptionsPerScreen " + m_iOptionsPerScreen);
            System.out.println("iCurrentPosition" + iCurrPosition);
            System.out.println("iRange" + iRange);
            System.out.println("m_iSizeThumbMiddle" + m_iSizeThumbMiddle);
            System.out.println("------------------------------");
*/
            //now draw the thumbwheel
            g.drawImage(m_imageScrollthumbTop, m_rRectBarDrawingArea.x, m_rRectBarDrawingArea.y+iCurrPosition, this) ;

            for (int i = 1; i <= m_iSizeThumbMiddle ; i++)
                    g.drawImage(m_imageScrollthumbMiddle, m_rRectBarDrawingArea.x, m_rRectBarDrawingArea.y+iCurrPosition+i, this) ;


            g.drawImage(m_imageScrollthumbBottom, m_rRectBarDrawingArea.x, m_rRectBarDrawingArea.y + m_iSizeThumbMiddle + DIM_THUMB_SIZE_TOP.height + iCurrPosition -1, this) ;

        }
    }


    /**
     *
     */
    public void undrawThumb(Graphics g, int iPosition)
    {
        if (m_bVisible) {
            // Just redraw the entire scrollbar...
//            g.drawImage(m_imageScrollbar, 0, 0, this) ;
            int iNumMiddleChunks = (int)((float)(m_rRectBarDrawingArea.height-m_imageScrollbarTop.getHeight(null)-
                         m_imageScrollbarBottom.getHeight(null)) /(float)m_imageScrollbarMiddle.getHeight(null));

            // Draw the background scrollbar
            g.drawImage(m_imageScrollbarTop, 0, 0, this) ;

            for (int i = 0; i < iNumMiddleChunks ; i++)
                    g.drawImage(m_imageScrollbarMiddle, 0,(i*2)+2, this) ;

            g.drawImage(m_imageScrollbarBottom, 0, (iNumMiddleChunks*2)+2, this) ;

        }
    }


    /**
     *
     */
    public void setNumOptionsPerScreen(int iNumOptions)
    {
        m_iOptionsPerScreen = iNumOptions;
    }


    /**
     *
     */
    public void setNumPosition(int iNumOptions)
    {
        /** This method currently sets the number of *additional* positions
            that the scrollbar can have (as well as first 3/4).
            We want the total number of options, so add the number on screen */

        /** set number of options */
        if (iNumOptions < 0)
            iNumOptions = 0;

        // Don't bother spinning our wheels if there is/was not change
        if (m_iNumOptions != iNumOptions) {
            m_iNumOptions = iNumOptions;

            /** reset the scroll position */
            if (m_iPosition > iNumOptions)
                m_iPosition = iNumOptions ;

            /** set size of thumb - as a proportion of number of options */
            float fThumbSize =  ((float) m_iOptionsPerScreen / (float)m_iNumOptions);

            /** set size of thumb middle */

            m_iSizeThumbMiddle = (int) (m_rRectBarDrawingArea.height * fThumbSize)
                - DIM_THUMB_SIZE_TOP.height - DIM_THUMB_SIZE_BOTTOM.height;
            if (m_iOptionsPerScreen >= m_iNumOptions)
                m_iSizeThumbMiddle = (m_rRectBarDrawingArea.height/2)-DIM_THUMB_SIZE_TOP.height - DIM_THUMB_SIZE_BOTTOM.height;

            if (m_iSizeThumbMiddle < I_MIN_THUMB_MIDDLE_SIZE)
                m_iSizeThumbMiddle = I_MIN_THUMB_MIDDLE_SIZE;

            if ( m_iNumOptions > 0 ) {
                m_bVisible = true ;
                redrawScrollbar() ;
            } else {
                m_bVisible = false ;
                repaint() ;
            }
        }
    }


    public boolean isVisible()
    {
        return m_bVisible ;
    }


    /**
     *
     */
    public void setPosition(int iPosition)
    {
        int iOldPosition = m_iPosition ;

        if (iPosition < m_iNumOptions+1)
            m_iPosition = iPosition ;

        redrawScrollbar() ;
    }
}

