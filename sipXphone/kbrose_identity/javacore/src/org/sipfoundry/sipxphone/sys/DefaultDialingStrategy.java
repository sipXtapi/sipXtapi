/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/DefaultDialingStrategy.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.sys;

import org.sipfoundry.stapi.* ;
import org.sipfoundry.sipxphone.sys.util.* ;
import org.sipfoundry.util.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.sys.app.* ;

/**
 * The default dialing strategy simply issues a connect specified call.  If
 * no call is specified, the connection is issued on the infocus call.  If
 * no call is in focus, a call is created.
 */
public class DefaultDialingStrategy extends AbstractDialingStrategy
{
      /**
     * this remote address with the default local address or
     * invoke the strategic method/action
     */
    public void dial(PAddress remoteAddress)
    {
        dial( null, remoteAddress);
    }

    /**
     * this remote address with the specific local address or
     * invoke the strategic method/action
     */
    public void dial(PAddress localAddress, PAddress remoteAddress)
    {
        PCall call = Shell.getInstance().getDialingStrategyCall() ;
        if (call == null)
            call = Shell.getCallManager().getInFocusCall() ;

        dial(localAddress, remoteAddress, call) ;
    }


     /**
     * this remote address with the default local address or
     * invoke the strategic method/action on a specific call
     */
    public void dial(PAddress remoteAddress, PCall call)
    {
        dial( null, remoteAddress, call);
    }


    /**
    * this remote address with the specific local address or
    * invoke the strategic method/action on a specific call.
    */
    public void dial(PAddress localAddress, PAddress remoteAddress, PCall call)
    {
        if (call == null)
        {
            try
            {
                Shell.getMediaManager().setDefaultAudioDevice() ;
            }
            catch (PMediaException e)
            {
                e.printStackTrace() ;
            }
            try
            {
                call = PCall.createCall() ;
            }
            catch (PCallException e)
            {
                e.printStackTrace() ;

                MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
                messageBox.setMessage(messageBox.getString("lblErrorMaxConnections")) ;
                messageBox.showModal() ;
            }
        }

        if (call != null)
        {
            if (call.getConnectionState() == PCall.CONN_STATE_IDLE)
            {
                try
                {
                    call.connect(localAddress, remoteAddress) ;
                    fireDialingInitiated(call, remoteAddress) ;
                }
                catch (PCallResourceUnavailableException e)
                {
                    try
                    {
                        call.disconnect() ;
                    }
                    catch (Exception ee)
                    {
                        ee.printStackTrace() ;
                    }
                    abort() ;

                    MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
                    messageBox.setMessage(messageBox.getString("lblErrorMaxConnections")) ;
                    messageBox.showModal() ;
                }
                catch (PCallAddressException cae)
                {
                    try
                    {
                        call.disconnect() ;
                    }
                    catch (Exception ee)
                    {
                        ee.printStackTrace() ;
                    }

                    abort() ;
                    MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
                    messageBox.setMessage(cae.getMessage()) ;
                    messageBox.showModal() ;
                }
                catch (Exception e)
                {
                    try
                    {
                        call.disconnect() ;
                    }
                    catch (Exception ee)
                    {
                        ee.printStackTrace() ;
                    }

                    abort() ;

                    MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
                    messageBox.setMessage(messageBox.getString("lblErrorTextUnhandledException") + e.getMessage()) ;
                    messageBox.showModal() ;
                }
            }
            else
            {
                DTMFPlayer.playDTMFString(call, remoteAddress.getAddress()) ;
            }
        }
    }


    /**
     * Gets the string based representation of this strategy.  This text is
     * typically prepended the the form name.
     */
    public String getFunction()
    {
        return null ;
    }


    /**
     * Gets the string based representation of the dialing strategy action.
     * This text is typically displayed as the B3 button label
     */
    public String getAction()
    {
        return "Dial" ;
    }


    /**
     * Get the string based instructions for this strategy.  This text is
     * typically displayed in lower half of the "dialer" form.
     */
    public String getInstructions()
    {
        return null ;
    }


    /**
     * Gets the hint text associatd with the dialing strategy action.  This
     * hint text is typically displayed when the B3 buttons is pressed and
     * held down.
     */
    public String getActionHint()
    {
        return AppResourceManager.getInstance().getString("hint/core/dial_strategy/dial") ;
    }


    /**
     * Get the cancel status for this dialing strategy.
     *
     * @return boolean true if this operation/strategy can be canceled,
     *         otherwise false
     */
    public boolean isCancelable()
    {
        return false ;
    }


    /**
     * Evaluates if this stategy supports a changeable user.  For example, a
     * new call can be placed as any valid user, however, a specific call leg
     * (as in conference), cannot.  Similary, once could not transfer a call
     * as another user.
     *
     * @return boolean true if this dialing stragegy supports changeable users.
     */
    public boolean isOutboundUserChangeable()
    {
        return true ;
    }
}
