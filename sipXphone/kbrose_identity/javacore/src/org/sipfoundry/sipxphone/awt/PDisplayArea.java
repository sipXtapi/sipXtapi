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
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.sipxphone.sys.startup.PingerApp;
import java.awt.* ;
import java.util.* ;


/**
 * The PDisplayArea is much like a TextArea, except that it is read-only.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PDisplayArea extends PContainer
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** Align the component to the east along the horizonal axis. */
    public static final int   ALIGN_EAST      = 0x0010 ;
    /** Align the component to the west along the horizonal axis. */
    public static final int   ALIGN_WEST      = 0x0001 ;
    /** Align the component to the center of both the horizonal and vertical
        axes.  This is the default if a horizontal or vertical alignment is
        not specified. */
    public static final int   ALIGN_CENTER    = 0x0000 ;
    /** Align the component to the north along the vertical axis. */
    public static final int   ALIGN_NORTH     = 0x1000 ;
    /** Align the component to the south along the vertical axis. */
    public static final int   ALIGN_SOUTH     = 0x0100 ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////

    /** Unprocessed text as specified by the user */
    private String m_strText = null ;
    /** Lines of text after \n expansion and wrapping */
    private Vector m_vLines = null ;
    /** The font we are using to display our data */
    private Font m_fontDisplay  ;
    /** reference to our scrollbar */
    private PScrollbar m_scrollbar  ;
    /** internal panel responsible for drawing our text */
    private icDrawPanel m_drawPanel ;

    /** number of lines per page */
    private int m_iLinesPerPage = -1 ;
    /** number of pixels available per line */
    private int m_iLineWidth = -1 ;
    /** height of each line of text */
    private int m_iLineHeight = -1 ;

    /** top index of our scrolling region */
    private int m_iTopIndex  = 0 ;

    /** Is automatic text wrapping enabled? */
    private boolean m_bWrappingEnabled ;

    /** How should we align our text? */
    private int m_iTextAlignment ;

    /** The String consisting of delimiters which are used
     *  as breakpoints while wrapping. Currently it is
     *  " \t-\\/:,." i.e. space, tab, hyphen, backslash, forwardslash, colon,
     *  comma and a period.
     */
    private String m_strWrappingDelimiters = " \t-\\/:,.";

//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * Constructs a blank PDisplayArea control.
     */
    public PDisplayArea()
    {
        constructionHelper() ;
    }


    /**
     * Constructs a PDisplayArea control initialized with passed text.
     *
     * @param strText initialize text that will be displayed in the
     *        PDisplayArea
     */
    public PDisplayArea(String strText)
    {
        constructionHelper() ;

        setText(strText );
    }


    /**
     * Initialization common across all constructors
     */
    private void constructionHelper()
    {
        setLayout(null) ;

        m_bWrappingEnabled = true ;
        m_scrollbar = new PScrollbar() ;
        m_drawPanel = new icDrawPanel() ;
        m_fontDisplay = SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT) ;
        m_vLines = null ;
        m_iTextAlignment = ALIGN_WEST | ALIGN_NORTH ;

        add(m_drawPanel) ;
        addButtonListener(new icButtonListener()) ;
        setOpaque(false) ;
    }



