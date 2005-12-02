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
 * <p>
 * This is the generic Hook interface used by all of the xpressa hooks. A 
 * single method, <i>hookAction</i>, is invoked when the hook is evaluated.  Also, 
 * the hook implementation is expected to cast the passed HookData object to
 * the appropriate, hook-specific data type.
 * 
 * <p>
 * Hooks are executed in the order added. Generally, the hooks are evaluated 
 * until a hook takes an explicit action and thereby terminates the chain.
 * If no user-installed hooks take action, a default, system-defined hook is 
 * evaluated and takes some default action.
 *
 * <p>
 * NOTE: A hook will be uninstalled if it throws an exception.
 *
 * @see org.sipfoundry.sipxphone.sys.HookManager
 * @see org.sipfoundry.sipxphone.hook.HookData
 * 
 * @author Robert J. Andreasen, Jr.
 */  
public interface Hook
{
    /**
     * A hook must define this method and may act on the passed HookData 
     * object if an action is desired.  Acting on the data will often
     * implicitly call the <i>terminate</i> method; however, you may be required
     * to call <i>terminate</i> explicitly.
     * <p>
     * Most hooks use custom data objects that extend HookData. Developers
     * should read the documentation on the specific hook/HookData before
     * attempting to interact with it.
     * 
     *
     * @see org.sipfoundry.sipxphone.hook.CallFilterHookData
     * @see org.sipfoundry.sipxphone.hook.CallerIDHookData
     * @see org.sipfoundry.sipxphone.hook.RingerHookData
     * @see org.sipfoundry.sipxphone.hook.MatchDialplanHookData
     * @see org.sipfoundry.sipxphone.hook.NewCallHookData
     */
    public void hookAction(HookData data) ;
}
