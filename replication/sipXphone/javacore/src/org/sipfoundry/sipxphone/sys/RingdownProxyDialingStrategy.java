/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/sys/RingdownProxyDialingStrategy.java#2 $
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
 * The Ringdown proxy dialing strategy first rings down to the a
 * configured address and then plays DTMF for the dialed digits.
 */
public class RingdownProxyDialingStrategy extends AbstractDialingStrategy
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected PAddress m_addrRingdown ; // Ringdown address to callsfs


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    RingdownProxyDialingStrategy(String strRingdownAddress)
        throws PCallAddressException
    {
        if (strRingdownAddress == null)
        {
            throw new PCallAddressException("null address") ;
        }
        else
            m_addrRingdown = PAddressFactory.getInstance().createAddress(strRingdownAddress) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Method
////
    /**
     * 'Dial' or invoke the method
     */
    public void dial(PAddress address)
    {
        PCall call = Shell.getInstance().getDialingStrategyCall() ;
        if (call == null)
            call = Shell.getCallManager().getInFocusCall() ;

        dial(address, call) ;
    }


    /**
     * 'Dial' or invoke the method
     */
    public void dial(PAddress address, PCall call)
    {
        PMediaManager mediaManager = Shell.getMediaManager() ;

        if (call == null)
        {
            try
            {
                mediaManager.setDefaultAudioDevice() ;
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
                PhoneNumberParser parser = new PhoneNumberParser() ;
                parser.addDialString(address.getAddress()) ;
                parser.addEnableDevice(mediaManager.getAudioDeviceID()) ;


                SecondStageDialer dialer = new SecondStageDialer(call, parser) ;
                dialer.setSilentDTMF(true) ;

                try
                {
                    mediaManager.setAudioDevice(PAudioDevice.ID_NONE) ;
                    call.setCallData("AUTO_ENABLE_DEVICE", new Boolean(false)) ;
                    call.connect(m_addrRingdown) ;
                    fireDialingInitiated(call, address) ;
                }
                catch (PCallResourceUnavailableException e)
                {
                    abort() ;

                    try
                    {
                        call.disconnect() ;
                    }
                    catch (Exception ee)
                    {
                        ee.printStackTrace() ;
                    }

                    MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
                    messageBox.setMessage(messageBox.getString("lblErrorMaxConnections")) ;
                    messageBox.showModal() ;
                }
                catch (PCallAddressException cae)
                {
                    abort() ;

                    try
                    {
                        call.disconnect() ;
                    }
                    catch (Exception ee)
                    {
                        ee.printStackTrace() ;
                    }

                    MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
                    messageBox.setMessage(cae.getMessage()) ;
                    messageBox.showModal() ;
                }
                catch (Exception e)
                {
                    abort() ;

                    try
                    {
                        call.disconnect() ;
                    }
                    catch (Exception ee)
                    {
                        ee.printStackTrace() ;
                    }

                    MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
                    messageBox.setMessage(messageBox.getString("lblErrorTextUnhandledException") + e.getMessage()) ;
                    messageBox.showModal() ;
                }
            }
            else
            {
                DTMFPlayer.playDTMFString(call, address.getAddress()) ;
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
