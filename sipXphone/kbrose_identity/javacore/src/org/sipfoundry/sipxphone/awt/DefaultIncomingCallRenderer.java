/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/awt/DefaultIncomingCallRenderer.java#2 $
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
 * Default renderer for a single incoming ringing call
 *
 */
public class DefaultIncomingCallRenderer extends DefaultSIPAddressRenderer
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** font used to display address display name */
    protected Font fontName = SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_LARGE) ;
    /** font used to display sip address */
    protected Font fontAddress = SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_SMALL) ;

    private String m_strLastDuration = null ;
    private String m_strDiplayName = null ;


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
            g.setFont(fontAddress) ;

            xOffset = calcXOffset(g, m_strLastDuration, fontAddress, EAST) ;
            yOffset = calcYOffset(g, m_strLastDuration, fontAddress, SOUTH) ;
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

            /*
             * Display Name
             */
            if (m_strDiplayName == null) {
                m_strDiplayName = "Unknown" ;
                PAddress participants[] = call.getParticipants() ;
                if (participants.length > 1) {
                    m_strDiplayName = participants.length + " Party Conference" ;
                } else if (participants.length == 1) {
                    m_strDiplayName = getDisplayName(participants[0].getAddress()) ;
                }
            }

            g.setFont(fontName) ;
            xOffset = calcXOffset(g, m_strDiplayName, fontName, WEST) ;
            yOffset = calcYOffset(g, m_strDiplayName, fontName, NORTH) ;
            g.drawString(m_strDiplayName, xOffset, yOffset) ;


           //setting "To:" address
            String strToAddress = "To: "+SipParser.stripParameters(call.getCalledAddress());
            xOffset = calcXOffset(g, strToAddress, fontAddress, WEST) ;
            yOffset = calcYOffset(g, strToAddress, fontAddress, SOUTH) ;

            g.setFont(fontAddress) ;
            if (strToAddress == null)
                strToAddress = "" ;

            m_strLastDuration =
                DateUtils.getDurationAsString(call.getDurationSinceFirstRang()*1000) ;

            String strToAddressThatFits = TextUtils.truncateToWidth
            ( strToAddress, g.getFontMetrics(),
                getSize().width - g.getFontMetrics().stringWidth(m_strLastDuration+5), "...");

            g.drawString(strToAddressThatFits, xOffset, yOffset) ;
            /*
             * Duration
             */
            if (m_strLastDuration != null) {
                g.setFont(fontAddress) ;

                xOffset = calcXOffset(g, m_strLastDuration, fontAddress, EAST) ;
                yOffset = calcYOffset(g, m_strLastDuration, fontAddress, SOUTH) ;
                g.drawString(m_strLastDuration, xOffset, yOffset) ;
            }
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////



    /**
     * What is the display name of the passed SIP URL?  The data returned here
     * depends on the SIP URL itself.  If it contains a Display Name, that
     * string is returned, otherwise, the user id is return.  If no user name
     * is found, the SIP URL itself is returned.
     */
    protected String getDisplayName(String strSIPURL)
    {
        String strName = getDisplayAddress(strSIPURL) ;

        SipParser parser = new SipParser(strSIPURL) ;

        // Do we have a Display Name?
        String strDisplayName = parser.getDisplayName() ;
        if ((strDisplayName != null) && (strDisplayName.length() > 0)) {
            strName = strDisplayName ;
        } else {
            // Okay, how about a user id?
            String strUserID = parser.getUser() ;
            if ((strUserID != null) && (strUserID.length() > 0)) {
                strName = strUserID ;
            }
        }

        HookManager hookManager = Shell.getHookManager() ;
        try {
            CallerIDHookData data = new CallerIDHookData(PAddressFactory.getInstance().createAddress(strSIPURL), strName) ;
            hookManager.executeHook(HookManager.HOOK_CALLER_ID, data) ;
            strName = data.getIdentity() ;
        } catch (PCallAddressException e) {
            SysLog.log(e) ;
        }

        return strName ;
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
