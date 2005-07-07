/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/app/CoreAboutbox.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.app;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.awt.form.* ;

import org.sipfoundry.util.* ;
import java.awt.event.* ;
import java.util.* ;


/**
 * The core about box is a generic about box that should be used to display
 * application information and versioning info from built-in application like
 * speeddial, conference, transfer, etc.
 */
public class CoreAboutbox
{
    public CoreAboutbox()
    {
    }

    static public void display(Application app, String appname)
    {
        XpressaSettings settings = Shell.getXpressaSettings() ;

        // Get the Software revision
        String strSoftwareRevision = settings.getVersion() ;
        if (strSoftwareRevision == null)
        {
            strSoftwareRevision = "Unknown" ;
        }
        else
        {
            int iBuildNumber = settings.getBuildNumber() ;
            strSoftwareRevision = VersionUtils.buildVersionString(strSoftwareRevision, iBuildNumber) ;
        }

        // Display the message box
        MessageBox about = new MessageBox(app, MessageBox.TYPE_INFORMATIONAL);

        about.setMessage(appname + " " + strSoftwareRevision +
            "\n\nCopyright (C) 2001, Pingtel Corp.\n");
        about.setTitle("About " + appname);

        about.showModal() ;
    }
}
