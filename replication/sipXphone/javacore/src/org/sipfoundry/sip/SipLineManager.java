/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/sip/SipLineManager.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.sip ;

import java.util.* ;

import org.sipfoundry.util.SipParser ;
import org.sipfoundry.sip.event.* ;
import org.sipfoundry.stapi.PAddress ;
import org.sipfoundry.stapi.PAddressFactory ;

public class SipLineManager
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    private final String EXCEPTION_NULL_LINE = "line cannot be null" ;
    private final String EXCEPTION_NULL_URL  = "register url cannot be null" ;
    private final String EXCEPTION_NULL_USER = "user cannot be null" ;

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private static SipLineManager m_instance ;
    private static Hashtable      m_htLineData ;
    private static Vector         m_vLineCache ;
    private static SipLine        m_defaultOutboundLine ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Private constructor guards against rogue construction.  To obtain a
     * reference to the SipLineManager, please use SipLineManager.getInstance().
     *
     * @see getInstance()
     */
    private SipLineManager()
    {
        m_htLineData = new Hashtable() ;
        addLineListener(new icLineListener()) ;
        m_vLineCache = new Vector() ;
        populateLineCache() ;
        m_defaultOutboundLine = null ;
    }


    /**
     * Get a singleton instance to the Line Manager.
     *
     * @return a reference to the SipLineManager.
     */
    public static SipLineManager getInstance()
    {
        if (m_instance == null)
            m_instance = new SipLineManager() ;

        return m_instance ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Create a new line.  Lines are created in the DISABLED state, unless
     * provisioned or auto enabled is set.
     *
     * @param strRegisterUrl The url where the REGISTER message should be sent.
     * @param strUser The user identifing phone.
     * @param bProvisioned Is the line provisioned?  If true the line will
     *        represent an identity, however, will not send out a register
     *        message.
     * @param bShowLine Should the line definition be displayed in the user
     *        interface?  Setting to true will allow the end user to select
     *        this line as an outbound line.
     * @param bAutoEnable Should the line automatically enable on phone
     *        restart?  If set to true, the phone will automatically attempt to
     *        register both now and when the phone restarts.  If set to false,
     *        the user will need to manually enable/disable the line.
     * @param bUseCallHandlingSettings Should we use the appropriate call
     *        handling preferences for this line.
     */
    public SipLine createLine(String  strRegisterUrl, //user entered url
                              String  strUser,
                              boolean bProvisioned,
                              boolean bShowLine,
                              boolean bAutoEnable,
                              boolean bUseCallHandlingSettings)
    {
        SipLine lineNew = null ;
        int iLineState = SipLine.LINE_STATE_DISABLED ;

        // Sanity check for our parameters.
        if (strRegisterUrl == null)
            throw new IllegalArgumentException(EXCEPTION_NULL_URL) ;
        if (strUser == null)
            throw new IllegalArgumentException(EXCEPTION_NULL_USER) ;


        // If the line is being created as provisioned, then we must
        // mark the state.
        if (bProvisioned)
        {
            iLineState = SipLine.LINE_STATE_PROVISIONED ;
        }



        // Create the line
        String strIdentity = JNI_createLine(strRegisterUrl,  strUser,
                bShowLine, bAutoEnable, iLineState, bUseCallHandlingSettings) ;
        if (strIdentity != null)
        {
            lineNew = new SipLine(strIdentity,strRegisterUrl) ;
        }

        // If the bAutoEnable is set, then enable the line.
        if ((bAutoEnable ) && (lineNew != null))
        {
           lineNew.enable(true) ;
        }

        return lineNew ;
    }


    /**
     * Deletes the specified line.
     */
    public void deleteLine(SipLine lineDelete)
    {
        if (lineDelete == null)
            throw new IllegalArgumentException(EXCEPTION_NULL_LINE) ;

        JNI_deleteLine(lineDelete.getIdentity(), 0) ;
    }


    /**
     * Get the default outbound line.
     */
    public SipLine getDefaultLine()
    {
        if (m_defaultOutboundLine == null)
        {
            String strIdentity[] = JNI_getDefaultLine() ;
            if (strIdentity != null && strIdentity.length == 2)
            {
                m_defaultOutboundLine = new SipLine(strIdentity[1],strIdentity[0]) ;
            }
        }

        return m_defaultOutboundLine ;
    }


    /**
     * Set the default outbound line.
     */
    public void setDefaultLine(SipLine line)
    {
        if (line == null)
            throw new IllegalArgumentException(EXCEPTION_NULL_LINE) ;

        JNI_setDefaultLine(line.getUserEnteredUrl(), 0) ;
    }



    /**
     * Get all of the lines known to the line manager.
     *
     * @return All of the sip lines known to the line manager.
     */
    public SipLine[] getLines()
    {
        SipLine lines[] = null ;
        synchronized (m_vLineCache)
        {
            int iSize = m_vLineCache.size() ;
            lines = new SipLine[iSize] ;
            m_vLineCache.copyInto(lines) ;
        }
        return lines ;
    }

    /**
     * Gets the number of lines known to the line manager.
     * @return the number of lines known to the line manager.
     */
    public int getLinesCount()
    {
        int iRet = 0;
        SipLine[] lines = getLines();
        if( lines != null )
            iRet = lines.length;
        return iRet;
    }


    /**
     * Gets the line by session
     */
    public SipLine getLineBySession(SipSession session)
    {
        SipLine line = null ;

        if (session != null)
        {
            String strIdentifer = JNI_getLine(session.getRemoteURL(), session.getLocalContact()) ;
            if (strIdentifer != null)
            {
                line = new SipLine(strIdentifer,strIdentifer) ;
            }
        }
        return line ;
    }


    /**
     * Searches for and finds a specific line for the specified address.
     *
     * @return the line matching the supplied address or null if not found.
     */
    public SipLine getLineByAddress(PAddress address)
    {
        SipLine lineMatching = null ;

        if (address == null)
            throw new IllegalArgumentException() ;

        // Strip any tags
        String strAddress  = SipParser.stripParameters(address) ;
        try
        {
            address = PAddressFactory.getInstance().createAddress(strAddress) ;
        } catch (Exception e) { /* burp */ }

        // Find the line
        SipLine lines[] = getLines() ;
        if (lines != null)
        {
            for (int i=0; i<lines.length; i++)
            {
                String strIdentity = lines[i].getUserEnteredUrl() ;
                try
                {
                    PAddress addressLine = PAddressFactory.getInstance().createAddress(strIdentity) ;
                    if (address.equals(addressLine))
                    {
                        lineMatching = lines[i] ;
                        break ;
                    }

                }
                catch (Exception e)
                {
                    // eat exception and try next line
                }
            }
        }

        return lineMatching ;
    }


    /**
     * Adds a new line listener to the line manager.  Listeners are notified
     * of line state changes.
     */
    public void addLineListener(SipLineListener listener)
        throws IllegalArgumentException
    {
        if (listener == null)
            throw new IllegalArgumentException("cannot add null listener") ;

        JNI_addLineListener(listener, listener.hashCode()) ;
    }


    /**
     * Removes a line listener from the line manager.  That listener will
     * cease to receive line state change notifications.
     */
    public void removeLineListener(SipLineListener listener)
        throws IllegalArgumentException
    {
        if (listener == null)
            throw new IllegalArgumentException("cannot remove null listener") ;

        JNI_removeLineListener(listener, listener.hashCode()) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////

    /**
     * Sets a keyed piece of data for a specific line.
     */
    protected void setLineData(SipLine line, String strKey, Object objData)
    {
        Hashtable htData = (Hashtable) m_htLineData.get(line) ;
        if (htData == null)
            htData = new Hashtable() ;

        htData.put(strKey, objData) ;
        m_htLineData.put(line, htData) ;
    }


    /**
     * Gets a keyed piece of data for a specific line.
     */
    protected Object getLineData(SipLine line, String strKey)
    {
        Object objRC = null ;

        Hashtable htData = (Hashtable) m_htLineData.get(line) ;
        if (htData != null)
        {
            objRC = htData.get(strKey) ;
        }

        return objRC ;
    }


    /**
     * Clears all data for the supplied line.
     */
    protected void clearLineData(SipLine line)
    {
        m_htLineData.remove(line) ;
    }


    protected void populateLineCache()
    {
        synchronized (m_vLineCache)
        {
            m_vLineCache.removeAllElements() ;

            Object strIdentifiers[][] = JNI_getLines() ;

            if (strIdentifiers != null && strIdentifiers.length == 2 &&
                strIdentifiers[0].length == strIdentifiers[1].length)
            {
                //get user entered url's
                for (int i=0; i<strIdentifiers[0].length; i++)
                {
                    String strUserEnteredUrl = (String)strIdentifiers[0][i].toString();
                    String strIdentity = (String)strIdentifiers[1][i].toString();

                    SipLine line = new SipLine(strIdentity,strUserEnteredUrl) ;
                    m_vLineCache.addElement(line) ;
                }
            }

            String strIdentity[] = JNI_getDefaultLine() ;
            if (strIdentity != null && strIdentity.length == 2)
            {
                m_defaultOutboundLine = new SipLine(strIdentity[1],strIdentity[0]) ;
            }
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////

    private class icLineListener implements SipLineListener
    {
        public void lineEnabled(SipLineEvent event)
        {
            populateLineCache() ;

            clearLineFailureData(event.getLine()) ;
        }

        public void lineFailed(SipLineEvent event)
        {
            populateLineCache() ;

            int iResponse = event.getSipResponseCode() ;
            String strResponseText = event.getSipResponseText() ;
            String strRealm = event.getAuthenticationRealm() ;
            String strScheme = event.getAuthenticationScheme() ;

            setLineFailureData(event.getLine(), iResponse, strResponseText, strRealm, strScheme) ;
        }


        public void lineTimeout(SipLineEvent event)
        {
            populateLineCache() ;

            int iResponse = event.getSipResponseCode() ;
            String strRealm = event.getAuthenticationRealm() ;
            String strScheme = event.getAuthenticationScheme() ;
            String strResponseText = event.getSipResponseText() ;

            setLineFailureData(event.getLine(), iResponse, strResponseText, strRealm, strScheme) ;
        }

        public void lineAdded(SipLineEvent event)
        {
            populateLineCache() ;

            clearLineFailureData(event.getLine()) ;
        }

        public void lineDeleted(SipLineEvent event)
        {
            populateLineCache() ;

            clearLineFailureData(event.getLine()) ;
        }

        public void lineChanged(SipLineEvent event)
        {
            populateLineCache() ;

            // Changes don't necessarily imply a change in registration status
        }


        public void lineDefault(SipLineEvent event)
        {
            m_defaultOutboundLine = event.getLine() ;
        }

        private void setLineFailureData(SipLine line,
                                        int iCode,
                                        String strText,
                                        String strRealm,
                                        String strScheme)
        {
            SipLineManager manager = SipLineManager.getInstance() ;
            manager.setLineData(line, SipLine.DATA_STATUS_CODE, new Integer(iCode)) ;
            manager.setLineData(line, SipLine.DATA_STATUS_TEXT, strText) ;
            manager.setLineData(line, SipLine.DATA_AUTH_REALM, strRealm) ;
            manager.setLineData(line, SipLine.DATA_AUTH_SCHEME, strScheme) ;
        }

        private void clearLineFailureData(SipLine line)
        {
            SipLineManager manager = SipLineManager.getInstance() ;
            manager.clearLineData(line) ;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// JNI Declarations
////
    protected static native String JNI_createLine(String strRegisterUrl,
                                                  String strUser,
                                                  boolean bIsVisible,
                                                  boolean bAutoEnable,
                                                  int     iLineState,
                                                  boolean bUseCallHandling) ;

    protected static native boolean JNI_deleteLine(String strIdentity,
                                                   int unused) ;
    protected static native Object[][] JNI_getLines() ;
    protected static native String[] JNI_getDefaultLine() ;
    protected static native boolean JNI_setDefaultLine(String strIdentity,
                                                       int unused) ;
    protected static native String JNI_getLine(String strRemoteUrl, String strLocalContact);
    protected static native boolean JNI_enableLine(String  strIdentity,
                                                   boolean bEnable) ;
    protected static native int JNI_getLineState(String  strIdentity,
                                                 int unused) ;
    protected static native boolean JNI_enableAutoEnable(String  strIdentity,
                                                         boolean bEnable) ;
    protected static native boolean JNI_isAutoEnableEnabled(String strIdentity,
                                                            int unused) ;
    protected static native boolean JNI_enableShowLine(String strIdentity,
                                                       boolean bEnable) ;
    protected static native boolean JNI_isShowLineEnabled(String strIdentity,
                                                          int unused) ;
    protected static native boolean JNI_enableUseCallHandlingSettings(String  strIdentity,
                                                                      boolean bEnable) ;
    protected static native boolean JNI_isUseCallHandlingSettingsEnabled(String strIdentity,
                                                                         int unused) ;
    protected static native boolean JNI_setUser(String strIdentity,
                                                String strUser) ;
    protected static native String JNI_getUser(String strIdentity,
                                               int unused) ;

    protected static native void JNI_addLineListener(SipLineListener listener,
                                                     long lHashCode) ;
    protected static native void JNI_removeLineListener(SipLineListener listener,
                                                        long lHashCode) ;

    protected static native String[] JNI_getRealms(String strIdentity) ;
    protected static native boolean JNI_removeRealm(String strIdentity, String strRealm) ;
    protected static native boolean JNI_addRealm
           (String strIdentity, String realm, String userid, String password) ;

}
