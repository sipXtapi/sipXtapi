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
 * Data object for a Ringer hook.  A Ringer hook allows developers to 
 * substitute different visual and auditory alerts for an inbound call.
 * The default implementation plays the user's default ring file; however,
 * developers could easily add logic for features like distinctive ring.
 * <p>
 * If an action is taken, the hook should call the <i>terminate</i> method to 
 * avoid the possiblity of feature interaction.  Calling <i>terminate</i> prevents 
 * downstream hooks from acting on the hook data. Hooks are called in the order  
 * they were added. 
 *
 * @see org.sipfoundry.sipxphone.hook.Hook
 * @see org.sipfoundry.sipxphone.sys.HookManager
 * @see org.sipfoundry.sipxphone.hook.HookData
 *
 * @author Robert J. Andreasen, Jr.
 */
public class RingerHookData extends HookData
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    
    
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** address the hookers get to twiddle about */
    private PAddress m_address ;
    /** default ring file */
    private String m_strDefaultRingfile ;    
       
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Constructor; takes address and default ring sound file.
     *
     * @param address The address of inbound caller.
     * @param strDefaultRingfile The default ring sound file.
     */
    public RingerHookData(PAddress address, String strDefaultRingfile)
    {
        m_address = address ;
        m_strDefaultRingfile = strDefaultRingfile ;
    }
    
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    
    /**
     * What is the address of the inbound caller?
     */
    public PAddress getAddress()
    {
        return m_address ;   
    }
    
    
    /**
     * What is the default ring sound file?
     *
     */
    public String getDefaultRingfile()
    {
        return m_strDefaultRingfile ;   
    }        
}
