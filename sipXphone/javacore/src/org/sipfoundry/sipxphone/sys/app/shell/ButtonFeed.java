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


package org.sipfoundry.sipxphone.sys.app.shell ;

import javax.telephony.* ;
import javax.telephony.phone.* ;

import org.sipfoundry.telephony.phone.* ;
import org.sipfoundry.telephony.phone.event.* ;

import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.service.* ;
import org.sipfoundry.sipxphone.service.Timer;
import org.sipfoundry.sipxphone.sys.*;
/**
 * Button Feed is a quick hack class that feeds of the JTAPI Terminal Component
 * Listener world to snag button events.  This translates the buttons events
 * and then dispatches them over the PingtelEventQueue (active application).
 * <br><br>
 * The ButtonFeed also has a special provision where it sends events to the
 * core application BEFORE anyone else.  This is sort of a hacky situation, however,
 * we want guarenteed control.
 *
 * @author Robert J. Andreasen, Jr.
 *         Daniel W. Winsor
 */
public class ButtonFeed extends PtTerminalComponentAdapter
{
    static boolean bButtonDown = false;
    static int prevButtonDownID = -1;
    int repeatFireCount = 1;
    static int buttonRepeatTime = 1000;
    static int buttonRepeatInterval = 200;

    /** place timer events get sent to when a button is held */
    protected icButtonRepeatDispatcher m_ButtonRepeatListener = new icButtonRepeatDispatcher() ;

    /**
     * constructor taking a button listener.  This button listener will be
     * notified all button events before the general public.
     */
    public ButtonFeed()
    {
        super(IGNORE_VOLUME_EVENTS | IGNORE_HOOKSWITCH_EVENTS) ;

        //get the defaults for initial repeat fire and repeat interval

        buttonRepeatTime = Settings.getInt("BUTTON_REPEAT_TIMEOUT",buttonRepeatTime);
        buttonRepeatInterval = Settings.getInt("BUTTON_REPEAT_INTERVAL",buttonRepeatInterval);
    }


    /**
     * called when a button was pressed down and is now released
     */
    public void phoneButtonUp(PtTerminalComponentEvent event)
    {
        String strButton = ((PtPhoneButton) event.getComponent()).getInfo() ;

        int iButtonID = translateButtonString(strButton) ;
        if (iButtonID != -1) {

            PButtonEvent newEvent ;

            if (    strButton.equals("BACKSPACE") ||
                    strButton.equals("HOME") ||
                    strButton.equals("END") ||
                    strButton.startsWith("KBD")) {
//                        System.out.println("Button Up: " + iButtonID);
                newEvent = new PButtonEvent(this, PButtonEvent.KEY_UP, iButtonID) ;
            } else {
                newEvent = new PButtonEvent(this, PButtonEvent.BUTTON_UP, iButtonID) ;
            }
            bButtonDown = false;

            //remove repeat listener
            Timer.getInstance().removeTimers(m_ButtonRepeatListener) ;
            repeatFireCount = 1; //repeat key is reset
            PingtelEventQueue.postEvent(newEvent) ;
        }
    }


    /**
     * called when a button is pressed down
     */
    public void phoneButtonDown(PtTerminalComponentEvent event)
    {

        String strButton = ((PtPhoneButton) event.getComponent()).getInfo() ;


        //if already down and not a backspace (we like to repeat those)
        int iButtonID = translateButtonString(strButton) ;

//        System.out.println("ButtonFeed: down "+iButtonID);

        //don't continue if we are still down and it's the same key.
        //it is possible with a keyboard to be down with one key and then receive another before
        //the first key goes up.
        if (bButtonDown && prevButtonDownID == iButtonID && !strButton.equals("BACKSPACE"))
            return;

        if (iButtonID != -1) {


            PButtonEvent newEvent;

            if (    strButton.equals("BACKSPACE") ||
                    strButton.equals("HOME") ||
                    strButton.equals("END") ||
                    strButton.startsWith("KBD")) {
//                        System.out.println("Button Down: " + iButtonID);
                newEvent = new PButtonEvent(this, PButtonEvent.KEY_DOWN, iButtonID) ;
            } else {
                newEvent = new PButtonEvent(this, PButtonEvent.BUTTON_DOWN, iButtonID) ;
            }

           //button has been hit, start a timer that goes of every n millisecs
            Timer.getInstance().addTimer(buttonRepeatTime, m_ButtonRepeatListener, newEvent, true) ;

            bButtonDown = true;
            repeatFireCount = 1; //repeat key is reset
            prevButtonDownID = iButtonID;
            PingtelEventQueue.postEvent(newEvent) ;
        }
    }



