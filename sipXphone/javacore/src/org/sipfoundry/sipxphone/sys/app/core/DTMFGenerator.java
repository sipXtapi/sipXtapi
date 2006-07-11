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


package org.sipfoundry.sipxphone.sys.app.core ;

import javax.telephony.* ;

import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.util.* ;

import org.sipfoundry.telephony.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.sys.* ;
import org.sipfoundry.sipxphone.service.* ;

import org.sipfoundry.stapi.* ;


/**
 * The DTMF Generator is a button listener that sifts through the event stream
 * looking for button presses.  While the button is being pressed, the
 * appropriate DTMF tone is played.  The button presses will be consumed.
 * <br><br>
 * To use, simply add an instance of this class as a button listener.
 *
 */
public class DTMFGenerator implements PButtonListener
{
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * The specified button has been pressed.
     */
    public void buttonDown(PButtonEvent event)
    {
       //org.sipfoundry.util.SysLog.debug(this, event.getButtonID()+" pressed down");
        switch (event.getButtonID()) {
            case PButtonEvent.BID_0:
            case PButtonEvent.BID_1:
            case PButtonEvent.BID_2:
            case PButtonEvent.BID_3:
            case PButtonEvent.BID_4:
            case PButtonEvent.BID_5:
            case PButtonEvent.BID_6:
            case PButtonEvent.BID_7:
            case PButtonEvent.BID_8:
            case PButtonEvent.BID_9:
                beginDTMFTone(event.getButtonID()) ;
                event.consume() ;
                break ;
            case PButtonEvent.BID_STAR:
                beginDTMFTone(event.getButtonID()) ;
                event.consume() ;
                break ;
            case PButtonEvent.BID_POUND:
                beginDTMFTone(event.getButtonID()) ;
                event.consume() ;
                break ;
        }
    }


    /**
     * The specified button has been released
     */
    public void buttonUp(PButtonEvent event)
    {
        //org.sipfoundry.util.SysLog.debug(this, event.getButtonID()+" pressed up");

        switch (event.getButtonID()) {
            case PButtonEvent.BID_0:
            case PButtonEvent.BID_1:
            case PButtonEvent.BID_2:
            case PButtonEvent.BID_3:
            case PButtonEvent.BID_4:
            case PButtonEvent.BID_5:
            case PButtonEvent.BID_6:
            case PButtonEvent.BID_7:
            case PButtonEvent.BID_8:
            case PButtonEvent.BID_9:
            case PButtonEvent.BID_STAR:
            case PButtonEvent.BID_POUND:
                endTone() ;
                event.consume() ;
                break ;
        }
    }


    /**
     * The specified button is being held down
     */
    public void buttonRepeat(PButtonEvent event)
    {

    }


    /**
     * Plays the specified DTMF tone to the specified call.
     *
     * @param iButtonID DTMF tone constant
     */
    public void beginDTMFTone(int iButtonID, PCall call)
    {
        if (call != null) {
            try {
                switch (iButtonID) {
                    case PButtonEvent.BID_0:
                        call.playTone(PCall.DTMF_0) ;
                        break ;
                    case PButtonEvent.BID_1:
                        call.playTone(PCall.DTMF_1) ;
                        break ;
                    case PButtonEvent.BID_2:
                        call.playTone(PCall.DTMF_2) ;
                        break ;
                    case PButtonEvent.BID_3:
                        call.playTone(PCall.DTMF_3) ;
                        break ;
                    case PButtonEvent.BID_4:
                        call.playTone(PCall.DTMF_4) ;
                        break ;
                    case PButtonEvent.BID_5:
                        call.playTone(PCall.DTMF_5) ;
                        break ;
                    case PButtonEvent.BID_6:
                        call.playTone(PCall.DTMF_6) ;
                        break ;
                    case PButtonEvent.BID_7:
                        call.playTone(PCall.DTMF_7) ;
                        break ;
                    case PButtonEvent.BID_8:
                        call.playTone(PCall.DTMF_8) ;
                        break ;
                    case PButtonEvent.BID_9:
                        call.playTone(PCall.DTMF_9) ;
                        break ;
                    case PButtonEvent.BID_STAR:
                        call.playTone(PCall.DTMF_STAR) ;
                        break ;
                    case PButtonEvent.BID_POUND:
                        call.playTone(PCall.DTMF_POUND) ;
                        break ;
                }
            } catch (PSTAPIException e) {
                SysLog.log(e) ;
            }
        }
    }


    /**
     * Plays the specified DTMF tone to the in focus call.
     *
     * @param iButtonID DTMF tone constant
     */
    public void beginDTMFTone(int iButtonID)
    {
        beginDTMFTone(iButtonID, Shell.getInstance().getCallManager().getInFocusCall()) ;
    }


    /**
     * Stop playing any tones.
     */
    public void endTone(PCall call)
    {
        if (call != null) {
            try {
                call.stopTone() ;
            } catch (PSTAPIException e) {
                SysLog.log(e) ;
            }
        }
    }



    /**
     * Stop playing any tones.
     */
    public void endTone()
    {
        endTone(Shell.getInstance().getCallManager().getInFocusCall()) ;
    }
}
