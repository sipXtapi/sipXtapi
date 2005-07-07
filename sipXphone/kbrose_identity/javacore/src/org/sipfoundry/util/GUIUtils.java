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

package org.sipfoundry.util ;

import java.awt.* ;
import java.util.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.app.* ;
import org.sipfoundry.sipxphone.sys.startup.* ;

/**
 * GUIUtils is a collection of GUI utilities used by Pingtel Corporation.
 * This should not be exp1osed to outside developers as they are only quick
 * convenience methods and shouldn't stand alone.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class GUIUtils
{
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


    /**
     * Determine of the passed component is a within the passed container.
     * This is something that was left off the container implementation IMHO.
     */
    public static boolean componentInContainer(Component comp, Container cont)
    {
        Component comps[] = cont.getComponents() ;
        boolean bFound = false ;

        for (int i=0; i<comps.length; i++) {
            if (comps[i] == comp) {
                bFound = true ;
                break ;
            }
        }
        return bFound ;
    }


    /**
     * Calculate the desired X Offset given the passed graphics context, string,
     * font, and alignment constraint.
     */
    public static int calcXOffset(String strText, Graphics g, Rectangle rectBounds, int iAlignment)
    {
        FontMetrics fm = g.getFontMetrics() ;
        int         xOffset = 0 ;
        int         iWidth ;

        if ((fm != null) && (strText != null)) {
            iWidth = fm.stringWidth(strText) ;

            // Align Flush Right
            if ((iAlignment & ALIGN_EAST) == ALIGN_EAST) {
                xOffset = rectBounds.x + Math.max(rectBounds.width - iWidth, 0) ;
            }
            // Align Flush Left
            else if ((iAlignment & ALIGN_WEST) == ALIGN_WEST) {
                xOffset = rectBounds.x ;
            }
            // Center
            else {
                xOffset = rectBounds.x + Math.max((rectBounds.width - iWidth)/2, 0) ;
            }
        }

        return xOffset ;
    }


    /**
     * Calculate the desired Y Offset given the passed graphics context, string,
     * font, and alignment constraint.
     */
    public static int calcYOffset(String strText, Graphics g, Rectangle rectBounds, int iAlignment)
    {
        FontMetrics fm = g.getFontMetrics() ;
        int         yOffset = 0 ;
        int         iHeight = 0 ;

        if ((fm != null) && (strText != null)) {
            iHeight = fm.getAscent() ;

            // Align on bottom
            if ((iAlignment & ALIGN_SOUTH) == ALIGN_SOUTH) {
                yOffset = rectBounds.y + Math.max((rectBounds.height - (iHeight+fm.getDescent())) , 0) ;
            }
            // Align on Top
            else if ((iAlignment & ALIGN_NORTH) == ALIGN_NORTH) {
                yOffset = rectBounds.y;
            }
            // Align Center
            else {
                yOffset = rectBounds.y + Math.max((rectBounds.height - iHeight)/2, 0) ;
            }
        }
        return yOffset + iHeight ;
    }


    /**
     * Calculate the desired X offet given the specified image, bounding
     * rectangle, alignment.
     */
    public static int calcXImageOffset(Image image, Rectangle rectBounds, int iAlignment)
    {
        int         xOffset = 0 ;
        int         iWidth ;

        if (image != null) {
            iWidth = image.getWidth(null) ;

            // Align Flush Right
            if ((iAlignment & ALIGN_EAST) == ALIGN_EAST) {
                xOffset = rectBounds.x + Math.max(rectBounds.width - iWidth, 0) ;
            }
            // Align Flush Left
            else if ((iAlignment & ALIGN_WEST) == ALIGN_WEST) {
                xOffset = rectBounds.x ;
            }
            // Center
            else {
                xOffset = rectBounds.x + Math.max((rectBounds.width - iWidth)/2, 0) ;
            }
        }

        return xOffset ;
    }


    /**
     * Calculate the desired Y offet given the specified image, bounding
     * rectangle, alignment.
     */
    public static int calcYImageOffset(Image image, Rectangle rectBounds, int iAlignment)
    {
        int         yOffset = 0 ;
        int         iHeight = 0 ;

        if (image != null) {
            iHeight = image.getHeight(null) ;

            // Align on bottom
            if ((iAlignment & ALIGN_SOUTH) == ALIGN_SOUTH) {
                yOffset = rectBounds.y + Math.max((rectBounds.height - iHeight) - 2, 0) ;
            }
            // Align on Top
            else if ((iAlignment & ALIGN_NORTH) == ALIGN_NORTH) {
                yOffset = rectBounds.y ;
            }
            // Align Center
            else {
                yOffset = rectBounds.y + Math.max((rectBounds.height - iHeight)/2, 0) ;
            }
        }
        return yOffset ;
    }


    /**
     * Draws a 3d frame around the designated rectangle.
     *
     * @param g The graphics context that will be drawn upon.
     * @param rect The rectangle that marks the bounds of the 3d frame
     * @param colorBackground The background color that will be painted within
     *        the frame.
     */
    public static void draw3DFrame(Graphics g, Rectangle rect, Color colorBackground)
    {
       Color colorOld = g.getColor() ;

        // Fill Background Area
        g.setColor(colorBackground) ;
        g.fillRect(rect.x, rect.y, rect.width, rect.height) ;

        // Draw borders
        g.setColor(SystemDefaults.getColor(SystemDefaults.COLORID_BORDER_DARK)) ;

        //    __
        //   x__|
        //
        g.drawLine( rect.x,
                    rect.y,
                    rect.x,
                    rect.y + (rect.height-1)) ;

        //    xx
        //   |__|
        //
        g.drawLine( rect.x,
                    rect.y,
                    rect.x + (rect.width-1),
                    rect.y) ;

        g.setColor(SystemDefaults.getColor(SystemDefaults.COLORID_BORDER_LIGHT)) ;

        //    __
        //   |  |
        //    xx
        g.drawLine( rect.x + 1,
                    rect.y + (rect.height-1),
                    rect.x + (rect.width-1),
                    rect.y + (rect.height-1)) ;

        //    __
        //   |__x
        //
        g.drawLine( rect.x + (rect.width-1),
                    rect.y + 1,
                    rect.x + (rect.width-1),
                    rect.y + (rect.height-1)) ;

        g.setColor(colorOld) ;
    }


    /**
     * Draws a 3d hilite frame around the designated rectangle.
     *
     * @param g The graphics context that will be drawn upon.
     * @param rect The rectangle that marks the bounds of the 3d frame
     * @param colorBackground The background color that will be painted within
     *        the frame.
     */
    public static void draw3dHiliteFrame(Graphics g, Rectangle rect)
    {
        Color colorOld = g.getColor() ;

        // Draw borders
        g.setColor(SystemDefaults.getColor(SystemDefaults.COLORID_BORDER_DARK)) ;

        /*
         * First draw a dark rectangle
         */
        g.drawRect(rect.x, rect.y, rect.width-2, rect.height-2) ;


        /*
         * Fill in lite areas
         */
        g.setColor(Color.white) ;

        //    __
        //   x__|
        //
        g.drawLine( rect.x+1,
                    rect.y+1,
                    rect.x+1,
                    rect.y+(rect.height-3)) ;

        //    xx
        //   |__|
        //
        g.drawLine( rect.x+1,
                    rect.y+1,
                    rect.x+(rect.width-3),
                    rect.y+1) ;

        //    __
        //   |  |
        //    xx
        g.drawLine( rect.x + 1,
                    rect.y + (rect.height-1),
                    rect.x + (rect.width-1),
                    rect.y + (rect.height-1)) ;

        //    __
        //   |__x
        //
        g.drawLine( rect.x + (rect.width-1),
                    rect.y,
                    rect.x + (rect.width-1),
                    rect.y + (rect.height-1)) ;

        g.setColor(colorOld) ;
    }



    public static void drawTruncatedString(Graphics g, String strSource,
            Rectangle rectBounds, int iAlignment)
    {
        int         xOffset = 0 ;
        int         yOffset = 0 ;

        int         iWidth = 0 ;
        int         iHeight = 0 ;

        Vector      vWrappedText = null ;
        FontMetrics fm = null ;

        /*
         * Initialize our h/vWrappedText
         */
        fm = g.getFontMetrics(g.getFont()) ;
        if (fm != null) {
            if (strSource != null) {
                vWrappedText = TextUtils.expandNewlines(strSource) ;
                iHeight = (vWrappedText.size() * fm.getAscent()) + fm.getDescent() ;
            } else {
                vWrappedText = null ;
                iWidth = 0 ;
                iHeight = fm.getAscent() ;
            }
        }


        /*
         * Figure out placements given Y/Vertical Axis
         */

        // Along along Y/Vertical Axis
        if ((iAlignment & ALIGN_SOUTH) == ALIGN_SOUTH) {
            yOffset = Math.max(rectBounds.height - iHeight, 2) + rectBounds.y ;
        }
        else if ((iAlignment & ALIGN_NORTH) == ALIGN_NORTH) {
            yOffset = rectBounds.y  ;
        }
        else {
            yOffset = Math.max((rectBounds.height - iHeight)/2, 0) + rectBounds.y ;
        }


        /*
         * Drawing Time
         */
        if ((vWrappedText != null) && (fm != null)) {

            yOffset += fm.getAscent() ;

            // Draw each line
            for (int i=0; i<vWrappedText.size(); i++) {
                String strLine = (String) vWrappedText.elementAt(i) ;
                iWidth = fm.stringWidth(strLine) ;

                if (iWidth > rectBounds.width) {
                    strLine = TextUtils.truncateToWidth(strLine, fm,
                            rectBounds.width) ;
                    iWidth = fm.stringWidth(strLine) ;
                }

                // Align along X/Horizonal Axis
                if ((iAlignment & ALIGN_EAST) == ALIGN_EAST) {
                    xOffset = Math.max(rectBounds.width - iWidth, 0) + rectBounds.x ;
                }
                else if ((iAlignment & ALIGN_WEST) == ALIGN_WEST) {
                    xOffset = rectBounds.x ;
                }
                else {
                    xOffset = Math.max((rectBounds.width - iWidth)/2, 0) + rectBounds.x ;
                }

                // Draw It!
                g.drawString(strLine, xOffset, yOffset + (i * fm.getAscent())) ;
            }
        }
    }


    /**
     * Displays the specified string into the specified bounds given a
     * alignment and the maximum number of lines.
     *
     * @param g The graphics context used to draw the string.  Don't forget to
     *        set the font before invoking this method.
     * @param string The string that will be displayed.
     * @param rectBounds The bound rectangle where the text should be wrapped
     *        and displayed.
     * @param iAlignment The alignment directives for the text.
     */
    public static void drawWrappingString(Graphics g, String string,
                                          Rectangle rectBounds, int iAlignment)
    {
        drawWrappingString( g, string, rectBounds, iAlignment, -1);
    }



    /**
     * Displays the specified string into the specified bounds given a
     * alignment and the maximum number of lines.
     *
     * @param g The graphics context used to draw the string.  Don't forget to
     *        set the font before invoking this method.
     * @param string The string that will be displayed.
     * @param rectBounds The bound rectangle where the text should be wrapped
     *        and displayed.
     * @param iAlignment The alignment directives for the text.
     * @param iDesiredNumberOfLines Wraps the wrapped text which is wrapped
     *        only until the desired number of lines. If the actual number of
     *        lines exceeds the desired number of lines, ... will be appended
     *        to the last desired line. A value of -1 means the actual number
     *        of lines is the desired number of lines.
     */
     public static void drawWrappingString(Graphics g,
                                           String string,
                                           Rectangle rectBounds,
                                           int iAlignment,
                                           int iDesiredNumberOfLines)
    {
        int         xOffset = 0 ;
        int         yOffset = 0 ;

        int         iWidth = 0 ;
        int         iHeight = 0 ;

        Vector      vWrappedText = null ;
        FontMetrics fm = null ;

        /*
         * Initialize our h/vWrappedText
         */
        fm = g.getFontMetrics(g.getFont()) ;
        if (fm != null) {
            if (string != null) {
                vWrappedText = TextUtils.wrapText(string, fm, rectBounds.width, " \t-\\/:,.", iDesiredNumberOfLines) ;
                iHeight = (vWrappedText.size() * fm.getAscent()) + fm.getDescent() ;
            } else {
                vWrappedText = null ;
                iWidth = 0 ;
                iHeight = fm.getAscent() ;
            }
        }


        /*
         * Figure out placements given Y/Vertical Axis
         */

        // Along along Y/Vertical Axis
        if ((iAlignment & ALIGN_SOUTH) == ALIGN_SOUTH) {
            yOffset = Math.max(rectBounds.height - iHeight, 2) + rectBounds.y ;
        }
        else if ((iAlignment & ALIGN_NORTH) == ALIGN_NORTH) {
            yOffset = rectBounds.y  ;
        }
        else {
            yOffset = Math.max((rectBounds.height - iHeight)/2, 0) + rectBounds.y ;
        }


        /*
         * Drawing Time
         */
        if ((vWrappedText != null) && (fm != null)) {

            yOffset += fm.getAscent() ;

            // Draw each line
            for (int i=0; i<vWrappedText.size(); i++) {
                String strLine = (String) vWrappedText.elementAt(i) ;
                iWidth = fm.stringWidth(strLine) ;

                // Align along X/Horizonal Axis
                if ((iAlignment & ALIGN_EAST) == ALIGN_EAST) {
                    xOffset = Math.max(rectBounds.width - iWidth, 0) + rectBounds.x ;
                }
                else if ((iAlignment & ALIGN_WEST) == ALIGN_WEST) {
                    xOffset = rectBounds.x ;
                }
                else {
                    xOffset = Math.max((rectBounds.width - iWidth)/2, 0) + rectBounds.x ;
                }

                // Draw It!
                g.drawString(strLine, xOffset, yOffset + (i * fm.getAscent())) ;
            }
        }
    }
}
