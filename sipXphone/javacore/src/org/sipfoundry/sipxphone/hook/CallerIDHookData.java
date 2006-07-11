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

package org.sipfoundry.sipxphone.hook ;

import org.sipfoundry.stapi.* ;

/**
 * Data object for a CallerID hook.  A CallerID hook allows a third party 
 * developer to supplement and/or replace the default caller identification.  
 * After a CallerID hook is added from the Hook Manager, it will be 
 * executed in the order that it was installed.  Call <i>setIdentity</i> to 
 * identify the call, or do nothing and allow other CallerID hooks to 
 * determine the identity.
 *
 * @see org.sipfoundry.sipxphone.sys.HookManager
 *
 * @author Robert J. Andreasen, Jr.
 */
public class CallerIDHookData extends HookData
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** final identity that we will display */
    private String   m_strIdentity ;
    /** address the hookers get to twiddle about */
    private PAddress m_address ;
    
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructor; takes an address and default identity string.
     *
     * @param address Address of caller/callee that we are trying to identify.
     * @param strDefaultIdentity Best guess as to the identity before any hooks
     *        get an opportunity to change it.
     *
     */
    public CallerIDHookData(PAddress address, String strDefaultIdentity)
    {
        m_address = address ;
        m_strIdentity = strDefaultIdentity ;
    }
    
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Set the identity of the user as displayed in the core user interface
     * forms.
     * <p>
     * NOTE: <i>setIdentity</i> causes an implicit <i>terminate</i>.
     */
    public void setIdentity(String strIdentity)
    {
        m_strIdentity = strIdentity ;                
        terminate() ;
    }
    
    
    /**
     * Get caller ID information. The {@link PAddress} is used to place calls and 
     * receive incoming calls. Use {@link PSIPAddress} to parse this information.
     */
    public PAddress getAddress()
    {
        return m_address ;   
    }
    

    /** 
     * Get the identity of this call.  This information defaults to the 
     * default identity specified when the hook data object was created.     
     */    
    public String getIdentity()
    {
        return m_strIdentity ;   
    }
}
