/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/util/PhoneNumberParser.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.util ;

import java.util.Vector;
import java.lang.StringBuffer;


/**
 * Utility class that can parse a phone number and break out the main dialing
 * string from waits, pauses, and additional dialing strings.  The parser
 * pulls the main phone number out and then allows you to walk through all
 * of the actions, where the actions include WAIT, PAUSE, or DIAL.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PhoneNumberParser
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final int ACTION_INVALID  = -1;  // Unknown/Invalid action
    public static final int ACTION_WAIT  = 0 ;     // Wait for the call to connect
    public static final int ACTION_PAUSE = 1 ;     // Pause for .5 seconds
    public static final int ACTION_DIAL  = 2 ;     // Dial a substring by playing DTMF
    public static final int ACTION_SET_DEVICE = 3 ;// Enable auto device

    public static final char CHAR_PAUSE  = 'p' ;   // Char used to signal a pause
    public static final char CHAR_WAIT   = 'i' ;   // Char used to signal a wait, i is for interactive

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected StringBuffer m_sbNumber ;     // The first part of the phone no.
    protected Vector m_vActions ;           // List of pieces/parts/actions
    protected int m_iIndex ;                // Index into m_vActions

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Public constructor accepting the phone number
     *
     * @param strNumber The phone number to parse.  This value should not
     *        be null.
     */
    public PhoneNumberParser(String strNumber)
    {
        m_sbNumber = new StringBuffer() ;
        m_vActions = new Vector() ;
        m_iIndex = 0 ;

        // Parse the phone number
        if (strNumber != null)
            parseNumber(strNumber) ;
    }

    public PhoneNumberParser()
    {
        m_sbNumber = new StringBuffer() ;
        m_vActions = new Vector() ;
        m_iIndex= 0 ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Count actiion tokens
     */
    public int countActions()
    {
        return m_vActions.size();
    }


    /**
     * Get the first part of the phone number before any waits or pauses.
     */
    public String getNumber()
    {
        return m_sbNumber.toString();
    }


    /**
     * Are there any actions (wait, pause, or dials)
     */
    public boolean hasMoreActions()
    {
        return ((m_iIndex+1) < m_vActions.size()) ;
    }


    /**
     * Advance to the next action.
     *
     * @return true if there are more entries otherwise false
     */
    public boolean next()
    {
        boolean bSuccess = false ;

        if (hasMoreActions())
        {
            m_iIndex++ ;
            bSuccess = true ;
        }

        return bSuccess ;
    }


    /**
     * Get the current action code.
     */
    public int getAction()
    {
        int iAction = ACTION_INVALID ;

        if (m_iIndex < m_vActions.size())
        {
            icDialerAction action = (icDialerAction) m_vActions.elementAt(m_iIndex) ;
            iAction = action.m_iAction ;
        }

        return iAction ;
    }


    /**
     * Get the current action parameter/string.
     */
    public String getActionString()
    {
        String strParameter = null ;

        if (m_iIndex < m_vActions.size())
        {
            icDialerAction action = (icDialerAction) m_vActions.elementAt(m_iIndex) ;
            strParameter = action.m_sbParameter.toString() ;
        }

        return strParameter ;
    }


    /**
     * Get the current action parameter/int
     */
    public int getActionInt()
    {
        int iParameter = -1 ;

        if (m_iIndex < m_vActions.size())
        {
            icDialerAction action = (icDialerAction) m_vActions.elementAt(m_iIndex) ;
            iParameter = action.m_iParameter ;
        }

        return iParameter ;

    }


    public void addWait()
    {
        m_vActions.addElement(new icDialerAction(ACTION_WAIT)) ;
    }


    public void addPause()
    {
        m_vActions.addElement(new icDialerAction(ACTION_PAUSE)) ;
    }


    public void addDialString(String strDTMF)
    {
        icDialerAction dialAction = new icDialerAction(ACTION_DIAL) ;
        dialAction.m_sbParameter.append(strDTMF) ;

        m_vActions.addElement(dialAction) ;
    }


    public void addEnableDevice(int iDeviceID)
    {
        icDialerAction setDeviceAction = new icDialerAction(ACTION_SET_DEVICE) ;
        setDeviceAction.m_iParameter = iDeviceID ;

        m_vActions.addElement(setDeviceAction) ;
    }


    /**
     * Get parsed contents of the number in a string format.  This assumes
     * that next() has never been called!
     */
    public String toString()
    {
        StringBuffer buf = new StringBuffer() ;

        buf.append(m_sbNumber.toString()) ;
        for (int i=0; i<m_vActions.size(); i++)
        {
            icDialerAction action = (icDialerAction) m_vActions.elementAt(i) ;
            switch (action.m_iAction)
            {
                case ACTION_WAIT:
                    buf.append("[wait]") ;
                    break ;
                case ACTION_PAUSE:
                    buf.append("[pause]") ;
                    break ;
                case ACTION_DIAL:
                    buf.append(action.m_sbParameter.toString()) ;
                    break ;
                case ACTION_INVALID:
                    default:
                    buf.append("[??]") ;
                    break ;

            }
        }
        return buf.toString() ;
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    /**
     * Parse the specified phone number into actions
     */
    protected void parseNumber(String strNumber)
    {
        boolean bParsedNumber = false ;
        icDialerAction dialAction = new icDialerAction(ACTION_DIAL) ;

        for (int i=0; i<strNumber.length(); i++)
        {
            char ch = strNumber.charAt(i) ;
            if (ch == CHAR_PAUSE)
            {
                bParsedNumber = true ;
                if (dialAction.m_sbParameter.length() > 0)
                {
                    m_vActions.addElement(dialAction) ;
                    dialAction = new icDialerAction(ACTION_DIAL) ;
                }
                addPause() ;
            }
            else if (ch == CHAR_WAIT)
            {
                bParsedNumber = true ;
                if (dialAction.m_sbParameter.length() > 0)
                {
                    m_vActions.addElement(dialAction) ;
                    dialAction = new icDialerAction(ACTION_DIAL) ;
                }
                addWait() ;
            }
            else
            {
                if (bParsedNumber == false)
                {
                    m_sbNumber.append(ch) ;
                }
                else
                {
                    dialAction.m_sbParameter.append(ch) ;
                }
            }
        }

        if (dialAction.m_sbParameter.length() > 0)
        {
            m_vActions.addElement(dialAction) ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Inner/Nested Classes
////
    /**
     * Data encapsulation for a dialer action.
     */
    protected class icDialerAction
    {
        public int m_iAction ;
        public StringBuffer m_sbParameter ;
        public int m_iParameter ;

        public icDialerAction(int iAction)
        {
            m_iAction = iAction ;
            m_sbParameter = new StringBuffer() ;
            m_iParameter = -1 ;
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Testing
////
    /**
     * Testing/Debugging Main
     */
    public static void main(String args[])
    {
        if (args.length != 1)
        {
            System.out.println("Usage org.sipfoundry.util.PhoneNumberParser <number>") ;
        }
        else
        {
            PhoneNumberParser parser = new PhoneNumberParser(args[0]) ;
            System.out.println("Results: " + parser.toString()) ;
        }
    }
}
