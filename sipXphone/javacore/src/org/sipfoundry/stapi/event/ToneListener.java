/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/stapi/event/ToneListener.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */
 
 
package org.sipfoundry.stapi.event;

import org.sipfoundry.sipxphone.sys.Shell;
import org.sipfoundry.stapi.*;
import org.sipfoundry.util.SysLog;
/**
 * ToneListener.java
 *
 * This class is not to be exposed. It is used by the lower layer
 * to pass the DTMF events to the JAVA layer.
 *
 * Created: Mon Apr 08 14:21:24 2002
 *
 * @author Pradeep Paudyal
 * @version 1.0
 */


public class ToneListener {



    /** DTMF tone id: 0 key */
    protected static final int DTMF_0      = 0 ;
    /** DTMF tone id: 1 key */
    protected static final int DTMF_1      = 1 ;
    /** DTMF tone id: 2 key */
    protected static final int DTMF_2      = 2 ;
    /** DTMF tone id: 3 key */
    protected static final int DTMF_3      = 3 ;
    /** DTMF tone id: 4 key */
    protected static final int DTMF_4      = 4 ;
    /** DTMF tone id: 5 key */
    protected static final int DTMF_5      = 5 ;
    /** DTMF tone id: 6 key */
    protected static final int DTMF_6      = 6 ;
    /** DTMF tone id: 7 key */
    protected static final int DTMF_7      = 7 ;
    /** DTMF tone id: 8 key */
    protected static final int DTMF_8      = 8 ;
    /** DTMF tone id: 9 key */
    protected static final int DTMF_9      = 9 ;
    /** DTMF tone id: star (*) key */
    protected static final int DTMF_STAR   = 10 ;
    /** DTMF tone id: pound (#) key */
    protected static final int DTMF_POUND  = 11 ;
    /** DTMF tone id: A key */
    protected static final int DTMF_A      = 12 ;
    /** DTMF tone id: B key */
    protected static final int DTMF_B      = 13 ;
    /** DTMF tone id: C key */
    protected static final int DTMF_C      = 14 ;
    /** DTMF tone id: D key */
    protected static final int DTMF_D      = 15 ;
    /** DTMF tone id: FLASH key */
    protected static final int DTMF_FLASH  = 16 ;


     public void toneEvent( String strCallID, String strConnection,
                                                int iButton, int iDuration ){
            //fire the DTMF listeners.
         //SysLog.debug(this, "NEW TONE EVENT CALLED IN JAVA LAYER");

          if( strCallID != null ){
            PCall call = Shell.getCallManager().getCallByCallID( strCallID );
            PAddress address = new PAddress(strConnection);
            if( call != null && address != null){
                int type = PDTMFEvent.TYPE_BUTTON_DOWN;
                if( iDuration > 0 )
                    type = PDTMFEvent.TYPE_BUTTON_UP;
                try{
                    /*SysLog.debug(this, " iButton is " + iButton
                                        +"\n getPCallDTMFButtonID(iButton) is " + getPCallDTMFButtonID(iButton)
                                        +"\niDuration "+ iDuration );
                    */

                    call.postDTMFEvent
                        ( new PDTMFEvent( getPCallDTMFButtonID(iButton),
                                          type,
                                          iDuration,
                                          address) );
                }catch( Exception e ){
                    SysLog.log(e);
                }
            }
        }
    }


    /**
     * maps the button id from this class to that of
     * PCall.
     */
    private int getPCallDTMFButtonID( int buttonid ){
       int iRet = -1;
       switch( buttonid ){
            case DTMF_0:      iRet= PCall.DTMF_0; break;
            case DTMF_1:      iRet= PCall.DTMF_1; break;
            case DTMF_2:      iRet= PCall.DTMF_2; break;
            case DTMF_3:      iRet= PCall.DTMF_3; break;
            case DTMF_4:      iRet= PCall.DTMF_4; break;
            case DTMF_5:      iRet= PCall.DTMF_5; break;
            case DTMF_6:      iRet= PCall.DTMF_6; break;
            case DTMF_7:      iRet= PCall.DTMF_7; break;
            case DTMF_8:      iRet= PCall.DTMF_8; break;
            case DTMF_9:      iRet= PCall.DTMF_9; break;
            case DTMF_STAR:   iRet= PCall.DTMF_STAR; break;
            case DTMF_POUND:  iRet= PCall.DTMF_POUND; break;
            case DTMF_A:      iRet= PCall.DTMF_A; break;
            case DTMF_B:      iRet= PCall.DTMF_B; break;
            case DTMF_C:      iRet= PCall.DTMF_C; break;
            case DTMF_D:      iRet= PCall.DTMF_D; break;
            case DTMF_FLASH:  iRet= PCall.DTMF_FLASH; break;
       }
       return iRet;
    }


}// ToneListener