//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Benign overwrite. Capture explicitly set font to use for font
     * scrolling calc.
     */
    public void setFont(Font f)
    {
        m_fontDisplay = f;
        super.setFont(f);
    }


    /**
     * Set the text that will be displayed in this display area.
     *
     * @param strText text that will be displayed in the PDisplayArea
     */
    public void setText(String strText)
    {
        m_iTopIndex = 0 ;

        m_strText = strText ;

        if (isVisible()) {
            doLayout() ;
        }
        repaint() ;
    }


    /**
     * Returns the text being displayed in the PDisplayArea.  This text was
     * either passed in the constructor or set by invoking setText().
     *
     * @return Text being displayed in the display area
     */
    public String getText()
    {
        return m_strText ;
    }


    /**
     * Enable or disable automatic text wrapping.  The PDisplayArea
     * component can automatically wrap text on white space (space, tab, and
     * a dash).  By default this is enabled.
     *
     * @param bEnable true to enable text wrapping or false to disable
     *
     */
    public void enableWrapping(boolean bEnable)
    {
        if (m_bWrappingEnabled != bEnable)  {
            m_bWrappingEnabled = bEnable ;

            if (isVisible()) {
                doLayout() ;
            }
        }
    }


    /**
     * Returns if automatic text wrapping is enabled or not.
     *
     * @return true if text wrapping is enabled otherwise false
     */
    public boolean isWrappingEnabled()
    {
        return m_bWrappingEnabled ;
    }

    /**
     * Sets strWrappingDelimiters as the wrapping delimiters for this
     * display area. The default delimiters used are  " \t-\\/:,."
     * i.e. space, tab, hyphen, backslash, forwardslash, colon, comma and
     * a period. Call this  method if you like to set the wrapping delimiters
     * to something other than the default ones.
     *
     * @param strWrappingDelimiters String consisting of delimiters used as
     * breaking points while wrapping.
     *
     * @deprecated Do not expose yet.
     */
    public void setWrappingDelimiters(String strWrappingDelimiters ){
        if( strWrappingDelimiters == null )
          throw new java.lang.IllegalArgumentException
                      ("wrapping delimiters cannot be null.");
        m_strWrappingDelimiters = strWrappingDelimiters;
    }



    /**
     * Prompts the layout manager to lay out this component. This is usually
     * called when the component (more specifically, container) is validated.
     */
    public void doLayout()
    {
        Dimension   dimSize = getSize() ;
        Graphics    g = getGraphics() ;
        FontMetrics fm = null;
        boolean     bNeedScrollbar = false ;

        // Figure out line width/height/linesperpage
        if (g != null) {
            fm = g.getFontMetrics(m_fontDisplay) ;
            if (fm != null) {
                m_iLineHeight = fm.getAscent() ;
            }
        }

        /*
         * This is a bit tricky.  We need to figure out whether or not we need
         * a scrollbar.  Now, this can be a processor intensive and we could
         * add some tricks.  For now, the implementation is slightly
         * inefficient and will renderer assuming a scrollbar a needed.  If
         * the number of lines after rendering is less than a screen, then we
         * re-render with no scrollbars.  The theory is re-rendering a short
         * blob of text is 'better' than re-rendering a long one.  Another
         * approach might include a method that starts rendering assuming no
         * scrollbar and then short-circuits if it renderer more than a
         * screen.  Methinks, wasting time on short/easy solution is better
         * then the longer/hard solution.
         */

        if (fm != null)
        {
            m_iLinesPerPage = ((dimSize.height - (fm.getDescent())) / m_iLineHeight) ;

            m_iLineWidth = dimSize.width - SystemDefaults.getMetric(SystemDefaults.METRICID_SCROLLBAR_WIDTH) ;
            m_vLines = processText(fm, m_strText, m_iLineWidth) ;

            if (m_vLines.size() <= m_iLinesPerPage) {
                m_iLineWidth = dimSize.width ;
                m_vLines = processText(fm, m_strText, m_iLineWidth) ;
            } else
                bNeedScrollbar = true ;
        }

        // Show / Hide the scrollbar as needed
        if (bNeedScrollbar) {
            if (!GUIUtils.componentInContainer(m_scrollbar, this)) {
                add(m_scrollbar) ;
            }
            // m_scrollbar.setPosition(m_iTopIndex) ;
            m_scrollbar.setNumPosition(m_vLines.size() - m_iLinesPerPage) ;
            m_scrollbar.setBounds(0, 0, SystemDefaults.getMetric(
                    SystemDefaults.METRICID_SCROLLBAR_WIDTH),
                    dimSize.height-8) ; //DWW -8 is the width of the bitmap border
//            System.out.println("PDisplayArea sets Scrollbar height as " + dimSize.height);
            m_drawPanel.setBounds(SystemDefaults.getMetric(SystemDefaults.METRICID_SCROLLBAR_WIDTH), 0, dimSize.width - SystemDefaults.getMetric(SystemDefaults.METRICID_SCROLLBAR_WIDTH), dimSize.height) ;
        } else {
            remove(m_scrollbar) ;
            m_drawPanel.setBounds(0, 0, dimSize.width, dimSize.height) ;
        }
        super.doLayout() ;
    }


    /**
     * Called when the user process the scroll up button
     */
    public void scrollUp(int iByLines)
    {
        if (m_iTopIndex > 0) {

            m_iTopIndex -= iByLines ;
            if (m_iTopIndex < 0)
                m_iTopIndex = 0 ;

            if (m_scrollbar != null) {
                m_scrollbar.setPosition(m_iTopIndex) ;
            }
            m_drawPanel.repaint() ;
        }
    }


    /**
     * Called when the user process the scroll down button
     */
    public void scrollDown(int iByLines)
    {
        if (m_vLines != null) {
            int iLowestTop = m_vLines.size() - m_iLinesPerPage ;

            if (m_iTopIndex < iLowestTop) {

                m_iTopIndex += iByLines ;
                if (m_iTopIndex > iLowestTop)
                    m_iTopIndex = iLowestTop ;

                if (m_scrollbar != null) {
                    m_scrollbar.setPosition(m_iTopIndex) ;
                }

                m_drawPanel.repaint() ;
            }
        }
    }


    /**
     * Set the alignment of the text within this display area.  Developers
     * can align the text to the west, east, or center it.
     *
     * @param iAlignment alignment code: ALIGN_EAST, ALIGN_WEST, or
     *        ALIGN_CENTER
     */
    public void setTextAlignment(int iAlignment)
    {
        m_iTextAlignment = iAlignment ;
    }



