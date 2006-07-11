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

import org.sipfoundry.util.* ;

import org.sipfoundry.sipxphone.sys.startup.* ;

import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.sys.* ;

/**
 * A PLabel is an extension of a basic AWT label.  It allows developers to
 * display either text or an image, and apply some basic alignment constraints.
 */
public class PLabel extends PComponent implements Cloneable
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////

    /** Align the component to the east along the horizonal axis. */
    public static final int   ALIGN_EAST      = 0x0010 ;
    /** Align the component to the west along the horizonal axis. */
    public static final int   ALIGN_WEST      = 0x0001 ;
    /** Align the component to the north along the vertical axis. */
    public static final int   ALIGN_NORTH     = 0x1000 ;
    /** Align the component to the south along the vertical axis. */
    public static final int   ALIGN_SOUTH     = 0x0100 ;
    /** Align the component to the center of both the horizonal and vertical
        axes. This is the default if a horizontal or vertical alignment is
        not specified. */
    public static final int   ALIGN_CENTER    = 0;

    /** Wrap Setting: Do not wrap the text. Instead, truncate the text abruptly
        at the boundaries of the label. */
    public static final int WRAP_NONE           = 1 ;
    /** Wrap Setting: Wrap the text at word breaks (and/or punctuation,
        dashes, etc.). */
    public static final int WRAP_WORD           = 2 ;
    /** Wrap Setting: Do not wrap the text. Instead, truncate the text
        and add ellipsis points ("...") where the text is truncated. */
    public static final int WRAP_ELLIPSIS_POINTS = 4 ;

    /** Default Label Font */
    private static final Font DEFAULT_FONT = SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT) ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** is this an image?  true if so, otherwise false (meaning string) */
    private boolean m_bIsImage ;
    /** image reference */
    private Image m_image ;
    /** string reference */
    private String m_string ;

    /** how should we align our text/image within our given 'space' */
    private int m_iAlignment ;

    /** how should we wrap our text withint our given 'space' */
    private int m_iWrapStyle ;

    /** offsets from edges (insets) */
    private Insets  m_insets = new Insets(0,0,0,0) ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////

    /**
     * Default Constructor.
     */
    public PLabel()
    {
        m_bIsImage = false ;
        m_iWrapStyle = WRAP_WORD ;
        m_image = null ;
        m_string = null ;

        setFont(DEFAULT_FONT) ;
        setOpaque(false) ;
        setForeground(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;
    }


    /**
     * Constructor, takes a source image.
     *
     * @param image The image that should display in the label.
     * @exception IllegalArgumentException Thrown if image is null.
     */
    public PLabel(Image image) throws IllegalArgumentException
    {
        if( image == null ){
             throw new IllegalArgumentException("Image passed cannot be null.") ;
        }
        setImage(image) ;
        m_iWrapStyle = WRAP_WORD ;

        setFont(DEFAULT_FONT) ;
        setForeground(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;
    }


    /**
     * Constructor, takes a source image and an alignment constraint.
     *
     * @param image The image that should display in the label.
     * @param iAlignment How the image should be aligned.  See ALIGN_EAST, ALIGN_WEST,
     *        ALIGN_CENTER, ALIGN_NORTH, and ALIGN_SOUTH constants above.
     * @exception IllegalArgumentException Thrown if image is null.
     */
    public PLabel(Image image, int iAlignment) throws IllegalArgumentException
    {
        if( image == null ){
             throw new IllegalArgumentException("Image passed cannot be null.") ;
        }
        m_iWrapStyle = WRAP_WORD ;
        setImage(image) ;
        setFont(DEFAULT_FONT) ;
        setAlignment(iAlignment) ;
        setForeground(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;
    }


    /**
     * Constructor, takes a source string.
     *
     * @param string Text string that should display in the label.
     */
    public PLabel(String string)
    {
        m_iWrapStyle = WRAP_WORD ;
        setText(string) ;
        setFont(DEFAULT_FONT) ;
        setForeground(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;
    }


    /**
     * Constructor, takes a source string and alignment constraints.
     *
     * @param string Text string that should display in the label.
     * @param iAlignment How the image should be aligned. See ALIGN_EAST, ALIGN_WEST,
     *        ALIGN_CENTER, ALIGN_NORTH, and ALIGN_SOUTH constants above.
     */
    public PLabel(String string, int iAlignment)
    {
        m_iWrapStyle = WRAP_WORD ;
        setText(string) ;
        setFont(DEFAULT_FONT) ;
        setAlignment(iAlignment) ;
        setForeground(SystemDefaults.getColor(SystemDefaults.COLORID_TEXT)) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Set the label image. Any text previously set will be cleared; however,
     * alignment constraints are carried forward.
     *
     * @param image The image that should display in the label
     */
    public void setImage(Image image)
    {
        m_image = image ;
        m_string = null ;
        m_bIsImage = true ;
    }


    /**
     * Set the background image of the label. This is independent of
     * setImage and setText.
     */
    public void setBackgroundImage(Image image)
    {
        super.setBackgroundImage(image) ;
    }


    /**
     * Set the insets associated with this control.
     *
     * @param insets The new insets.
     */
    public void setInsets(Insets insets)
    {
        m_insets = insets ;
    }


    /**
     * Set the text that should display in the label.
     *
     * @param string The text string that should display in the label.
     */
    public void setText(String string)
    {
        m_image = null ;
        m_string = string ;
        m_bIsImage = false ;

        if (isVisible())
            repaint() ;
    }

    /**
     * gets the text of this Label if it not an image Label.
     * @return null if its an image, text of the label otherwise.
     *
     * @deprecated DO NOT EXPOSE
     */
    public String getText()
    {
        if (m_image != null)
        {
            return null;
        }
        else
        {
            return m_string;
        }
    }


    /**
     * Get the image for this Label.
     *
     * @return the image specified as part of the constructor or the setImage
     *         accessor or null if no image has been specified.
     *
     * @deprecated DO NOT EXPOSE
     */
    public Image getImage()
    {
        return m_image ;
    }


    /**
     * Set the alignment for this label.
     *
     * @param iAlignment How the image should be aligned. See ALIGN_EAST, ALIGN_WEST,
     *        ALIGN_CENTER, ALIGN_NORTH, and ALIGN_SOUTH constants above.
     */
    public void setAlignment(int iAlignment)
    {
        if (!isValidAlignment(iAlignment))
            throw new IllegalArgumentException("illegal Argument Passed to PLabel.setAlignment");
        m_iAlignment = iAlignment ;
    }


    /**
     * Set the wrapping style for this label.
     *
     * @param iWrapStyle This parameter defines whether or not the label text should
     *        be wrapped. Options include WRAP_NONE, WRAP_WORD, and
     *        WRAP_ELLIPSIS_POINTS, described above.
     */
    public void setWrapping(int iWrapStyle)
    {
        if (!isValidWrapping(iWrapStyle))
            throw new IllegalArgumentException("illegal Argument Passed to PLabel. setWrapping");

        if (m_iWrapStyle != iWrapStyle)
        {
            m_iWrapStyle = iWrapStyle ;

            doLayout() ;
            repaint() ;
        }
    }


    /**
     * @deprecated Hide from users
     */

    public String toString()
    {
        String strRC = null ;

        if (m_bIsImage) {
            if (m_image != null)
                strRC  = m_image.toString() ;
        }
        else {
            if (m_string != null)
                strRC = m_string ;
        }

        return strRC ;
    }


    /**
     * @deprecated Hide
     */
    public void paint(Graphics g)
    {
        super.paint(g) ;

        Rectangle   rectBounds = getBounds() ;
        int         xOffset = 0 ;
        int         yOffset = 0 ;

        int         iWidth = 0 ;
        int         iHeight = 0 ;


        // adjust rectBounds for insets
        rectBounds.x += m_insets.left ;
        rectBounds.y += m_insets.top ;
        rectBounds.width -= ((m_insets.right + m_insets.left)) ;
        rectBounds.height -= ((m_insets.top + m_insets.bottom)) ;

        Vector      vWrappedText = null ;
        FontMetrics fm = null ;

        g.setColor(getForeground()) ;

        /*
         * Initialize our h/vWrappedText
         */
        if ((isImage()) && (m_image != null)) {
            iWidth = m_image.getWidth(this) ;
            iHeight = m_image.getHeight(this) ;
        } else {
            fm = g.getFontMetrics(getFont()) ;
            if (fm != null) {
                if (m_string != null) {
                    if ((m_iWrapStyle & WRAP_WORD) == WRAP_WORD) {
                        vWrappedText = TextUtils.wrapText(m_string, fm, rectBounds.width, " \t-\\/:,.") ;
                    } else {
                        vWrappedText = TextUtils.expandNewlines(m_string) ;
                    }
                    iHeight = (vWrappedText.size() * fm.getAscent()) + fm.getDescent() ;
                    if (vWrappedText.size() > 0)
                        iHeight += (vWrappedText.size()-1) ;
                } else {
                    vWrappedText = null ;
                    iWidth = 0 ;
                    iHeight = fm.getAscent() ;
                }
            }
        }


        /*
         * Figure out placements given Y/Vertical Axis
         */

        // Along along Y/Vertical Axis
        if ((m_iAlignment & ALIGN_SOUTH) == ALIGN_SOUTH) {
            yOffset = Math.max(rectBounds.height - iHeight, 2) ;
        }
        else if ((m_iAlignment & ALIGN_NORTH) == ALIGN_NORTH) {
            yOffset = 0 ;
        }
        else {
            yOffset = Math.max((rectBounds.height - iHeight)/2, 0) ;
        }


        /*
         * Drawing Time
         */
        if ((isImage()) && (m_image != null)) {
            // Align along X/Horizonal Axis
            if ((m_iAlignment & ALIGN_EAST) == ALIGN_EAST) {
                xOffset = Math.max(rectBounds.width - iWidth, 0) ;
            }
            else if ((m_iAlignment & ALIGN_WEST) == ALIGN_WEST) {
                xOffset = 0 ;
            }
            else {
                xOffset = Math.max((rectBounds.width - iWidth)/2, 0) ;
            }

            g.drawImage(m_image, xOffset, yOffset, this) ;
        }
        else if ((vWrappedText != null) && (fm != null)) {

            yOffset += fm.getAscent() ;

            // Draw each line
            for (int i=0; i<vWrappedText.size(); i++) {
                String strLine = (String) vWrappedText.elementAt(i) ;
                iWidth = fm.stringWidth(strLine) ;

                if (iWidth > rectBounds.width) {
                    if ((m_iWrapStyle & WRAP_ELLIPSIS_POINTS) ==
                            WRAP_ELLIPSIS_POINTS)  {
                        strLine = TextUtils.truncateToWidth(strLine, fm,
                                rectBounds.width) ;
                        iWidth = fm.stringWidth(strLine) ;
                    }
                }

                // Align along X/Horizonal Axis
                if ((m_iAlignment & ALIGN_EAST) == ALIGN_EAST) {
                    xOffset = Math.max(rectBounds.width - iWidth, 0) ;
                }
                else if ((m_iAlignment & ALIGN_WEST) == ALIGN_WEST) {
                    xOffset = 2 ;
                }
                else {
                    xOffset = Math.max((rectBounds.width - iWidth)/2, 0) ;
                }

                // Draw It!
                g.drawString(strLine, m_insets.left + xOffset, m_insets.top + yOffset + (i * (fm.getAscent()+1))) ;
            }
        }
    }


    /**
     * @deprecated do not expose
     */
    public PLabel makeClone()
    {
        PLabel label = null ;

        try {
            label = (PLabel) clone() ;
        } catch (CloneNotSupportedException cnse) { }

        return label ;
    }


    /**
     * @deprecated do not expose
     */
    public Dimension getPreferredSize()
    {
        Dimension dimSize = new Dimension(0, 0) ;

        if (isImage()) {
            if (m_image != null) {
                dimSize = new Dimension(m_image.getWidth(this), m_image.getHeight(this)) ;
            }
        } else {
            if (m_string != null) {
                Graphics g = getGraphics() ;
                if (g != null) {
                    FontMetrics fm = g.getFontMetrics(getFont()) ;
                    if (fm != null)
                        dimSize = new Dimension(fm.stringWidth(m_string), fm.getAscent() + fm.getDescent()) ;
                }
            }
        }
        return dimSize ;
    }


    /**
     * @deprecated do not expose
     */
    public Dimension getMinimumSize()
    {
        return getPreferredSize() ;
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    private boolean isValidAlignment(int i)
    {
        boolean bValid = true;
        int validMask = ALIGN_CENTER | ALIGN_SOUTH | ALIGN_NORTH |
                        ALIGN_WEST | ALIGN_EAST ;

        //reverse the bits on valid mask
        validMask ^= 0xFFFFFFFF;

        //check that it is valid
        // by anding the xor'ed bitmask against the incoming value
        if ((i & validMask) > 0)
            bValid = false;

        return bValid;
    }


    private boolean isValidWrapping(int i)
    {
        boolean bValid = true;
        int validMask = WRAP_NONE | WRAP_ELLIPSIS_POINTS | WRAP_WORD ;

        //reverse the bits on valid mask
        validMask ^= 0xFFFFFFFF;

        //check that it is valid
        // by anding the xor'ed bitmask against the incoming value
        if ((i & validMask) > 0)
            bValid = false;

        return bValid;
    }


    /**
     *
     */
    private boolean isImage()
    {
        return m_bIsImage ;
    }


    /**
     *
     */
    private boolean isString()
    {
        return !m_bIsImage ;
    }

}
