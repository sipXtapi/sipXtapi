/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/app/preferences/linemanager/LineRenderer.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.app.preferences.linemanager ;

import java.awt.* ;
import java.net.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.hook.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.sip.* ;

import org.sipfoundry.sipxphone.awt.* ;

/**
 * Displays the visual rendering for a line definition.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class LineRenderer extends PDefaultItemRenderer
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////

    /** font used to display address display name */
    protected Font fontName = SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT) ;

    /** font used to display sip address */
    protected Font fontAddress = SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT) ;


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * renderer your data onto the passed graphics context
     */
    public void drawItemText(Graphics g)
    {
        Rectangle rectBounds = new Rectangle(getSize()) ;

        SipLine line = (SipLine) super.getData() ;


        if (line != null)
        {
            // Draw Line Status
            Image imgStatus = getLineStatusIcon(line) ;
            if (imgStatus != null)
            {
                Rectangle rectImgBounds = rectBounds ;
                rectImgBounds.width = 20 ;

                int xOffset = GUIUtils.calcXImageOffset(imgStatus, rectImgBounds, GUIUtils.ALIGN_CENTER) ;
                int yOffset = GUIUtils.calcYImageOffset(imgStatus, rectImgBounds, GUIUtils.ALIGN_CENTER) ;

                g.drawImage(imgStatus, xOffset, yOffset, this) ;
            }



            // Draw Text
            String strSIPURL = line.getUserEnteredUrl() ;
            Dimension dimSize = getSize() ;
            int xOffset ;
            int yOffset ;
            int iLargeHeight = 0 ;

            g.setColor(m_colorText) ;

            /*
             * Display Name
             */
            String strDisplayName = getDisplayName(strSIPURL) ;

            g.setFont(fontName) ;

            rectBounds.x += 20;
            rectBounds.width -= 20 ;

            GUIUtils.drawTruncatedString(g, strDisplayName, rectBounds, GUIUtils.ALIGN_NORTH | GUIUtils.ALIGN_WEST) ;

            /*
             * Display Address (if different than name)
             */
            String strDisplayAddress = getDisplayAddress(strSIPURL) ;
            if (!strDisplayAddress.equals(strDisplayName))
            {
                g.setFont(fontAddress) ;

                GUIUtils.drawTruncatedString(g, strDisplayAddress, rectBounds, GUIUtils.ALIGN_SOUTH + GUIUtils.ALIGN_WEST) ;
            }
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    protected Image getLineStatusIcon(SipLine line)
    {
        Image imgRC = null ;

        switch (line.getState())
        {
            case SipLine.LINE_STATE_EXPIRED:
                imgRC = AppResourceManager.getInstance().getImage("imgErrorIcon") ;
                break ;
            case SipLine.LINE_STATE_FAILED:
                // imgRC = AppResourceManager.getInstance().getImage("imgErrorIcon") ;
                // break ;
            case SipLine.LINE_STATE_DISABLED:
            case SipLine.LINE_STATE_PROVISIONED:
            case SipLine.LINE_STATE_REGISTERED:
            case SipLine.LINE_STATE_TRYING:
            case SipLine.LINE_STATE_UNKNOWN:
                break ;
        }

        return imgRC ;
    }


    /**
     * What is the display name of the passed SIP URL?
     * The data returned here depends on the SIP URL itself.
     * gets displayName for displaying in renderer. If strSIPURL
     * contains a Display Name, that
     * string is returned, otherwise, the user id is returned.  If no user name
     * is found, the SIP URL itself is returned.
     */
    protected String getDisplayName(String strSIPURL)
    {
        String strName = getDisplayAddress(strSIPURL) ;

        SipParser parser = new SipParser(strSIPURL) ;

        // Do we have a Display Name?
        String strDisplayName = parser.getDisplayName() ;
        if ((strDisplayName != null) && (strDisplayName.length() > 0))
        {
            strName = strDisplayName ;
        }
        else
        {
            // Okay, how about a user id?
            String strUserID = parser.getUser() ;
            if ((strUserID != null) && (strUserID.length() > 0))
            {
                strName = strUserID ;
            }
        }

        return strName ;
    }


    /**
     * returns null if no displayname is provided.
     */
    protected String getUserLabel(String strSIPURL)
    {
        SipParser parser = new SipParser(strSIPURL) ;
        return  (parser.getDisplayName()) ;
    }


    /**
     * What is the display address of the SIP URL? The data returned here
     * depends on the SIP URL.  The SIP URL is returned minus any tags
     * or display names
     */
    protected  String getDisplayAddress(String strSIPURL)
    {
        String strAddress = strSIPURL ;

        SipParser parser = new SipParser(strSIPURL) ;

        try {
            SipParser builder = new SipParser() ;

            builder.setUser(parser.getUser()) ;
            builder.setHost(parser.getHost()) ;
            builder.setPort(parser.getPort()) ;

            strAddress = builder.render() ;
            strAddress.trim() ;
        } catch (Exception e) {
            strAddress = strSIPURL ;
        }
        return strAddress ;
    }
}
