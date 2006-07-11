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


package org.sipfoundry.sipxphone.sys ;

import java.awt.* ;

/**
 * SystemDefaults is a utility class that user applications and system
 * components can use to look up default fonts, colors, and system metrics.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class SystemDefaults
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /*
     * The following constants are numbered by group. Please add like entries
     * to existing groups, or create a new group with a starting index rounded
     * upwards to the nearest 100. Completely new categories are rounded
     * upwards to the nearest 10000.
     */

    /** Default text color ID. */
    public static final int COLORID_TEXT = 100 ;
    /** Default disabled text color ID. */
    public static final int COLORID_TEXT_DISABLED = 101 ;
    /** Default selected item text color ID. */
    public static final int COLORID_TEXT_SELECTED = 102 ;
        /** Default hint title color ID. */
    public static final int COLORID_TEXT_HINT_TITLE = 103 ;

    /** Default background color ID. */
    public static final int COLORID_BACKGROUND = 200 ;
    /** Default selected item background color ID. */
    public static final int COLORID_BACKGROUND_SELECTED = 201 ;
    /** Default focused item background color ID. */
    public static final int COLORID_BACKGROUND_FOCUS = 202 ;
    /** Default background color ID for tab controls. */
    public static final int COLORID_BACKGROUND_TAB = 203 ;

    /** Default 3D border dark color component ID. */
    public static final int COLORID_BORDER_DARK = 300 ;
    /** Default 3D border light color component ID. */
    public static final int COLORID_BORDER_LIGHT = 301 ;

    /** Default font. */
    public static final int FONTID_DEFAULT        = 10000 ;
    /** Default small font ID. */
    public static final int FONTID_DEFAULT_SMALL  = 10001 ;
    /** Default large font ID. */
    public static final int FONTID_DEFAULT_LARGE  = 10002 ;
    /** Default font ID used for menu entries. */
    public static final int FONTID_MENU_ENTRY     = 10003 ;
    /** Default font ID used for tab entries. */
    public static final int FONTID_TAB_ENTRY      = 10004 ;
    /** Default font ID used for titlebars. */
    public static final int FONTID_TITLEBAR       = 10005 ;
    /** Default font ID used for hints. */
    public static final int FONTID_HINT           = 10006 ;
    /** Default font ID used for a hint title. */
    public static final int FONTID_HINT_TITLE     = 10007 ;

    /** Default small font used to display a caller ID. */
    public static final int FONTID_CALLERID_SMALL   = 10008 ;
    /** Default large font used to display a caller ID. */
    public static final int FONTID_CALLERID_LARGE   = 10009 ;

    /** Default scrollbar width ID. */
    public static final int METRICID_SCROLLBAR_WIDTH = 20000 ;
    /** Default scrollbar height ID. */
    public static final int METRICID_SCROLLBAR_HEIGHT = 20001 ;


    /** Default format to show the information in the title bar.
     *  @deprecated DO NOT EXPOSE
     */
    public static final String TITLEBAR_PATTERN = "{h:mm a MM/dd/yy} \n {displayname}<{sipurl}>";

    /** Default format to show the caller ID information.
     *  @deprecated DO NOT EXPOSE
     */
    public static final String CALLERID_PATTERN = "{name} \n {sipurl}";

    /** Default locale language.
     *  @deprecated DO NOT EXPOSE
     */
    public static final String LOCALE_LANGUAGE = "en";

    /** Default locale country.
     *  @deprecated DO NOT EXPOSE
     */
    public static final String LOCALE_COUNTRY = "US";



//////////////////////////////////////////////////////////////////////////////
// Private Constants
////
    private static final Color COLOR_TEXT = new Color(0x00000000) ;
    private static final Color COLOR_TEXT_DISABLED = new Color(0x00808080) ;
    private static final Color COLOR_TEXT_SELECTED = new Color(0x00222222) ;
    private static final Color COLOR_TEXT_HINT_TITLE = new Color(0x00EEEEEE) ;
    private static final Color COLOR_BACKGROUND = new Color(0x00FFFFFF) ;
    private static final Color COLOR_BACKGROUND_SELECTED = new Color(0x00999999) ;
    private static final Color COLOR_BACKGROUND_FOCUS = new Color(0x00EEEEEE) ;
    private static final Color COLOR_BACKGROUND_TAB = new Color(0x00FFFFFF) ;
    private static final Color COLOR_BORDER_DARK = new Color(0x00777777) ;
    private static final Color COLOR_BORDER_LIGHT = new Color(0x00EEEEEE) ;

    private static final Font FONT_DEFAULT = new Font("dialog", Font.PLAIN, 10) ;
    private static final Font FONT_DEFAULT_SMALL = new Font("dialog", Font.PLAIN, 8) ;
    private static final Font FONT_DEFAULT_LARGE  = new Font("dialog", Font.PLAIN, 12) ;

    private static final Font FONT_MENU_ENTRY = new Font("dialog", Font.BOLD, 10) ;
    private static final Font FONT_TAB_ENTRY = new Font("dialog", Font.BOLD, 10) ;
    private static final Font FONT_TITLEBAR = new Font("dialog", Font.BOLD, 10) ;
    private static final Font FONT_HINT = new Font("dialog", Font.PLAIN, 10) ;
    private static final Font FONT_HINT_TITLE = new Font("dialog", Font.BOLD, 10) ;

    private static final Font FONT_CALLERID_SMALL = new Font("dialog", Font.PLAIN, 8) ;
    private static final Font FONT_CALLERID_LARGE = new Font("dialog", Font.PLAIN, 14) ;

    private static final int SCROLLBAR_WIDTH  = 12 ;
    private static final int SCROLLBAR_HEIGHT = 108 ;



