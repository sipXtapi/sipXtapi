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

/**
 * 
 * The HookData object is passed to an installed hook by the system.  Please 
 * see the Hook interface and package level documentation for details on the
 * hook infrastructure.
 * <p>
 * A hook implementation typically extends the HookData class. See the 
 * documentation for the hook in question for more information.
 *
 * @see org.sipfoundry.sipxphone.hook.Hook
 * @see org.sipfoundry.sipxphone.sys.HookManager
 *
 * @author Robert J. Andreasen, Jr.
 */
public class HookData
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** Have we been terminated? */
    private boolean m_bTerminated ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    
    /**
     * Default Constructor.
     */
    public HookData()
    {
        m_bTerminated = false ;
        
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Tell the system that any registered hooks
     * of this type that are further down in the list should not be considered. 
     * <p>
     * If the hook did not take any action, it should not call <i>terminate</i>.
     * <p>
     * See the documentation associated with the hook that you are implementing for
     * more details.
     *
     * @see CallFilterHookData
     * @see CallerIDHookData
     * @see RingerHookData
     * @see MatchDialplanHookData
     * @see NewCallHookData
     */
    public void terminate()
    {
        m_bTerminated = true ;
    }
 
    
    /**
     * Query: Did the hook indicate that
     * other hooks of this type should be considered?
     *     
     * @return A flag indicating whether or not to terminate this hook.
     */
    public boolean isTerminated()
    {
        return m_bTerminated ;
    }    
}
