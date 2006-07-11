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


package org.sipfoundry.sipxphone.app.conference ;

import java.awt.* ;
import java.net.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.hook.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.stapi.* ;

/**
 * Renderer used to display a single conference participant.  This
 * renderer is blind and must be told when to update.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class ConferenceStatusRenderer extends DefaultSIPAddressRenderer
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * renderer your data onto the passed graphics context
     */
    public void drawItemText(Graphics g)
    {

        Dimension dimSize = getSize() ;
        FontMetrics fm = g.getFontMetrics();
        Rectangle rectClient = new Rectangle(0, 0, dimSize.width, dimSize.height) ;
        Object objData = super.getData();
        if ( objData != null) {
            ConferenceCallDataItem data = (ConferenceCallDataItem) objData ;
            int iState = data.getState() ;

            String strSIPURL = (String) data.getAddress().toString() ;
            String strFirstLine = super.getDisplayText(strSIPURL, 1);
            String strConnectionState = toStateString(iState) ;

            int xOffset ;
            int yOffset ;

            /** First Line of display */
            g.setColor(super.getTextColor()) ;
            g.setFont(FONT_DEFAULT_LARGE) ;
            xOffset = GUIUtils.calcXOffset(strFirstLine, g, rectClient, GUIUtils.ALIGN_WEST) ;
            yOffset = GUIUtils.calcYOffset(strFirstLine, g, rectClient, GUIUtils.ALIGN_NORTH) ;
            String strFirstLineToFit = TextUtils.truncateToWidth
                ( strFirstLine, fm, dimSize.width-fm.stringWidth(strConnectionState) );
            g.drawString(strFirstLineToFit, xOffset, yOffset) ;

            /** Connection State */
            g.setFont(FONT_DEFAULT_LARGE) ;
            xOffset = GUIUtils.calcXOffset(strConnectionState, g, rectClient, GUIUtils.ALIGN_EAST) ;
            yOffset = GUIUtils.calcYOffset(strConnectionState, g, rectClient, GUIUtils.ALIGN_NORTH) ;
            g.drawString(strConnectionState, xOffset, yOffset) ;

            /*
             * if second line of text is different than the first line of text.
             */
            String strSecondLine = super.getDisplayText(strSIPURL, 2);
            if (!strSecondLine.equals(strFirstLine)) {
                g.setFont(FONT_DEFAULT_LARGE) ;
                xOffset = GUIUtils.calcXOffset(strSecondLine, g, rectClient, GUIUtils.ALIGN_WEST) ;
                yOffset = GUIUtils.calcYOffset(strSecondLine, g, rectClient, GUIUtils.ALIGN_SOUTH) ;

                g.drawString(strSecondLine, xOffset, yOffset) ;
            }
        }
    }


    public String toStateString(int iState)
    {
        String strRC = "Unknown" ;

        switch (iState)
        {
            case ConferenceCallDataItem.STATE_UNAVAILABLE:
                strRC = "Unavailable" ;
                break ;
            case ConferenceCallDataItem.STATE_HELD:
                strRC = "On Hold" ;
                break ;
            case ConferenceCallDataItem.STATE_TRYING:
                strRC = "Trying" ;
                break ;
            case ConferenceCallDataItem.STATE_RINGING:
                strRC = "Ringing" ;
                break ;
            case ConferenceCallDataItem.STATE_FAILED:
                strRC = "Failed" ;
                break ;
            case ConferenceCallDataItem.STATE_CONNECTED:
                strRC = "Active" ;
                break ;
            case ConferenceCallDataItem.STATE_UNKNOWN:
            default:
                // strRC pre-initialized to strRC
                break ;
        }
        return strRC ;
    }
}
