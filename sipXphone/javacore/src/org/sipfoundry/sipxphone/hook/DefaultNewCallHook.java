/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/hook/DefaultNewCallHook.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sipxphone.hook;

import org.sipfoundry.util.SysLog;
import org.sipfoundry.stapi.* ;
import org.sipfoundry.stapi.event.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.sys.app.* ;

import org.sipfoundry.telephony.PtTerminalConnection ;
import org.sipfoundry.sipxphone.sys.appclassloader.ApplicationManager ;
import org.sipfoundry.util.AppResourceManager ;
import org.sipfoundry.util.PingerConfig ;
import org.sipfoundry.sipxphone.awt.form.MessageBox ;

/**
 * The New Call hook controls what happens when a new call is created by
 * pressing the speaker phone button, pressing the headset button,
 * lifting the handset, or pressing the new call button.
 * <p>
 * The class implemnents the default implementation: Launch the dialer (if no
 * dialing applications are being displayed) and start sounding dial tone.
 */
public class DefaultNewCallHook implements Hook
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected String m_strRingdownAddress ; // Configured Ringdown address

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public DefaultNewCallHook()
    {
        String strRingdownMethod = PingerConfig.getInstance().getValue(PingerConfig.PHONESET_RINGDOWN) ;

        m_strRingdownAddress = null ;
        if ((strRingdownMethod != null) && strRingdownMethod.equalsIgnoreCase("IMMEDIATE"))
        {
            m_strRingdownAddress = PingerConfig.getInstance().getValue(PingerConfig.PHONESET_RINGDOWN_ADDRESS) ;
            if (m_strRingdownAddress != null)
            {
                m_strRingdownAddress = m_strRingdownAddress.trim();
                if (m_strRingdownAddress.length() == 0)
                    m_strRingdownAddress = null ;
            }
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Hook interface method invoked as part of the hook chain.
     */
    public void hookAction(HookData data)
    {
        NewCallHookData hookData = (NewCallHookData) data ;

        if (m_strRingdownAddress == null)
        {
            doLaunchDialer(hookData) ;
        }
        else
        {
            doRingdown(m_strRingdownAddress, hookData) ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    protected void doRingdown(String strAddress, NewCallHookData hookData)
    {
        try
        {
            // Create a new call
            PCall call = PCall.createCall() ;

            // Enable an audio device
            try
            {
                if (hookData.getAudioDeviceID() == PAudioDevice.ID_NONE)
                    Shell.getMediaManager().setDefaultAudioDevice() ;
                else
                    Shell.getMediaManager().setAudioDevice(hookData.getAudioDeviceID()) ;
            }
            catch (PMediaException e)
            {
                e.printStackTrace() ;
            }

            // Initiate a call
            try
            {
                PAddressFactory factory = PAddressFactory.getInstance() ;
                call.connect(factory.createAddress(m_strRingdownAddress)) ;
            }
            catch (PSTAPIException e)
            {
                e.printStackTrace() ;
                displayCannotConnectCallError() ;
            }

        }
        catch (PCallException e)
        {
            e.printStackTrace() ;
            displayCannotCreateCallError() ;
        }
    }


    protected void doLaunchDialer(NewCallHookData hookData)
    {
         try
        {
            PCall call = PCall.createCall() ;
            try
            {
                // Enable the appropriate audio device
                if (hookData.getAudioDeviceID() == PAudioDevice.ID_NONE)
                    Shell.getMediaManager().setDefaultAudioDevice() ;
                else
                    Shell.getMediaManager().setAudioDevice(hookData.getAudioDeviceID()) ;
            }
            catch (PMediaException e)
            {
                SysLog.log(e) ;
            }

            ApplicationManager appManager = ApplicationManager.getInstance() ;

            if (    (!appManager.isCoreApplicationInFocus(ApplicationRegistry.SPEED_DIAL_APP)) &&
                    (!appManager.isCoreApplicationInFocus(ApplicationRegistry.CALL_LOG_APP)) &&
                    (!appManager.isCoreApplicationInFocus(ApplicationRegistry.DIAL_BY_URL_APP)))
            {
                ShellApp.getInstance().getCoreApp().doDisplayDialer(null) ;
            }

            try
            {
                call.playTone(PtTerminalConnection.DTMF_TONE_DIALTONE, true, false) ;
            }
            catch (PSTAPIException e)
            {
                SysLog.log(e) ;
            }

        }
        catch (PCallException e)
        {
            SysLog.log(e) ;
            displayCannotCreateCallError() ;
        }
    }


    protected void displayCannotCreateCallError()
    {
        String msg = AppResourceManager.getInstance().getString("lblErrorMaxConnections") ;

        MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
        messageBox.setMessage(msg) ;
        messageBox.showModeless() ;
    }


    protected void displayCannotConnectCallError()
    {
        String msg = AppResourceManager.getInstance().getString("lblErrorRingdownError") ;

        MessageBox messageBox = new MessageBox(ShellApp.getInstance().getCoreApp(), MessageBox.TYPE_ERROR) ;
        messageBox.setMessage(msg) ;
        messageBox.showModal() ;
    }


}
