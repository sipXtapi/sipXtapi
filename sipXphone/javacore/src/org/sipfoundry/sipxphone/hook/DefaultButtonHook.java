/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/hook/DefaultButtonHook.java#2 $
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
 * DefaultButtonHook.java
 *
 * Default Button hook used by the xpressa(TM) phone.  The default Button hook
 * delegates the formmanager to fire this event if this event is not consumed.
 *
 * Created: Tue Apr 02 16:03:27 2002
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */

public class DefaultButtonHook implements Hook{

    /**
     * The default Button hook delegates the formmanager to fire this event
     * if this event is not consumed.
     */
    public void hookAction(HookData data)
    {
        PButtonEvent event = ((ButtonHookData)(data)).getEvent();
        if (!event.isConsumed()) {
            ShellApp.getInstance().getFormManager().firePingtelEvent(event) ;
        }
    }

}// DefaultButtonHook







