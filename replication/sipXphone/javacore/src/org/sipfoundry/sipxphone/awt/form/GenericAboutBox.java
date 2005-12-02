/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/awt/form/GenericAboutBox.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sipxphone.awt.form;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;

import org.sipfoundry.util.* ;
import java.awt.event.* ;
import java.util.* ;
import java.io.* ;


public class GenericAboutBox
{
    public GenericAboutBox()
    {
    }
/**
  * The generic about box is a generic about box that should be used to display
  * application version and name for a user application
  * title and version are extracted from Application properties file in jar
 *
 * @since 1.2
 *
 *
 * @param app reference to the application class
 * @param strCopyright copyright to display
 *
 * @author dwinsor
 * @deprecated <B>NOT EXPOSED</B>
 */
    static public void display(Application app, String strCopyright)
    {

        // Get the Software revision
        String strSoftwareRevision = app.getPropertyValue("Version") ;
        String strAppName = app.getPropertyValue("Title") ;
        if (strSoftwareRevision == null)
        {
            strSoftwareRevision = "Unknown" ;
        }
        if (strAppName == null)
        {
            strSoftwareRevision = "Unknown" ;
        }

        // Display the message box
        MessageBox about = new MessageBox(app, MessageBox.TYPE_INFORMATIONAL);

        about.setMessage(strAppName + " Version " + strSoftwareRevision +
            "\n\n" + strCopyright + "\n");
        about.setTitle("About " + strAppName);

        about.showModal() ;
    }
}