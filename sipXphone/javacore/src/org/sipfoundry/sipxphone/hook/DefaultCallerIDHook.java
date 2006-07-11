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

package org.sipfoundry.sipxphone.hook ;

/**
 * Default CallerID hook used by the xpressa(TM) phone.  The default CallerID hook
 * uses the default identity specified when the hook data object was created.
 *
 */
public class DefaultCallerIDHook implements Hook
{
    /**
     * Required; try to identify a call.  
     * Invoking <i>setIdentity</i> implicitly calls <i>terminate</i> and 
     * prevents downstream hooks from executing.
     */
    public void hookAction(HookData data)
    {        
        if ((data != null) && (data instanceof CallerIDHookData)) {       
            CallerIDHookData hookData = (CallerIDHookData) data ;
            
            // Default CallerID Hook just uses whatever the hook invoker
            // suggested, but you could easily do something more 
            // interesting...  Perhaps look up in the yellow pages or query
            // a company directory service.
            hookData.setIdentity(hookData.getIdentity()) ;
        }
    }
}
