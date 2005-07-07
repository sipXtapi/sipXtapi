/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/hook/CoreAppButtonHook.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

 package org.sipfoundry.sipxphone.hook ;
 import org.sipfoundry.sipxphone.sys.appclassloader.ApplicationManager;
 import org.sipfoundry.sipxphone.awt.form.*;
 import org.sipfoundry.sipxphone.awt.event.PButtonEvent;
 import org.sipfoundry.sipxphone.sys.* ;
 import org.sipfoundry.sipxphone.sys.app.* ;

/**
 * CoreAppButtonHook.java
 *
 * The CoreAppButtonHook is installed by the ShellApp when ShellApp is
 * loaded and make CoreApp take appropriate actions after receiving buttonEvents.
 *
 * Created: Tue Apr 02 16:03:27 2002
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */

public class CoreAppButtonHook implements Hook{

    /**
     * Basically delegates CoreApp to take appropriate action.
     */
    public void hookAction(HookData data)
    {
        try {
            PButtonEvent event = ((ButtonHookData)(data)).getEvent();
            switch (event.getEventType()) {
                case PButtonEvent.BUTTON_DOWN:
                    ShellApp.getInstance().getCoreApp().buttonDown(event) ;
                    break ;
                case PButtonEvent.BUTTON_UP:
                    ShellApp.getInstance().getCoreApp().buttonUp(event) ;
                    break ;
            }
        } catch (Throwable t) {
            Shell.getInstance().showUnhandledException(t, false) ;
        }

    }

}// CoreAppButtonHook







