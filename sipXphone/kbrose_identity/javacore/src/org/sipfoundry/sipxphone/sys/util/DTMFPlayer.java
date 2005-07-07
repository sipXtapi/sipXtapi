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


package org.sipfoundry.sipxphone.sys.util ;

import org.sipfoundry.stapi.* ;
import org.sipfoundry.util.SysLog;

/**
 * Simple utility class that can play a string of DTMF tones.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class DTMFPlayer
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** length of time that a DTMF tone will be played for in ms */
    protected static final int PLAY_TONE_LENGTH_MS    = 100 ;
    /** length of time between DTMF tones in ms */
    protected static final int BETWEEN_TONE_LENGTH_MS = 100 ;

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Play a string of DTMF tones to the specified call.
     *
     * @param call The call that will receive DTMF tones
     * @param strDTMF The source string containing DTMF digits
     *        ['0'-'9','*','#'] to be played.
     */
    public static void playDTMFString(PCall call, String strDTMF)
    {
        int iLength = strDTMF.length() ;
        char ch[] = new char[iLength] ;
        strDTMF.getChars(0, iLength, ch, 0) ;

        for (int i=0; i<iLength; i++) {
            playDTMFTone(call, ch[i]) ;
        }
    }


    /**
     * Play a single DTMF tone to the specified call.
     *
     * @param call The call that will receive DTMF tones
     * @param c The source char containing DTMF digits  ['0'-'9','*','#']
     *          to be played.
     */
    public static void playDTMFTone(PCall call, char c)
    {
        playDTMFTone(call, c, true, true) ;
    }


    /**
     * Play a single DTMF tone to the specified call.
     *
     * @param call The call that will receive DTMF tones
     * @param c The source char containing DTMF digits  ['0'-'9','*','#']
     *          to be played.
     * @param bLocal Should the tone be sounded locally?
     * @param bRemote Should the tone be transmitted remotely?
     */
    public static void playDTMFTone(PCall call, char c, boolean bLocal, boolean bRemote)
    {
        boolean bValidChar = true ;

        try {
            switch (c)
            {
                case '0':   call.playTone(PCall.DTMF_0, bLocal, bRemote) ;       break ;
                case '1':   call.playTone(PCall.DTMF_1, bLocal, bRemote) ;       break ;
                case '2':   call.playTone(PCall.DTMF_2, bLocal, bRemote) ;       break ;
                case '3':   call.playTone(PCall.DTMF_3, bLocal, bRemote) ;       break ;
                case '4':   call.playTone(PCall.DTMF_4, bLocal, bRemote) ;       break ;
                case '5':   call.playTone(PCall.DTMF_5, bLocal, bRemote) ;       break ;
                case '6':   call.playTone(PCall.DTMF_6, bLocal, bRemote) ;       break ;
                case '7':   call.playTone(PCall.DTMF_7, bLocal, bRemote) ;       break ;
                case '8':   call.playTone(PCall.DTMF_8, bLocal, bRemote) ;       break ;
                case '9':   call.playTone(PCall.DTMF_9, bLocal, bRemote) ;       break ;
                case '*':   call.playTone(PCall.DTMF_STAR, bLocal, bRemote) ;    break ;
                case '#':   call.playTone(PCall.DTMF_POUND, bLocal, bRemote) ;   break ;
                default:
                    bValidChar = false ;
                    break ;
            }
        } catch (PSTAPIException e) {
            SysLog.log(e) ;
        }

        // Only block / wait if we proccessed a valid tone
        if (bValidChar) {
            try {
                Thread.currentThread().sleep(PLAY_TONE_LENGTH_MS) ;
            } catch (InterruptedException ie) { }
            try {
                call.stopTone() ;
            } catch (PSTAPIException e) {
                SysLog.log(e) ;
            }
            try {
                Thread.currentThread().sleep(BETWEEN_TONE_LENGTH_MS) ;
            } catch (InterruptedException ie) { }
        }
    }
}
