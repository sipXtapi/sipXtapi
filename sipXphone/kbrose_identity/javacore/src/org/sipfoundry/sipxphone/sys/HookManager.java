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


package org.sipfoundry.sipxphone.sys ;

import org.sipfoundry.sipxphone.hook.* ;
import java.util.* ;


/**
 * The Hook Manager provides a centralized place for installing and deinstalling
 * system hooks.
 * <p>
 * Hooks are time critical actions or decisions that can be changed by third
 * party developers. To supplement or replace a default implementation provided
 * by the system, developers can install a hook.
 * <p>
 * As an example, one of the supported hooks is the HOOK_RINGER. This hook allows
 * the application developer to override the default behavior of the system
 * when it wishes to notify the user of an incoming call. To change the ring
 * sound used for incoming calls, you would use HOOK_RINGER.
 * <p>
 * More than one hook of a given type can be installed. When the system attempts to
 * perform an action that has a related hook, it proceeds
 * through the list of registered hooks of that type. The list is ordered by when the
 * hook was installed. For example, the first hook of type HOOK_RINGER installed is
 * considered first when there is an incoming call. When called, a
 * hook can explicitly terminate the chain, or do nothing and implicitly allow
 * the next hook to proceed and take action.
 * <p>
 * NOTE: If a hook throws an exception, that hook will be uninstalled. The system
 * assumes that if the hook throws an exception, then the hook has failed and is no
 * longer valid.
 *
 * @see Hook
 * @see HookManager
 * @see org.sipfoundry.sipxphone.hook.CallFilterHookData
 * @see org.sipfoundry.sipxphone.hook.CallerIDHookData
 * @see org.sipfoundry.sipxphone.hook.RingerHookData
 * @see org.sipfoundry.sipxphone.hook.MatchDialplanHookData
 * @see org.sipfoundry.sipxphone.hook.NewCallHookData
 *
 * @author Robert J. Andreasen, Jr.
 */
