/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/xpressa/app/transfer/TransferUtils.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sipxphone.app.transfer ;

import org.sipfoundry.stapi.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.sys.Shell ;
import org.sipfoundry.sipxphone.sys.Settings ;
import java.net.MalformedURLException ;
import org.sipfoundry.telephony.PtTerminalConnection ;
import org.sipfoundry.util.SysLog;

/**
 * This class is intended to provide helper/convenience methods for functions/
 * methods that are common across consultative and blind transfer.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class TransferUtils
{
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    private TransferUtils()
    {
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Report that the transfer has failed report the ability to recover the
     * original call.
     */
    static public void reportTransferFailure(Application app)
    {
        System.out.println("reportTransferFailure") ;

        MessageBox msgBox = new MessageBox(app, MessageBox.TYPE_INFORMATIONAL) ;

        msgBox.setTitle("Transfer Failed") ;
        msgBox.setMessage("Your transfer cannot be completed. Please press Ok to resume your original call.") ;

        msgBox.showModal() ;
    }


    /**
     * Report that the transferee has hungup.
     */
    static public void reportTransfereeHungup(Application app)
    {
        System.out.println("reportTransfereeHungup") ;

        MessageBox msgBox = new MessageBox(app, MessageBox.TYPE_INFORMATIONAL) ;

        msgBox.setTitle("Transfer Status") ;
        msgBox.setMessage("Transfer cannot be completed. The other call has been disconnected. Press Ok to resume your call.") ;

        msgBox.showModal() ;
    }


    /**
     * Report that the transfer target has hung up.
     */
    static public void reportTransferTargetHungup(Application app)
    {
        System.out.println("reportTransferTargetHungup") ;

        MessageBox msgBox = new MessageBox(app, MessageBox.TYPE_INFORMATIONAL) ;

        msgBox.setTitle("Transfer Status") ;
        msgBox.setMessage("Transfer cannot be completed. The destination call has been disconnected. Press Ok to resume your original call.") ;

        msgBox.showModal() ;
    }


    static public void recoverBlindTransferee(PCall call)
    {
        if (call != null)
        {
            // remove and transfer markings...
            unmarkBlindTransferCall(call) ;

            // Determine if a call is in focus, if so, push it aside.
            PCall callInFocus = Shell.getCallManager().getInFocusCall() ;
            if ((callInFocus != call) && callInFocus != null)
            {
                try
                {
                    callInFocus.placeOnHold() ;
                }
                catch (PCallException e)
                {
                    SysLog.log(e) ;
                }
            }

            // Push this call into focus
            try
            {
                call.stopTone() ;
                call.releaseFromHold() ;
                Shell.getMediaManager().setDefaultAudioDevice() ;
            }
            catch (PCallException e)
            {
                SysLog.log(e) ;
            }
            catch (PMediaException e)
            {
                SysLog.log(e) ;
            }

            // Lastly, tell the core app to monitor the call
            Shell.getCallManager().monitorCall(call) ;
        }
    }


    /**
     * This method clears and data stashed away in the call, disables any magic
     * flags,
     */
    static public void unmarkBlindTransferCall(PCall call)
    {
        if (call != null)
        {
            call.ignoreExternalDrops(false) ;
            call.setAutoDrop(true) ;
            call.setHangupHandler(null) ;
            call.m_bTransferControllerRelated = false ;
            call.m_bTransferControllerOriginalCall = false ;
            call.m_bTransferControllerMonitorCall = false ;
        }
    }


    /**
     * Utility method that drops a blind transfer related call.  This method
     * clears and data stashed away in the call, disables any magic flags, and
     * drops the call.  No attempt is made to monitor the call if the call was
     * previously ignored.
     */
    static public void dropBlindTransferCall(PCall call)
    {
        if (call != null)
        {
            unmarkBlindTransferCall(call) ;

            try
            {
                call.disconnect() ;
            }
            catch (PSTAPIException e)
            {
                SysLog.log(e) ;
            }
        }
    }


    /**
     * If the call is in focus, it is removed and audio devices disabled.
     */
    static public void removeCallFromFocus(PCall call)
    {
        if ((call != null) && (Shell.getCallManager().getInFocusCall() == call))
        {
            try
            {
                call.placeOnHold() ;
                Shell.getMediaManager().setAudioDevice(PAudioDevice.ID_NONE) ;
            }
            catch (PSTAPIException e)
            {
                SysLog.log(e) ;
            }

        }
    }




    /**
     *  Utility method that resumes the transferee call that was previously
     *  part of blind transfer.  This method clears all data stashed away
     *  in the call, disables any magic flags, takes the call off hold,
     *  and issues a monitor call.
     */
    static public void resumeBlindTransferCall(PCall call)
    {
        System.out.println("TransferUtils::resumeBlindTransferCall call=" + call.getCallID())  ;

        if (call != null)
        {
            TransferUtils.unmarkBlindTransferCall(call) ;

            try
            {
                call.stopTone() ;
                call.releaseFromHold() ;
                Shell.getMediaManager().setDefaultAudioDevice() ;
                Shell.getCallManager().showCall(call) ;
                Shell.getCallManager().monitorCall(call) ;
            }
            catch (PSTAPIException e)
            {
                SysLog.log(e) ;
            }
        }

        Shell.getMediaManager().stopTone() ;
    }


    /**
     * This method is invoked to play a audible called failed message after a
     * transfer attempted has failed.  This may happen many 10 of seconds after
     * the user has dismissed the transfer status.  This assumes that some
     * GUI notification has also been displayed (playing call waiting without
     * a visual clue is useless).
     */
    static public void playFailedCallNotification(PCall call)
    {
        System.out.println("playFailedCallNotification: " + call) ;

        if (call != null)
        {
            // If another call is in focus, alter the user via a call waiting beep.
            PCall callInFocus = Shell.getCallManager().getInFocusCall() ;
            if ((callInFocus != call) && (callInFocus != null))
            {
                try
                {
                    int iPlayLength = Settings.getInt("CALL_WAITING_TONE_LENGTH", 200) ;
                    callInFocus.playTone(PtTerminalConnection.DTMF_TONE_CALLWAITING, iPlayLength) ;
                } catch (PCallException e)
                {
                    SysLog.log(e) ;
                }
            }
            else
            {
                // If no call is in focus, then place are call into focus.
                if (callInFocus == null)
                {
                    try
                    {
                        call.releaseFromHold() ;
                    }
                    catch (PSTAPIException e)
                    {
                        SysLog.log(e) ;
                    }
                }

                // Last play failed call tone...
                try
                {
                    PhoneHelper.getInstance().getAudioSourceControl().enableRinger(true) ;
                    call.playTone(PtTerminalConnection.DTMF_TONE_CALLFAILED, true, false) ;
                }
                catch (PSTAPIException e)
                {
                    SysLog.log(e) ;
                }
            }
        }
        else
        {
            try
            {
                Shell.getMediaManager().playTone(PtTerminalConnection.DTMF_TONE_CALLFAILED) ;
            }
            catch (PSTAPIException e)
            {
                SysLog.log(e) ;
            }
        }
    }



    /**
     * Initiates a blind transfer
     *
    static public BlindTransferMonitor doBlindTransfer(PCall       call,
                                                       PAddress    addressTarget,
                                                       TransferControllerListener listener)
        throws PSTAPIException, MalformedURLException
    {
        PAddress addressTransferee = null ;

        System.out.println("doBlindTransfer:") ;
        System.out.println("  call=" + call) ;
        System.out.println("  addr=" + addressTarget);

        // The original call should be ignored from this point forward.
        Shell.getCallManager().ignoreCall(call) ;
        //call.m_bTransferControllerRelated = true ;
        //call.m_bTransferControllerOriginalCall = true ;

        // Figure out the transferee
        PAddress participants[] = call.getParticipants() ;
        if (participants.length > 0)
        {
            addressTransferee = participants[0] ;
        }

        // Do the transfer
        PCall callMonitor = call.transfer(addressTarget) ;
        //callMonitor.m_bTransferControllerMonitorCall = true ;

        // Kick off a monitor to watch the transfer
        BlindTransferMonitor monitor = new BlindTransferMonitor(call, callMonitor) ;
        if (listener != null)
            monitor.addTransferListener(listener) ;
        monitor.setTransfereeAddress(addressTransferee) ;
        TransferStatusIndicator indicator = new TransferStatusIndicator(addressTransferee, addressTarget) ;
        monitor.addTransferListener(indicator) ;


        // Ignore the newly created call.
        Shell.getCallManager().ignoreCall(callMonitor) ;
        //callMonitor.m_bTransferControllerRelated = true ;
        //callMonitor.ignoreExternalDrops(true) ;

        return monitor ;
    }
    */


    /**
     * Initiate a consultative transfer
     *
    static public ConsultativeTransferMonitor setupConsultativeTransfer(PCall callTransferee,
                                                                        TransferControllerListener listener)
    {
        System.out.println("setupConsultativeTransfer:") ;
        System.out.println("  call=" + callTransferee) ;

        // Create the consulative call
        PCall callTransferTarget ;
        try
        {
            callTransferTarget = PCall.createCall() ;
        }
        catch (Exception e)
        {
            Shell.getInstance().showUnhandledException(e, false) ;
            return null ;
        }

        // Create the monitor
        ConsultativeTransferMonitor monitor = new ConsultativeTransferMonitor(
                callTransferee, callTransferTarget) ;
        PAddress addresses[] = callTransferee.getParticipants() ;
        if (addresses.length > 0)
        {
            monitor.setTransfereeAddress(addresses[0]) ;
        }

        if (listener != null)
            monitor.addTransferListener(listener) ;

        return monitor ;
    }
    */
}
