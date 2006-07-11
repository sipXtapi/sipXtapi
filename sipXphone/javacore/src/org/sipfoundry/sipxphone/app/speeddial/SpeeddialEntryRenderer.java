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


package org.sipfoundry.sipxphone.app.speeddial ;

import java.awt.* ;
import javax.naming.directory.BasicAttributes;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.directoryservice.provider.*;
    
    
/**
 *
 */
public class SpeeddialEntryRenderer extends PDefaultItemRenderer
{
    // font for speed dial number in LH column of list    
    protected Font fontSpeeddialNumber = SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_LARGE) ;
    // font for entry name - top row of an entry
    protected Font fontName = SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT_LARGE) ;
    // font for address - bottom row of an entry
    protected Font fontUserID = SystemDefaults.getFont(SystemDefaults.FONTID_DEFAULT) ; 

    protected SpeedDialDSP m_speedDialDSP =
        (SpeedDialDSP) DirectoryServiceProviderRegistry.getDirectoryServiceProvider("speed_dial");
    
    /**
     *
     */
    public void drawItemText(Graphics g) 
    {           
        if (m_objData != null) {
            int x = 0 ;
            int y = 2 ; 
            
            BasicAttributes attr = (BasicAttributes) m_objData;
            
            String strSpeedDialId    = m_speedDialDSP.getSpeedDialId(attr);
            String strSIPUserName    = m_speedDialDSP.getSIPUserName(attr);
            String strSIPUserId  = m_speedDialDSP.getSIPUserId(attr);
            String strSIPAddress = m_speedDialDSP.getSIPAddress(attr);                              
            String strLabel = m_speedDialDSP.getLabel(attr);        
            String strPhoneNumber = m_speedDialDSP.getSpeedDialPhoneNumber(attr);
            
            if (strSpeedDialId == null)
                strSpeedDialId = "" ;
            if (strSIPUserName == null)
                strSIPUserName = "" ;
            if (strSIPUserId == null)
                strSIPUserId = "" ;    
            if (strSIPAddress == null)
                strSIPAddress = "" ;  
            if (strLabel == null)
                strLabel = "" ;
            if (strPhoneNumber == null)
                strPhoneNumber = "" ;
        
            //System.out.println("#### strSpeedDialId " + strSpeedDialId + ", strLabel " + strLabel + ", strSIPUserName " + strSIPUserName + ", strSIPUserId " + strSIPUserId + ", strSIPAddress " + strSIPAddress + ", strPhoneNumber " + strPhoneNumber);

            FontMetrics fm   = g.getFontMetrics(fontSpeeddialNumber) ;
            int iStatusWidth = fm.stringWidth(strSpeedDialId) ;
                                        
            g.setColor(m_colorText) ;
            
            // Draw the Speed dial number
            g.setFont(fontSpeeddialNumber) ;
            g.drawString(strSpeedDialId, x, y+12) ;
    
            // Draw the label
            g.setFont(fontName);
            g.drawString(strLabel, x+iStatusWidth+4, y+8) ;
            
            // Regular old phone number
            if (strPhoneNumber.length() != 0) {
                //System.out.println("####> strPhoneNumber");
                g.setFont(fontUserID) ;
                g.drawString(strPhoneNumber, x+iStatusWidth+4, y+18) ;
            }
            
            // SIP User Name
            else if (strSIPUserName.length() != 0) {
                //System.out.println("####> strSIPUserName");
                g.setFont(fontUserID) ;
                g.drawString(strSIPUserName, x+iStatusWidth+4, y+18) ;
            }
            
            // SIP User ID
            else if (strSIPUserId.length() != 0) {
                //System.out.println("####> strSIPUserId");
                g.setFont(fontUserID) ;
                g.drawString(strSIPUserId, x+iStatusWidth+4, y+18) ;
            }
            
            // SIP Address, we hope
            else {
                //System.out.println("####> strSIPAddress");
                g.setFont(fontUserID) ;
                g.drawString(strSIPAddress, x+iStatusWidth+4, y+18) ;
            }
        }
    }
    
}         