    /**
     * provides translation from JTAPI button string into a PButtonEvent
     */
    protected int translateButtonString(String strButton)
    {
        int iButtonID = -1 ;



        //handle the NT keyboard events
        if (strButton.startsWith("BACKSPACE"))
        {
            iButtonID = 8; //backspace character
        }
        else
        if (strButton.startsWith("HOME"))
        {
            iButtonID = PButtonEvent.BID_HOME; //home
        }
        else
        if (strButton.startsWith("END"))
        {
            iButtonID = PButtonEvent.BID_END; //end
        }
        else
        if (strButton.startsWith("KBD") && strButton.length() > 3)
        {
            iButtonID = (int)strButton.charAt(3);
        }
        else
        // Single key are relatively painless...
        if ( strButton.length() == 1) {
            switch (strButton.charAt(0)) {
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
                    iButtonID = (int) strButton.charAt(0) ;
                    break ;
                default:
                    System.out.println("Unknown/Expected Button: " + strButton) ;
            }
        }

        // We need to compare each string.  There are better ways to do this!
        else {

            //
            // OLD WORLD
            //

            if (strButton.equals("USER10")) {
                iButtonID = PButtonEvent.BID_L1 ;
            } else if (strButton.equals("USER4")) {
                iButtonID = PButtonEvent.BID_L2 ;
            } else if (strButton.equals("USER15")) {
                iButtonID = PButtonEvent.BID_L3 ;
            } else if (strButton.equals("USER9")) {
                iButtonID = PButtonEvent.BID_L4 ;

            } else if (strButton.equals("USER1")) {
                iButtonID = PButtonEvent.BID_R1 ;
            } else if (strButton.equals("USER7")) {
                iButtonID = PButtonEvent.BID_R2 ;
            } else if (strButton.equals("USER13")) {
                iButtonID = PButtonEvent.BID_R3 ;
            } else if (strButton.equals("USER2")) {
                iButtonID = PButtonEvent.BID_R4 ;

            } else if (strButton.equals("USER3")) {
                iButtonID = PButtonEvent.BID_B1 ;
            } else if (strButton.equals("USER14")) {
                iButtonID = PButtonEvent.BID_B2 ;
            } else if (strButton.equals("USER8")) {
                iButtonID = PButtonEvent.BID_B3 ;

            } else if (strButton.equals("USER11")) {
                iButtonID = PButtonEvent.BID_PINGTEL ;

            } else if (strButton.equals("USER16")) {
                iButtonID = PButtonEvent.BID_SCROLL_UP ;
            } else if (strButton.equals("USER5")) {
                iButtonID = PButtonEvent.BID_SCROLL_DOWN ;

            } else if (strButton.equals("REDIAL")) {
                iButtonID = PButtonEvent.BID_REDIAL ;
            } else if (strButton.equals("CONFERENCE")) {
                iButtonID = PButtonEvent.BID_CONFERENCE ;
            } else if (strButton.equals("TRANSFER")) {
                iButtonID = PButtonEvent.BID_TRANSFER ;
            } else if (strButton.equals("HOLD")) {
                iButtonID = PButtonEvent.BID_HOLD ;
            } else if (strButton.equals("HEADSET")) {
                iButtonID = PButtonEvent.BID_HEADSET ;
            }

            // NEW WORLD

            else if (strButton.equals("SKEYL1")) {
                iButtonID = PButtonEvent.BID_L1 ;
            } else if (strButton.equals("SKEYL2")) {
                iButtonID = PButtonEvent.BID_L2 ;
            } else if (strButton.equals("SKEYL3")) {
                iButtonID = PButtonEvent.BID_L3 ;
            } else if (strButton.equals("SKEYL4")) {
                iButtonID = PButtonEvent.BID_L4 ;

            } else if (strButton.equals("SKEYR1")) {
                iButtonID = PButtonEvent.BID_R1 ;
            } else if (strButton.equals("SKEYR2")) {
                iButtonID = PButtonEvent.BID_R2 ;
            } else if (strButton.equals("SKEYR3")) {
                iButtonID = PButtonEvent.BID_R3 ;
            } else if (strButton.equals("SKEYR4")) {
                iButtonID = PButtonEvent.BID_R4 ;

            } else if (strButton.equals("SKEYB1")) {
                iButtonID = PButtonEvent.BID_B1 ;
            } else if (strButton.equals("SKEYB2")) {
                iButtonID = PButtonEvent.BID_B2 ;
            } else if (strButton.equals("SKEYB3")) {
                iButtonID = PButtonEvent.BID_B3 ;

            } else if (strButton.equals("MORE")) {
                iButtonID = PButtonEvent.BID_PINGTEL ;

            } else if (strButton.equals("SCROLL_UP")) {
                iButtonID = PButtonEvent.BID_SCROLL_UP ;
            } else if (strButton.equals("SCROLL_DOWN")) {
                iButtonID = PButtonEvent.BID_SCROLL_DOWN ;

            } else if (strButton.equals("STAR")) {
                iButtonID = PButtonEvent.BID_STAR;
            } else if (strButton.equals("POUND")) {
                iButtonID = PButtonEvent.BID_POUND ;


            } else if (strButton.equals("VOL_UP")) {
                iButtonID = PButtonEvent.BID_VOLUME_UP ;
            } else if (strButton.equals("VOL_DOWN")) {
                iButtonID = PButtonEvent.BID_VOLUME_DN ;

// HACK: JPH volume hack
            } else if (strButton.equals("SKEY1")) {
                iButtonID = PButtonEvent.BID_VOLUME_UP ;
            } else if (strButton.equals("SKEY2")) {
                iButtonID = PButtonEvent.BID_VOLUME_DN ;

            } else if (strButton.equals("SPEAKER")) {
                iButtonID = PButtonEvent.BID_SPEAKER ;
            } else if (strButton.equals("MUTE")) {
                iButtonID = PButtonEvent.BID_MUTE ;

            } else {
                System.out.println("Unknown/Expected Button: " + strButton) ;
            }
        }
        return iButtonID ;
    }


    public class icButtonRepeatDispatcher implements PActionListener
    {

        public void actionEvent(PActionEvent event)
        {

            if (event.getActionCommand().equals(Timer.ACTION_TIMER_FIRED))
            {
                PButtonEvent buttonEvent = (PButtonEvent)event.getObjectParam();

                if (buttonEvent != null)
                {
                    PButtonEvent repeatButtonEvent = new PButtonEvent(this,buttonEvent.BUTTON_REPEAT,
                                            buttonEvent.getButtonID());
                    if (repeatFireCount == 1)
                    {
//                        System.out.println("adding faster timer " + repeatFireCount);
//                        System.out.println("value " + buttonRepeatInterval);
                        //add a new faster one
                        Timer.getInstance().resetTimer(buttonRepeatInterval, m_ButtonRepeatListener, repeatButtonEvent,true) ;
                    }
                    repeatFireCount++;

                    PingtelEventQueue.postEvent(repeatButtonEvent) ;
                }
            }
        }

    }
}
