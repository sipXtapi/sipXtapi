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


package org.sipfoundry.sipxphone.sys.app.core ;

import java.net.* ;
import java.awt.* ;
import java.util.* ;
import java.text.* ;

import javax.telephony.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.sys.app.* ;
import org.sipfoundry.sipxphone.sys.startup.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.telephony.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.appclassloader.* ;
import org.sipfoundry.sipxphone.sys.calllog.* ;
import org.sipfoundry.sipxphone.service.* ;

import org.sipfoundry.sipxphone.sys.app.shell.* ;
import org.sipfoundry.sip.* ;

import org.sipfoundry.stapi.* ;
import org.sipfoundry.sipxphone.sys.util.*;
import org.sipfoundry.sipxphone.featureindicator.* ;

/**
 * Generic about box used to display version information for the core
 * application forms such as dialing, call in progress, etc.
 *
 * @author Dan Winsor
 */
public class SystemAboutBox extends MessageBox
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////////
// Attributes
////


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructor our form under the specified application context
     */
    public SystemAboutBox(Application application)
    {
        super(application) ;

        populate();
    }


    /**
     * Constructor this form given a parent form
     */
    public SystemAboutBox(PForm formParent)
    {
        super(formParent) ;

        populate();
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    public void onAbout()
    {
        if (showModal() == MessageBox.B1)
        {
            displayInfobox(getParentForm(), getApplication()) ;
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     *
     */
    private void populate()
    {
        XpressaSettings settings = Shell.getXpressaSettings() ;

        // Get SIP URL Identity
        String strSIPURL = getIdentity() ;

        // Get the Software revision
        String strSoftwareRevision = settings.getVersion() ;
        if (strSoftwareRevision == null)
            strSoftwareRevision = "Unknown" ;
        else
        {
            int iBuildNumber = settings.getBuildNumber() ;
            strSoftwareRevision = VersionUtils.buildVersionString(strSoftwareRevision, iBuildNumber) ;
        }

        // Get the Build Date
        String strBuildDate = settings.getBuildDate() ;
        if (strBuildDate == null)
            strBuildDate = "Unknown" ;

        // Get the build comment
        String strBuildComment = settings.getBuildComment() ;
        if (strBuildComment == null)
            strBuildComment = "" ;

        //get resources version if any
        String strResourcesVersion = settings.getResourcesVersion() ;

        // Get Kernel Version
        String strKernelRelease = PingerInfo.getInstance().getKernelVersion() ;
        String strKernelBuildDate = PingerInfo.getInstance().getKernelBuildDate() ;

        // for a user like in a hotel,
        //  Software Revision displays the software versions and copyrights,
        //  but does *not* display their SIP URL or "Info" button unless
        //  in Admin mode .
        boolean bGuestMode = PSecurityManager.getInstance().isInGuestFeaturesMode();
        boolean bShowGuestAdminFeatures = PSecurityManager.getInstance().shouldShowGuestAdminFeatures();
        boolean bRestricted = false;
        if( bGuestMode && !bShowGuestAdminFeatures )
            bRestricted = true;

        String messageStr = "";
         //TO DO(pradeep): change this to use StringBuffer
        if( !bRestricted)
        {
            messageStr = "SIP URL: " + strSIPURL + "\n\n" ;
        }

         messageStr +=   "Core Apps: " + strSoftwareRevision + "\n" +
            strBuildDate + "\n" +
            strBuildComment + "\n\n" +
            ((strResourcesVersion == null)?"":
                ("Resources: "+strResourcesVersion+"\n\n"))+
            "Kernel: " + strKernelRelease + "\n" +
            strKernelBuildDate;


        if (PingerApp.isTestbedWorld())
        {
            String strRegistered = "\n\nRegistered To:\n";
            String strAdminDomain = PingerInfo.getInstance().getAdminDomain();

            String strExpireDate = "\nLicense Expiration Date:\n";
            String strExpire = PingerInfo.getInstance().getExpireDate() ;

            if (strExpire.equals("-1"))
            {
                strExpireDate += "None";
            }
            else
            {
                try
                {
                   long iSecs = Long.valueOf(strExpire).longValue()* 1000 ;
                   Date datExpire = new Date(iSecs) ;
                   strExpireDate += datExpire.toString();
                } 
                catch (NumberFormatException nfe)
                {
                    strExpireDate += strExpire ; 
                }

            }

            messageStr += strRegistered;
            messageStr += strAdminDomain;
            messageStr += strExpireDate;
        }

        setMessage(messageStr) ;

        if( !bRestricted )
            setButton(MessageBox.B1, "Info", getString("hint/coreapp/idle/about_info")) ;

        setTitle("About your sip softphone") ;
   }


    /**
     *
     */
    private void displayInfobox(PForm formParent, Application app)
    {
        String strUptime = null ;
        String strIPAddress = "Unknown" ;
        String strIPMode = null ;
        String strMACAddress = null ;
        PingerInfo pingerInfo = PingerInfo.getInstance() ;

        // Get Uptime
        strUptime = pingerInfo.getUptime() ;

        // Mac Address / Serial Number
        strMACAddress = pingerInfo.getMacAddress() ;
        String strMACAddressLabel ;
        if (PingerApp.isTestbedWorld() == true) {
            strMACAddressLabel = "Serial No.: " ;
        } else {
            strMACAddressLabel = "MAC: " ;
        }

        // Initialize the message box
        MessageBox msgBox ;
        if (formParent != null)
        {
            msgBox = new MessageBox(formParent) ;
        }
        else
        {
            msgBox = new MessageBox(app) ;
        }
        msgBox.setMessage( "\n" +
                           "Uptime: " + strUptime + "\n" +
                           strIPMode + strIPAddress + "\n" +
                           strMACAddressLabel + strMACAddress + "\n" +
                           "Free JVM Memory: " + (Runtime.getRuntime().freeMemory() / 1024)+"K\n" +
                           "Total JVM Memory: " + (Runtime.getRuntime().totalMemory()/1024)+"K\n" +
                           "\n" +
                           getString("lblCopyrightInfo")) ;

        msgBox.setTitle("More Information") ;
        msgBox.showModal() ;
    }


    /**
     * Determine the "SIP URL" for this phone, where the SIP URL: is the
     * device line if present, otherwise the first User line.
     */
    private String getIdentity()
    {
        String  strRC = "Unavailable" ;
        SipLine identity = null ;
        SipLineManager manager = SipLineManager.getInstance() ;


        // First try to find the device line
        SipLine lines[] = manager.getLines() ;
        for (int i=0; i<lines.length; i++)
        {
            if (lines[i].isDeviceLine())
            {
                identity = lines[i] ;
                break ;
            }
        }

        // If we don't find the device line then pick the default line
        if (identity == null)
            identity = manager.getDefaultLine() ;

        // Lastly clean up the line identity, by removing the display name
        // and all tags.
        if (identity != null)
        {
            SipParser parser = new SipParser(identity.getIdentity()) ;
            parser.removeAllFieldParameters() ;
            parser.removeAllHeaderParameters() ;
            parser.removeAllURLParameters() ;
            parser.setDisplayName(null) ;

            strRC = parser.render() ;


            // Lastly remove the "sip:"
            if (strRC.toLowerCase().startsWith("sip:"))
            {
                strRC = strRC.substring(4) ;
            }
        }
        return strRC ;
    }
}