///////////////////////////////////////////////////////////////////////////////
// Constructions
////
    /**
     * Private constructor to external block creation.
     */
    private SystemDefaults()
    {

    }


///////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Get one of the default colors by ID. The ID must be one of the
     * COLORID_* constants defined by this class or an IllegalArgumentException
     * is thrown.
     *
     * @param iColorID The COLORID_* constant of the color that you would like
     *        to get.
     *
     * @return The default color for the specified iColorID.
     *
     * @exception IllegalArgumentException If the passed iColorID is invalid.
     */
    public static Color getColor(int iColorID)
        throws IllegalArgumentException
    {
        Color color = null ;

        switch (iColorID) {
            case COLORID_TEXT:
                color = COLOR_TEXT ;
                break ;
            case COLORID_TEXT_DISABLED:
                color = COLOR_TEXT_DISABLED ;
                break ;
            case COLORID_TEXT_SELECTED:
                color = COLOR_TEXT_SELECTED ;
                break ;
            case COLORID_TEXT_HINT_TITLE:
                color = COLOR_TEXT_HINT_TITLE ;
                break ;
            case COLORID_BACKGROUND:
                color = COLOR_BACKGROUND ;
                break ;
            case COLORID_BACKGROUND_SELECTED:
                color = COLOR_BACKGROUND_SELECTED ;
                break ;
            case COLORID_BACKGROUND_FOCUS:
                color = COLOR_BACKGROUND_FOCUS ;
                break ;
            case COLORID_BACKGROUND_TAB:
                color = COLOR_BACKGROUND_TAB ;
                break ;
            case COLORID_BORDER_DARK:
                color = COLOR_BORDER_DARK ;
                break ;
            case COLORID_BORDER_LIGHT:
                color = COLOR_BORDER_LIGHT ;
                break ;
            default:
                throw new IllegalArgumentException("Unknown color id passed to getColor") ;
        }
        return color ;
    }


    /**
     * Get one of the default fonts by ID. The ID must be one of the
     * FONTID_* constants defined by this class or an IllegalArgumentException
     * is thrown.
     *
     * @param iFontID The FONTID_* constant of the font that you would like
     *        to get.
     *
     * @return The default font for the specified iFontID.
     *
     * @exception IllegalArgumentException If the passed iFontID is invalid.
     */
    public static Font getFont(int iFontID)
    {
        Font font = null ;

        switch (iFontID)
        {
            case FONTID_DEFAULT:
                font = FONT_DEFAULT ;
                break ;
            case FONTID_DEFAULT_SMALL:
                if (org.sipfoundry.sipxphone.sys.startup.PingerApp.isTestbedWorld())
                    font = new Font("dialog", Font.PLAIN, 9) ;
                else
                    font = FONT_DEFAULT_SMALL ;
                break ;
            case FONTID_DEFAULT_LARGE:
                font = FONT_DEFAULT_LARGE ;
                break ;
            case FONTID_MENU_ENTRY:
                font = FONT_MENU_ENTRY ;
                break ;
            case FONTID_TAB_ENTRY:
                font = FONT_TAB_ENTRY ;
                break ;
            case FONTID_TITLEBAR:
                font = FONT_TITLEBAR ;
                break ;
            case FONTID_HINT:
                font = FONT_HINT ;
                break ;
            case FONTID_HINT_TITLE:
                font = FONT_HINT_TITLE ;
                break ;
            case FONTID_CALLERID_SMALL:
                if (org.sipfoundry.sipxphone.sys.startup.PingerApp.isTestbedWorld())
                    font = new Font("dialog", Font.PLAIN, 9) ;
                else
                    font = FONT_CALLERID_SMALL ;
                break ;
            case FONTID_CALLERID_LARGE:
                font = FONT_CALLERID_LARGE ;
                break ;
            default:
                throw new IllegalArgumentException("Unknown font id passed to getfont") ;
        }
        return font ;
    }


    /**
     * Get one of the default metrics by ID. The ID must be one of the
     * METRICID_* constants defined by this class or an
     * IllegalArgumentException is thrown.
     *
     * @param iMetricID The METRICID_* constant of the font that you would
     *        like to get.
     *
     * @return The default metric for the specified iMetricID.
     *
     * @exception IllegalArgumentException If the passed iMetricID is invalid.
     */
    public static int getMetric(int iMetricID)
    {
        int iMetric ;

        switch (iMetricID) {
            case METRICID_SCROLLBAR_WIDTH:
                iMetric = SCROLLBAR_WIDTH ;
                break ;
            case METRICID_SCROLLBAR_HEIGHT:
                iMetric = SCROLLBAR_HEIGHT ;
                break ;
            default:
                throw new IllegalArgumentException("Unknown metric id passed to getMetric") ;
        }


        return iMetric ;
    }
}
