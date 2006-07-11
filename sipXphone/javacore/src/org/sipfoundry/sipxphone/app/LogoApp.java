/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/app/LogoApp.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.app ;

import org.sipfoundry.sipxphone.awt.*;
import org.sipfoundry.sipxphone.awt.event.*;
import org.sipfoundry.sipxphone.awt.form.*;
import org.sipfoundry.sipxphone.Application;
import org.sipfoundry.sipxphone.sys.*;
import org.sipfoundry.sipxphone.featureindicator.*;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.sipxphone.sys.startup.* ;

import org.sipfoundry.util.* ;
import java.awt.* ;
import java.io.* ;
import java.net.* ;


/**
 * This is a simple application that places a Logo into the feature
 * indicator area.  The logo URL displayed is configured by setting
 * the PHONESET_LOGO_URL configuration parameter.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class LogoApp extends Application
{
    /**
     * The Main Application Area
     */
    public void main(String argv[])
    {
        URL url = getLogoURL() ;
        if (url != null) {
            Image image = Toolkit.getDefaultToolkit().getImage(url) ;
            if (image != null) {
                // Install the feature indicator, assuming that the
                // URL is available and obtainable
                FeatureIndicatorManager indicatorManager = Shell.getFeatureIndicatorManager() ;
                if (indicatorManager != null) {
                    indicatorManager.installIndicator(new icLogoIndicator(image), FeatureIndicatorManager.VIEWSTYLE_LINE) ;
                }
            } else {
                System.out.println("LogoApp: Unable to get image: " + url.toExternalForm()) ;
            }
        }
    }


    /**
     * Get the URL of the logo that we should display in our feature indicator
     * area.
     */
    public URL getLogoURL()
    {
        URL     url = null ;
        String  strURL = PingerConfig.getInstance().getValue(PingerConfig.PHONESET_LOGO_URL) ;

        // Ignore blank URLs
        if ((strURL != null) && (strURL.trim().length() > 0)) {
            try {
                url = new URL(strURL) ;
            } catch (MalformedURLException mue) {
                SysLog.log(mue) ;
                url = null ;
            }
        }
        return url ;
    }


    /**
     * This object represents the logo/icon for feature indicator.
     */
    public class icLogoIndicator implements FeatureIndicator
    {
        Image m_image ;

        public icLogoIndicator(Image image)
        {
            m_image = image ;
        }


        public Image getIcon()
        {
            return null ;
        }

        public String getShortDescription()
        {
            return null ;
        }


        public Component getComponent()
        {
            PLabel label = new PLabel(m_image) ;

            MediaTracker            tracker ;
            tracker = new MediaTracker(label) ;

            tracker.addImage(m_image, 0) ;

            try {
                tracker.waitForID(0) ;
                if (tracker.isErrorID(0)) {
                    System.out.println("\tError loading logo image.") ;
                    Shell.getFeatureIndicatorManager().removeIndicator(this) ;
                    label = null ;
                }
            } catch (InterruptedException ie) {
                SysLog.log(ie);
            }

            return label ;
        }


        public String getHint()
        {
            return null ;
        }


        public void buttonPressed()
        {
        }
    }
}
