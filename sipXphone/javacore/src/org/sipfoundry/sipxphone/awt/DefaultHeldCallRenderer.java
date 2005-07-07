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
import java.net.* ;

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.hook.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.stapi.* ;

/**
 * Default renderer for a single held call
 *
 * @author Robert J. Andreasen, Jr.
 */
public class DefaultHeldCallRenderer extends DefaultSIPAddressRenderer
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private String m_strLastDuration = null ;


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * undraws the text by re-drawing the text using the background color.
     *
     * <br> NOTE: This only undraws the duration.
     */
    public void undrawItemText(Graphics g)
    {
        int xOffset ;
        int yOffset ;

        if ( super.isSelected() && ( super.getData() != null)) {
            g.setColor(super.getSelBkColor()) ;
        } else {
            g.setColor(super.getBkColor()) ;
        }

        /*
         * Duration
         */
        if (m_strLastDuration != null) {
            g.setFont(FONT_DEFAULT_SMALL) ;

            xOffset = calcXOffset(g, m_strLastDuration, FONT_DEFAULT_SMALL, EAST) ;
            yOffset = calcYOffset(g, m_strLastDuration, FONT_DEFAULT_SMALL, SOUTH) ;
            g.drawString(m_strLastDuration, xOffset, yOffset) ;
        }
    }


    /**
     * renderer your data onto the passed graphics context
     */
    public void drawItemText(Graphics g)
    {
        Dimension   dim = getSize() ;
        int xOffset ;
        int yOffset ;
        Object objData = super.getData();
        if ( objData != null) {
            PCall call = (PCall) objData ;
            String strFirstLine = "";

            PAddress participants[] = call.getParticipants() ;
            if (participants.length > 1) {
                strFirstLine = participants.length + " Party Conference" ;
            } else if (participants.length == 1) {
                strFirstLine = super.getDisplayText(participants[0].getAddress(), 1);
            }

            g.setFont(FONT_DEFAULT_LARGE) ;
            xOffset = calcXOffset(g, strFirstLine, FONT_DEFAULT_LARGE, WEST) ;
            yOffset = calcYOffset(g, strFirstLine, FONT_DEFAULT_LARGE, NORTH) ;
            g.drawString(strFirstLine, xOffset, yOffset) ;

            ///
            //setting "To:" address
            String strToAddress = "To: "+SipParser.stripParameters(call.getCalledAddress());
            xOffset = calcXOffset(g, strToAddress, FONT_DEFAULT_SMALL, WEST) ;
            yOffset = calcYOffset(g, strToAddress, FONT_DEFAULT_SMALL, SOUTH) ;

            g.setFont(FONT_DEFAULT_SMALL) ;
            if (strToAddress == null)
                strToAddress = "" ;

            m_strLastDuration =
                DateUtils.getDurationAsString(call.getHoldDuration()*1000) ;

            String strToAddressThatFits = TextUtils.truncateToWidth
            ( strToAddress, g.getFontMetrics(),
                getSize().width - g.getFontMetrics().stringWidth(m_strLastDuration+5), "...");

            g.drawString(strToAddressThatFits, xOffset, yOffset) ;
            /*
             * Duration
             */
            if (m_strLastDuration != null) {
                g.setFont(FONT_DEFAULT_SMALL) ;

                xOffset = calcXOffset(g, m_strLastDuration, FONT_DEFAULT_SMALL, EAST) ;
                yOffset = calcYOffset(g, m_strLastDuration, FONT_DEFAULT_SMALL, SOUTH) ;
                g.drawString(m_strLastDuration, xOffset, yOffset) ;
            }

        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////

}