public class HookManager
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /**
     * @deprecated DO NOT EXPOSE
     */
    public static boolean DEBUG_EVENTS = false ;

    /** The Caller ID hook provides identifying information for callers and callees.
        This information is displayed to the end user in core phone forms, including
        call in progress, incoming call, and waiting for call, etc.
        Upon evaluation, hook implementations can safely cast the HookData
        object to a CallerIDHookData object.*/
    public static final int HOOK_CALLER_ID   = 1 ;

    /** The Ringer hook defines the visual or auditory alert presented to the
        end user for an incoming call. Upon evaluation, hook implementations
        can safely cast the HookData object to a RingerHookData object.*/
    public static final int HOOK_RINGER      = 2 ;

    /** The Call Filter hook allows an application to accept, reject, or
        redirect an incoming call. Upon evaluation, hook implementations can
        safely cast the HookData object to a CallFilterHookData object.*/
    public static final int HOOK_CALL_FILTER = 3 ;

    /** The Dialplan Match hook allows developers to determine when a dialing
        string matches a known or "good" dialing plan. The xpressa phone uses this
        information to preempt digit collection when dialing. Developers
        can also adjust or reform the dialing string or address as needed. Upon
        evaluation, hook implementations can safely cast the HookData object
        to a MatchDialplanHookData object.*/
    public static final int HOOK_MATCH_DIALPLAN = 4 ;

    /** The New Call hook controls what happens when the core software creates
        a new call for the purpose of dialing.  This occurs when the end user
        lifts the handset, presses the speaker button, presses the headset
        button, or presses the new call soft button.  */
    public static final int HOOK_NEW_CALL = 5 ;

    /**
     * ButtonHook allows you to control what happens when your application
     * receives a ButtonEvent.
     * @deprecated DO NOT EXPOSE
     */
    public static final int HOOK_BUTTON   = 6 ;



    private static final int MIN_HOOK_ID = 1 ;  // Min hook value; used for
                                                // bounds checking
    private static final int MAX_HOOK_ID = 6 ;  // Max hook value; used for
                                                // bounds checking


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** our table of hooks key off the Integer representation of the hook
        id */
    private Hashtable htHooks ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    protected HookManager()
    {
        htHooks = new Hashtable() ;
        installDefaultHooks() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Install a hook. This method does nothing if the hook was
     * already added.
     *
     * @param iHookID The hook identifier.
     * @param hook The hook implementation.
     *
     * @exception SecurityException Thrown if the user/application does not
     *            have permission to install a hook.
     * @exception IllegalArgumentException Thrown if the iHookID is not valid.
     */
    public synchronized void installHook(int iHookID, Hook hook)
        throws SecurityException, IllegalArgumentException
    {
        if ((iHookID < MIN_HOOK_ID) || (iHookID > MAX_HOOK_ID))
            throw new IllegalArgumentException("invalid hook id") ;

        Integer key = new Integer(iHookID) ;

        if (htHooks.containsKey(key)) {
            Vector vList = (Vector) htHooks.get(key) ;
            if (!vList.contains(hook)) {
                // Insert before default hook...
                if (vList.size() > 0)
                    vList.insertElementAt(hook, vList.size()-1) ;
                else
                    vList.addElement(hook) ;
            }
        } else {
            Vector vList = new Vector() ;
            vList.addElement(hook) ;
            htHooks.put(key, vList) ;
        }
    }


    /**
     * Deinstall a hook. This method does nothing if the hook is not found.
     *
     * @param iHookID The hook identifier.
     * @param hook The hook implementation.
     *
     * @exception SecurityException Thrown if the user/application does not
     *            have permission to install a hook.
     * @exception IllegalArgumentException Thrown if the iHookID is not valid.
     */
     public synchronized void deinstallHook(int iHookID, Hook hook)
        throws SecurityException, IllegalArgumentException
    {
        if ((iHookID < MIN_HOOK_ID) || (iHookID > MAX_HOOK_ID))
            throw new IllegalArgumentException("invalid hook id") ;

        Integer key = new Integer(iHookID) ;
        if (htHooks.containsKey(key)) {
            Vector vList = (Vector) htHooks.get(key) ;
            vList.removeElement(hook) ;
        }
    }


    /**
     * Invoke or execute a hook.
     *
     * @param iHookID The hook identifier.
     * @param data The data object passed to the hook.
     *
     * @exception SecurityException Thrown if the user/application does not
     *            have permission to install a hook.
     * @exception IllegalArgumentException Thrown if the iHookID is not valid.
     */
    public void executeHook(int iHookID, HookData data)
        throws SecurityException, IllegalArgumentException
    {
        if (DEBUG_EVENTS)
            System.out.println("************* JAVA executeHook ENTER");

        if ((iHookID < MIN_HOOK_ID) || (iHookID > MAX_HOOK_ID))
            throw new IllegalArgumentException("invalid hook id") ;

        Integer key = new Integer(iHookID) ;
        if (htHooks.containsKey(key)) {
            Vector vList = (Vector) htHooks.get(key) ;

            synchronized (vList)
            {

                for (int i=0; i<vList.size(); i++) {
                    Hook hook = (Hook) vList.elementAt(i) ;
                    if (hook != null)
                    {
                        try {
                            hook.hookAction(data) ;
                            if (data.isTerminated())
                                break ;
                        } catch (Exception e) {
                            System.out.println("Uncaught Exception from Hook: " + e) ;
                            System.out.println("Removing hook...") ;
                            deinstallHook(iHookID, hook) ;
                            i-- ;
                        }
                    }
                }
            }
        } else {
            System.out.println("HookManager: Trying to execute empty hook:" + iHookID) ;
        }

        if (DEBUG_EVENTS)
            System.out.println("************* JAVA executeHook EXIT");
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    protected void installDefaultHooks()
    {
        installHook(HOOK_CALLER_ID, new DefaultCallerIDHook()) ;
        installHook(HOOK_RINGER, new DefaultRingerHook()) ;
        installHook(HOOK_CALL_FILTER, new DefaultCallFilterHook()) ;
        installHook(HOOK_MATCH_DIALPLAN, new DefaultMatchDialplanHook()) ;
        installHook(HOOK_MATCH_DIALPLAN, new DigitMapMatchDialplanHook()) ;
        installHook(HOOK_NEW_CALL, new DefaultNewCallHook()) ;
        installHook(HOOK_BUTTON, new DefaultButtonHook()) ;
    }
}
