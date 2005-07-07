/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/sip/SipLine.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */

package org.sipfoundry.sip;

import java.io.PrintStream ;
import org.sipfoundry.stapi.PSIPAddress ;
import java.net.MalformedURLException ;

public class SipLine
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final int LINE_STATE_UNKNOWN      = 0 ;
    public static final int LINE_STATE_REGISTERED   = 1 ;
    public static final int LINE_STATE_DISABLED     = 2 ;
    public static final int LINE_STATE_FAILED       = 3 ;
    public static final int LINE_STATE_PROVISIONED  = 4 ;
    public static final int LINE_STATE_TRYING       = 5 ;
    public static final int LINE_STATE_EXPIRED      = 6 ;

    protected static final String DATA_STATUS_CODE = "status_code" ;
    protected static final String DATA_STATUS_TEXT = "status_text" ;
    protected static final String DATA_AUTH_REALM  = "auth_realm" ;
    protected static final String DATA_AUTH_SCHEME = "auth_scheme" ;
    protected static final String DEVICE_LINE_USER = "Device" ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected String m_strIdentity ;    // Identity of this line.  This is used
                                        // as a handle/key for calls to the
                                        // lower layers.

    protected String m_strUserEnteredUrl; //User entered url

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Protected constructor that guards construction to members of this
     * package.  Developers should use the createLine on SipLineManager.
     *
     * @param strIdentity The identity of this line.
     */
    protected SipLine(String strIdentity, String strUserEnteredUrl)
    {
        m_strIdentity = strIdentity ;
        m_strUserEnteredUrl = strUserEnteredUrl;
        if (m_strUserEnteredUrl.endsWith("@"))
        {
            m_strUserEnteredUrl = m_strIdentity ;
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Get the state of this line.  See the various LINE_STATE constants
     * defined in this class.
     *
     * @return The state of this line.
     */
    public int getState()
    {
        return SipLineManager.JNI_getLineState(getIdentity(), 0) ;
    }


    /**
     * Get the identity of the line.
     *
     * @return identity of the line.
     */
    public String getIdentity()
    {
        return m_strIdentity ;
    }

    public String getUserEnteredUrl()
    {
        return m_strUserEnteredUrl ;
    }


    /**
     * Enable or disable the line.
     *
     * @param bEnable Specify <i>true</i> to enable the line or <i>false</i> to
     *        disable the line.
     */
    public void enable(boolean bEnable)
    {
        SipLineManager.JNI_enableLine(getIdentity(), bEnable) ;
    }


    /**
     * Get the user for the line.
     *
     * @return The user for the line.
     */
    public String getUser()
    {
        return SipLineManager.JNI_getUser(getIdentity(), 0) ;
    }

    /**
     * get Credentials associated with this line.
     */
     /*
    public SipCredentials getCredentials(){
        SipCredentials credentials = null;
        String[] strCredentialsArray =  SipLineManager.JNI_getCredentials(getIdentity());
        if( ( strCredentialsArray != null ) &&
            ( strCredentialsArray.length == 4 ) ){
            credentials = new SipCredentials( strCredentialsArray[0],
                                            strCredentialsArray[1],
                                            strCredentialsArray[2],
                                            strCredentialsArray[3] );
        }
        return credentials;
    }
    */

    /**
     * gets the realms of this line.
     */
    public String[] getRealms(){
        return SipLineManager.JNI_getRealms(getIdentity());
    }

    /**
     * removes the realm from this line.
     */
    public boolean removeRealm(String realm){
            return SipLineManager.JNI_removeRealm(getIdentity(), realm);
    }

    /**
     * This will add a realm with new username and password
     */
    public boolean addRealm(String realm, String userid, String password){
            return SipLineManager.JNI_addRealm(getIdentity(), realm, userid, password);
    }

    /**
     * Is this auto enable set for the line.
     *
     * @return <i>true</i> if auto enable is set, otherwise <i>false</i>.
     *
     */
    public boolean isAutoEnableEnabled()
    {
        return SipLineManager.JNI_isAutoEnableEnabled(getIdentity(), 0) ;
    }


    /**
     * Sets the auto enable attribute for the line.  Auto enable is used to
     * control whether a line automatically enables on phone restarts.  If a
     * line is disabled, it will not be enabled until the next restart or the
     * line is enabled manually.
     *
     * @param bEnable Set to <i>true</i> to enable the auto enable feature or
     *        <i>false</i> to disable it.
     */
    public void enableAutoEnable(boolean bEnable)
    {
        SipLineManager.JNI_enableAutoEnable(getIdentity(), bEnable) ;
    }


    /**
     * Is the line configured to use call forwarding settings?
     *
     * @return <i>true</i> if use forwarding settings are enabled, otherwise
     *         <i>false</i>.
     */
    public boolean isAllowForwardingEnabled()
    {
        return SipLineManager.JNI_isUseCallHandlingSettingsEnabled(getIdentity(), 0) ;
    }


    /**
     * Sets the use forwarding settings for the line.  If enabled, all of the
     * call forwarding settings will be applied to this line.
     *
     * @param bEnable Set to <i>true</i> to enable the forwarding features for the line
     *        or <i>false</i> to disable it.
     */
    public void enableAllowForwarding(boolean bEnable)
    {
        SipLineManager.JNI_enableUseCallHandlingSettings(getIdentity(), bEnable) ;
    }


    /**
     * Is show line enabled for the line.
     *
     * @return <i>true</i> if show line is enabled, otherwise <i>false</i>.
     */
    public boolean isShowLineEnabled()
    {
        return SipLineManager.JNI_isShowLineEnabled(getIdentity(), 0) ;
    }


    /**
     * Sets the show line attribute for the line.  Show line controls whether
     * the line is displayed in the list of outbound lines.
     *
     * @param bEnable Set to <i>true</i> to display the line as an outbound
     *        line pick or <i>false</i> to hide it.
     */
    public void enableShowLine(boolean bEnable)
    {
        SipLineManager.JNI_enableShowLine(getIdentity(), bEnable) ;
    }


    /**
     * @deprecated do not expose
     */
    public String toString()
    {
        return m_strIdentity ;
    }


    /**
     * Returns the protocol failure code if available.  If a line has not
     * failed or if the error is unavailable, this method will return a value
     * of 0.
     *
     * @return the underlying protocol status code or 0 if not applicable/
     *         available.
     */
    public int getFailedStatusCode()
    {
        int iRC = 0 ;

        Integer intStatusCode = (Integer)
                SipLineManager.getInstance().getLineData(this,  DATA_STATUS_CODE) ;

        if (intStatusCode != null)
        {
            iRC = intStatusCode.intValue() ;
        }

        return iRC ;
    }


    /**
     * Returns the protocol failure text if available.  If a line has not
     * failed or if the error is unavailable, this method will return a null
     * value.
     *
     * @return the underlying protocol status text or null if not applicable/
     *         available.
     */
    public String getFailedStatusText()
    {
        String strRC = null ;

        strRC = (String)
                SipLineManager.getInstance().getLineData(this, DATA_STATUS_TEXT) ;

        return strRC ;
    }


    /**
     * Returns the authentication failure realm if available.  If a line has
     * not failed or if the infomration is unavailable, this method will return
     * a value of null.
     *
     * @return the underlying authentication failure realm or null if not
     *         applicable/available.
     */
    public String getFailedAuthRealm()
    {
        String strRC = null ;

        strRC = (String)
                SipLineManager.getInstance().getLineData(this, DATA_AUTH_REALM) ;

        return strRC ;
    }


    /**
     * Returns the authentication failure scheme if available.  If a line has
     * not failed or if the infomration is unavailable, this method will return
     * a value of null.
     *
     * @return the underlying authentication failure scheme or null if not
     *         applicable/available.
     */
    public String getFailedAuthScheme()
    {
        String strRC = null ;

        strRC = (String)
                SipLineManager.getInstance().getLineData(this, DATA_AUTH_SCHEME) ;

        return strRC ;
    }


    /**
     * Determines if this line is a device line or not.
     *
     * @return true if the line is a device line otherwise false.
     */
    public boolean isDeviceLine()
    {
        boolean bDeviceLine = false ;
        String strUser = getUser() ;

        if (strUser != null)
        {
            if (strUser.equalsIgnoreCase(DEVICE_LINE_USER))
            {
                bDeviceLine = true ;
            }
        }

        return bDeviceLine ;
    }



    /**
     * @deprecated do not expose
     */
    public void dump(PrintStream out)
    {
        out.println("**Dumping SipLine**") ;
        out.println("       Identity: " + getIdentity());
        out.println("          State: " + getStateAsString(getState())) ;
        out.println("           User: " + getUser());
        out.println("     AutoEnable: " + isAutoEnableEnabled());
        out.println("     Forwarding: " + isAllowForwardingEnabled());
        out.println("      Show Line: " + isShowLineEnabled());
        out.println("   Failure code: " + getFailedStatusCode()) ;
        out.println("     Auth Realm: " + getFailedAuthRealm()) ;
        out.println("    Auth Scheme: " + getFailedAuthScheme()) ;
    }


    /**
     * @deprecated do not expose
     */
    public String getStateAsString(int iState)
    {
        String strState = "ERROR/UNKNOWN" ;

        switch (iState)
        {
            case SipLine.LINE_STATE_EXPIRED:
                strState = "Expired" ;
                break ;
            case SipLine.LINE_STATE_FAILED:
                strState = "Failed" ;
                break ;
            case SipLine.LINE_STATE_DISABLED:
                strState = "Disabled" ;
                break ;
            case SipLine.LINE_STATE_PROVISIONED:
                strState = "Provisioned" ;
                break ;
            case SipLine.LINE_STATE_REGISTERED:
                strState = "Registered" ;
                break ;
            case SipLine.LINE_STATE_TRYING:
                strState = "Trying" ;
                break ;
            case SipLine.LINE_STATE_UNKNOWN:
                strState = "Unknown" ;
                break ;
        }

        return strState ;
    }

    /**
     * @deprecated do not expose
     */
    public boolean equals(Object obj)
    {
        boolean bRC = false ;

        if ((obj != null) && (obj instanceof SipLine))
        {
            bRC = getIdentity().equals(((SipLine) obj).getIdentity()) ;
        }

        return bRC ;
    }


    /**
     * @deprecated do not expose
     */
    public int hashCode()
    {
        return getIdentity().hashCode() ;
    }


//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////


//////////////////////////////////////////////////////////////////////////////
// JNI Methods
////

}
