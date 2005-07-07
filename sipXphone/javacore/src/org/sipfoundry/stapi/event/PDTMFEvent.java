/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/stapi/event/PDTMFEvent.java#2 $
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

import org.sipfoundry.stapi.*;

/**
 * The PDTMFEvent class is delievered to a PDTMFListener whenever a remote
 * party generates out-of-band DTMF (Dual Tone Multiple Frequency) according
 * to RFC 2833.
 *
 * The PDTMFEvent class contains information about the DTMF event:
 * <ul>
 *   <li>Whether the button was pressed or released</li>
 *   <li>Which button was pressed or released</li>
 *   <li>How long the button was held before being released</li>
 *   <li>The source of the button event (which remote party)</li>
 * </ul>
 *
 * @see PCall
 * @see PDTMFListener
 *
 * @author Pradeep Paudyal
 */

public class PDTMFEvent
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** The button pressed. The events are defined in PCall.*/
    private int m_button;

    /** The type of button event, ie TYPE_BUTTON_UP or TYPE_BUTTON_DOWN. */
    private int m_type;

    /** The length the button was held down if it is a TYPE_BUTTON_UP event,
     *  or 0 if TYPE_BUTTON_DOWN.
     */
    private int m_duration;

    /** DTMF tone originator address. */
    private PAddress m_address;

//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /**
     * Button event type that identifies a button being pressed down.
     */
    public static final int TYPE_BUTTON_DOWN = 1;


    /**
     * Button event type that identifies a button being released.
     */
    public static final int TYPE_BUTTON_UP   = 0;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Private constructor- PDTMFEvents are not constructed by users.
     */
    protected PDTMFEvent()
    {
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Constructor
     *
     * @param iButton The DTMF tone ID of the button that was pressed to generate the DTMF event.
     *                Please see the DTMF tone IDs in PCall.
     * @param iType  The type of event: is it a TYPE_BUTTON_UP or a TYPE_BUTTON_DOWN?
     * @param iDuration The duration of the event in
     *                  milliseconds. For TYPE_BUTTON_DOWN events, 0.
     * @param address The address of the originating phone that is playing the DTMF tone.
     *
     * @deprecated DO NOT EXPOSE
     */
    public PDTMFEvent(int iButton,
                         int iType,
                         int iDuration,
                         PAddress address)
    {
        if( iButton < PCall.DTMF_0 || iButton > PCall.DTMF_FLASH )
        {
            throw new IllegalArgumentException("Invalid button value "+ iButton +" passed. ");
        }

        m_button   = iButton;
        m_type     = iType;
        m_duration = iDuration;
        m_address  = address;
    }


    /**
     * Get the DTMF tone ID of the button that was pressed or released. This ID
     * is identified by DTMF constants in PCall. For example, if the remote
     * party presses down the "1" key, getButton() will return PCall.DTMF_1
     *
     * @return Identifier of button that was pressed or released.
     *
     * @see PCall
     */
    public int getButton()
    {
        return m_button;
    }


    /**
     * Get the event type, where the event type is either:
     * <UL>
     * <li>TYPE_BUTTON_DOWN if the button was pressed
     * <li>TYPE_BUTTON_UP if the button was released
     * </ul>
     *
     * @return Either TYPE_BUTTON_UP or TYPE_BUTTON_DOWN depending on the event.
     */
    public int getType()
    {
        return m_type;
    }


    /**
     * Get the duration of a TYPE_BUTTON_UP event in milliseconds.
     *
     * @return The duration of this event in milliseconds.
     *
     * @throw IllegalStateException Thrown
     *        if this method is called when the event type is not TYPE_BUTTON_UP.
     */
    public int getDuration()
        throws IllegalStateException
    {
        // Make sure it is valid to return a duration.
        if (getButton() != TYPE_BUTTON_UP)
            throw new IllegalStateException("Cannot obtain duration") ;

        return m_duration;
    }


    /**
     * Get the originator of the DTMF event. This method is most useful when
     * in a multiparty conference.
     *
     * @return The address of the phone that generated this event.
     */
    public PAddress getAddress()
    {
        return m_address;
    }


    /**
     * @deprecated DO NOT EXPOSE
     */
    public String toString(){
        StringBuffer buffer = new StringBuffer();
        buffer.append("DTMF button is " + (char)m_button   );
        buffer.append(" .\n");
        buffer.append("duration    is " + m_duration );
        buffer.append(" .\n");
        buffer.append("address     is " + m_address  );
        buffer.append(" .\n");
        return buffer.toString();
    }
}
