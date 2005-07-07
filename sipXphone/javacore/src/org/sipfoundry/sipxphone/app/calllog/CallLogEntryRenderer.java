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


package org.sipfoundry.sipxphone.app.calllog ;

import java.awt.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.calllog.* ;
import org.sipfoundry.sipxphone.awt.* ;


/**
 * CallLogRenderer displays call log information for a particular call
 *
 * @author Robert J. Andreasen, Jr.
 */
public class CallLogEntryRenderer extends DefaultSIPAddressRenderer
{
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * renderer your data onto the passed graphics context
     */
    public void drawItemText(Graphics g)
    {
        String strSIPURL = "Unknown" ;
        int xOffset ;
        int yOffset ;

        if (m_objData != null) {
            CallLegEntry callLegData = (CallLegEntry) m_objData ;

            // Display the address as dialed, not the post-processed address.
            if (callLegData != null)
            {
                String strDialedAddress = (String) callLegData.getOtherField("dialed_address") ;
                if (strDialedAddress != null)
                {
                    strSIPURL = strDialedAddress ;
                }
                else
                {
                    strSIPURL = callLegData.getRemoteAddress().getAddress() ;
                }
            }

            CallLegEntryDescriptor data = new CallLegEntryDescriptor(callLegData);
            g.setColor(m_colorText) ;

            /** Display Name: left justified */

            String strFirstLine = super.getDisplayText(strSIPURL, 1);
            g.setFont(FONT_DEFAULT_LARGE) ;
            xOffset = calcXOffset(g, strFirstLine, FONT_DEFAULT_LARGE, WEST) ;
            yOffset = calcYOffset(g, strFirstLine, FONT_DEFAULT_LARGE, NORTH) ;

            g.drawString(strFirstLine, xOffset, yOffset) ;


            /** Display Time slightly indented. */
            if (data != null) {
                //String strTime = data.getTimeStampAsString() ;
                String strTime = data.getStartTimeAsString() ;
                xOffset = calcXOffset(g, strTime, FONT_DEFAULT_SMALL, WEST) ;
                yOffset = calcYOffset(g, strTime, FONT_DEFAULT_SMALL, SOUTH) ;

                g.setFont(FONT_DEFAULT_SMALL) ;
                if (strTime == null)
                    strTime = "Error" ;
                g.drawString(strTime, xOffset+8, yOffset) ;
            }


            /*
             * Display type of entry right justified
             */
            if (data != null) {

                //String strStatus = data.getCallStatusAsString() ;
                String strStatus = data.getStateAsShortString() ;
                xOffset = calcXOffset(g, strStatus, FONT_DEFAULT_SMALL, EAST) ;
                yOffset = calcYOffset(g, strStatus, FONT_DEFAULT_SMALL, SOUTH) ;

                g.setFont(FONT_DEFAULT_SMALL) ;
                if (strStatus == null)
                    strStatus = "Unknown" ;
                g.drawString(strStatus, xOffset, yOffset) ;
            }
        }
    }
}
