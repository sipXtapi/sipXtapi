/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/app/ModalMessageBoxApp.java#2 $
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

import org.sipfoundry.sipxphone.Application ;
import org.sipfoundry.sipxphone.awt.form.MessageBox ;

/**
 * This app is used to show a modal message box. You may wonder that we already
 * have a MessageBox class that can be shown modally. Well, that is true in single
 * thread context but not true in multiple thread contexts. Lets say,
 * when a "Conference" button is pressed, you create a MessageBox
 * and show it using code like
 *
 * <code>
 * MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
 * messageBox.showModal() ;
 * </code>
 *
 * Multiple message boxs will pop up if the "Conference" button is
 * pressed multiple times as the events are received and processed
 * in different thread contexts.
 *
 * In a situation like the one described above, this becomes handy.
 *
 *
 * @author Pradeep Paudyal
 */
public class ModalMessageBoxApp extends Application
{

    /**
     * show a modal message box with the title and message passed in argv.
     * Title is the first string in argv and message is the second string in
     * argv.
     */
    public void main(String argv[])
    {
        String strTitle = "";
        String strMessage = "";
        int argLength = argv.length;
        if( argLength > 0 )
            strTitle   = argv[0];
        if( argLength > 1 )
            strMessage = argv[1];
        MessageBox errorMsgBox =
                    new MessageBox(this, MessageBox.TYPE_ERROR) ;
        errorMsgBox.setTitle( strTitle );
        errorMsgBox.setMessage( strMessage );
        errorMsgBox.showModal();
    }
}
