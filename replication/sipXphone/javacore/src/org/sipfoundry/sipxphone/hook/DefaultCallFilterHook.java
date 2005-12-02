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

import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.sipxphone.app.preferences.CallHandlingManager ;
import org.sipfoundry.sip.SipLine ;
import org.sipfoundry.sip.SipSession ;
import org.sipfoundry.sip.SipLineManager ;


/**
 * Default CallFilter hook used by the xpressa(TM) phone.  The default CallFilter hook
 * accepts the call if no other calls are in focus.
 *
 */
public class DefaultCallFilterHook implements Hook
{
    /**
     * Required; take a filter action.
     * Calling <i>accept</i>, <i>decline</i>, or <i>redirect</i> will implicitly call
     * <i>terminate</i> and prevents downstream hooks from executing.
     */
    public void hookAction(HookData data)
    {
         if ((data != null) && (data instanceof CallFilterHookData))
        {
            CallFilterHookData hookData = (CallFilterHookData) data ;
            CallHandlingManager manager = Shell.getCallHandlingManager();
            PCallManager callManager = Shell.getCallManager() ;
            PAddress address = hookData.getCalledAddress() ;
            PAddress sipAddress = null ;
            PCall call = callManager.getCallByCallID(hookData.getCallID()) ;
            SipSession session = null ;
            if (call != null)
                session = (SipSession) call.getSession(hookData.getAddress()) ;

            if (address instanceof PSIPAddress)
                sipAddress = (PSIPAddress) address ;
            else
            {
                try
                {
                    sipAddress = PAddressFactory.getInstance().createAddress(address.getAddress()) ;
                }
                catch (Exception e) {}
            }
            boolean bForwardingAllowed = callManager.isForwardingAllowed(session) ;


            // Check to see if DND is enabled
            if (manager.isDNDEnabled())
            {
                if (bForwardingAllowed)
                {
                    switch (manager.getDNDMethod())
                    {
                        case CallHandlingManager.DND_FORWARD_ON_BUSY:
                            String strRedirectURL = manager.getTranslatedSipForwardingBusy() ;
                            if ((strRedirectURL != null) && (strRedirectURL.length() > 0))
                            {
                                System.out.println("DND_FORWARD_ON_BUSY redirect: " + strRedirectURL) ;
                                try
                                {
                                    hookData.redirect(PAddressFactory.getInstance().createAddress(strRedirectURL)) ;
                                }
                                catch (PCallAddressException e)
                                {
                                    Shell.getInstance().showUnhandledException(e, false);
                                }
                            }
                            else
                            {
                                System.out.println("DND_FORWARD_ON_BUSY decline: no forward address set") ;
                                hookData.decline() ;
                            }
                            break ;
                        case CallHandlingManager.DND_FORWARD_ON_NO_ANSWER:
                            Shell.getCallManager().ignoreCallByID(hookData.getCallID()) ;
                            break ;
                        case CallHandlingManager.DND_SEND_BUSY:
                            System.out.println("DND_SEND_BUSY decline") ;
                            hookData.decline() ;
                            break ;
                    }
                }
                else
                {
                    hookData.decline() ;
                }
            }
            else
            {
                if (bForwardingAllowed && manager.isForwardingAllEnabled())
                {
                    System.out.println("DefaultCallFilterHook: FORWARD ALL: " + manager.getTranslatedSipForwardingAll()) ;
                    try
                    {
                        hookData.redirect(PAddressFactory.getInstance().createAddress(manager.getTranslatedSipForwardingAll())) ;
                    }
                    catch (PCallAddressException e)
                    {
                        Shell.getInstance().showUnhandledException(e, false);
                    }


                }
                else
                {
                    PCall callThis = Shell.getCallManager().getCallByCallID(hookData.getCallID()) ;
                    // If a call is in focus and call waiting is off or we are over the max connections
                    if ((Shell.getCallManager().getInFocusCall() != callThis) && (!manager.isCallWaitingEnabled())) {

                        if (manager.isForwardOnBusyEnabled() && bForwardingAllowed)
                        {
                            try
                            {
                                System.out.println("DefaultCallFilterHook: BUSY redirect: " + manager.getTranslatedSipForwardingBusy()) ;
                                hookData.redirect(PAddressFactory.getInstance().createAddress(manager.getTranslatedSipForwardingBusy())) ;
                            }
                            catch (PCallAddressException e)
                            {
                                Shell.getInstance().showUnhandledException(e, false);
                            }
                        }
                        else
                        {

                            if (Shell.getCallManager().getInFocusCall() == null)
                            {
                                // there is no call in focus, so, just accept
                                System.out.println("DefaultCallFilterHook: accept") ;
                                hookData.accept();
                            }
                            else
                            {
                                System.out.println("DefaultCallFilterHook: BUSY decline: " + manager.getTranslatedSipForwardingBusy()) ;
                                hookData.decline() ;
                            }
                        }
                    }
                    else
                    {
                        System.out.println("DefaultCallFilterHook: accept") ;
                        hookData.accept() ;
                    }
                }
            }
        }
    }
}
