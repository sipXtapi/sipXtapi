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

package org.sipfoundry.sipxphone.app ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.sipxphone.sys.* ;

import org.sipfoundry.util.AppResourceManager ;

import org.sipfoundry.sipxphone.app.preferences.* ;

/**
 * Allows users to view and set their preferences.  If no command line
 * parameters are included then the main form is displayed.  Otherwise, the
 * invoker can bring up a specific page by selecting the one of the following
 * string keys
 *
 */
public class DevicePreferencesApp extends Application
{
    public static final String PREF_NETWORK          = "NETWORK" ;
    public static final String PREF_CALL_HANDLING    = "CALL_HANDLING" ;
    public static final String PREF_TIME_LOCALE      = "TIME_LOCALE" ;
    public static final String PREF_VOLUME_CONTRAST  = "VOLUME_CONTRAST" ;
    public static final String PREF_WEB              = "WEB" ;
    public static final String PREF_USER_MAINTENANCE = "USER_MAINTENANCE" ;
    public static final String PREF_LINE_MAINTENANCE = "LINE_MAINTENANCE" ;

    public void main(String argv[])
    {
        PreferencesMainForm form = new PreferencesMainForm(this) ;

        if ((argv != null) && (argv.length > 0))
        {
            String strArg = argv[0] ;

            // Init Resource Manager
            AppResourceManager resMgr = AppResourceManager.getInstance() ;
            resMgr.addStringResourceFile(PreferencesMainForm.RESOURCE_FILE, this) ;

            // Compare away
            if (strArg.equalsIgnoreCase(PREF_NETWORK))
            {
                form.selectCategory(resMgr.getString("lblNetwork"));
            }
            else if (strArg.equalsIgnoreCase(PREF_CALL_HANDLING))
            {
                form.selectCategory(resMgr.getString("lblCallHandling"));
            }
            else if (strArg.equalsIgnoreCase(PREF_TIME_LOCALE))
            {
                form.selectCategory(resMgr.getString("lblTimezone"));
            }
            else if (strArg.equalsIgnoreCase(PREF_VOLUME_CONTRAST))
            {
                form.selectCategory(resMgr.getString("lblVolume"));
            }
            else if (strArg.equalsIgnoreCase(PREF_WEB))
            {
                form.selectCategory(resMgr.getString("lblWebServer"));
            }
            else if (strArg.equalsIgnoreCase(PREF_USER_MAINTENANCE))
            {
                form.selectCategory(resMgr.getString("lblUserMaintenance"));
            }
            else if (strArg.equalsIgnoreCase(PREF_LINE_MAINTENANCE))
            {
                form.selectCategory(resMgr.getString("lblLineManager"));
            }
            else
            {
                form.showModal() ;
            }
        }
        else
        {
            form.showModal() ;
        }

        // Adjust the DND feature indicator as needed.
        DoNotDisturbFeatureIndicator fiDND = DoNotDisturbFeatureIndicator.getInstance() ;
        if (fiDND.shouldInstall())
            fiDND.install() ;
        else
            fiDND.uninstall() ;

        CallForwardingFeatureIndicator fiCW = CallForwardingFeatureIndicator.getInstance() ;
        if (fiCW.shouldInstall())
            fiCW.install() ;
        else
            fiCW.uninstall() ;

    }

}
