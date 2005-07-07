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

 
package org.sipfoundry.sipxphone.sys.app.core ; 

import java.awt.* ; 
import java.util.* ;

import org.sipfoundry.sipxphone.* ;
import org.sipfoundry.sipxphone.awt.* ;
import org.sipfoundry.sipxphone.awt.event.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.util.* ;

/**
 * The TaskContextMenu is the context/application specific help text.  This
 * is displayed as the content of one of the task manager's tab.
 *
 * @author Robert J. Andreasen. Jr.
 */
public class TaskContextHelp extends PDisplayArea
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** reference to the parent form */
    private PForm m_formParent ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Create the tab form given the specified parent frame
     */        
    public TaskContextHelp(PForm formParent)
    {
        m_formParent = formParent ;
    }
           
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * Populate the menu given the specified 'active form'
     */         
    public void populateHelp(PForm activeForm)
    {                
        if (activeForm != null) {
            setText(activeForm.getHelpText()) ;            
        } else {
            setText("Error: No Form is active!") ;
        }
    }
    
    
    /**
     * This method is invoked from the TaskForm whenever the Task form loses
     * focus.  We clear our data to help promote garbage collection and reduce
     * the amount of noise present when trying to track down resource leaks.
     */
    public void cleanup()
    {     
        setText("Error: No Form is active!") ;
    }
    
                
//////////////////////////////////////////////////////////////////////////////
// Inner / Nested Classes
////

}