//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * Helper method that expands and wraps the specified text and returns
     * the resultant vector of strings.
     *
     * NOTE: This examines the global m_bWrappingEnabled attribute before
     *       attempting to wrap text.
     */
    private Vector processText(FontMetrics fm, String strText, int iWidth)
    {
        Vector      vResults = null ;

        if (fm != null) {
            vResults = new Vector() ;
            if (strText != null) {
                // Expand new lines
                Vector vExpanded = TextUtils.expandNewlines(strText) ;

                if (m_bWrappingEnabled) {
                    // Wrap each expanded line
                    for (int i=0; i<vExpanded.size(); i++) {
                        Vector vWrappedLine = TextUtils.wrapText
                           ((String) vExpanded.elementAt(i), fm, iWidth, m_strWrappingDelimiters) ;
                        for (int j=0; j<vWrappedLine.size(); j++) {
                            vResults.addElement(vWrappedLine.elementAt(j)) ;
                        }
                    }
                } else {
                    vResults = vExpanded ;
                }
            }
        }
        return vResults ;
    }


//////////////////////////////////////////////////////////////////////////////
// Inner and Nested Classes
////
    /**
     * Panel that actually displays / renderers our display panel data
     */
    private class icDrawPanel extends PComponent
    {
        public icDrawPanel()
        {
            super() ;
            this.enableDoubleBuffering(true) ;
        }


        public void paint(Graphics g)
        {
            super.paint(g) ;

            m_scrollbar.setPosition(m_iTopIndex) ;
            if (m_vLines != null) {
                int iNumLines = m_vLines.size() ;
                int iWidth = this.getSize().width ;
                int iHeight = this.getSize().height ;

                g.setFont(m_fontDisplay) ;
                FontMetrics fm = g.getFontMetrics() ;

                int yOffset = 0 ;
                if (iNumLines < m_iLinesPerPage) {
                    int iTextHeight = (iNumLines * fm.getAscent()) + fm.getDescent() ;

                    // Along along Y/Vertical Axis
                    if ((m_iTextAlignment & ALIGN_SOUTH) == ALIGN_SOUTH) {
                        yOffset = Math.max(iHeight - iTextHeight, 2) ;
                    }
                    else if ((m_iTextAlignment & ALIGN_NORTH) == ALIGN_NORTH) {
                        yOffset = 0 ;
                    }
                    else {
                        yOffset = Math.max((iHeight - iTextHeight)/2, 0) ;
                    }
                }


                for (int i=m_iTopIndex; i<(m_iTopIndex+m_iLinesPerPage); i++) {
                    if (i < iNumLines) {
                        String strLine = (String) m_vLines.elementAt(i) ;
                        g.setColor(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;

                        int iTextWidth = fm.stringWidth(strLine) ;
                        int xOffset = 0 ;

                        if ((m_iTextAlignment & ALIGN_EAST) == ALIGN_EAST)
                            xOffset = Math.max(iWidth - iTextWidth, 0) ;
                        else if ((m_iTextAlignment & ALIGN_WEST) == ALIGN_WEST)
                            xOffset = 0 ;
                        else
                            xOffset = Math.max((iWidth - iTextWidth)/2, 0) ;

                        g.drawString(strLine, xOffset, yOffset + m_iLineHeight*(1+(i-m_iTopIndex))) ;
                    }
                }
            }
        }
    }


    /**
     * Button Listener listening for buttons presses / scrolling
     */
    private class icButtonListener implements PButtonListener
    {
        public void buttonUp(PButtonEvent event)
        {
            switch (event.getButtonID()) {
                case PButtonEvent.BID_SCROLL_UP:
                    scrollUp(2) ;
                    break ;
                case PButtonEvent.BID_SCROLL_DOWN:
                    scrollDown(2) ;
                    break ;
            }
        }

        public void buttonDown(PButtonEvent event) { }

        public void buttonRepeat(PButtonEvent event) { }
    }
}
