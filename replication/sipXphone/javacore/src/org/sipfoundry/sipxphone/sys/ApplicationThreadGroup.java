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

package org.sipfoundry.sipxphone.sys ;

import java.lang.* ;
import org.sipfoundry.sipxphone.awt.form.* ;
import org.sipfoundry.sipxphone.sys.appclassloader.*;
/**
 * The application group thread exists so that we can easily enumerate 
 * applications and so we can easily catch thread deaths.  We do this
 * by overriding the uncaught exception and immediately telling the
 * application manager about it.
 */
public class ApplicationThreadGroup extends ThreadGroup
{
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** singleton instance of the thread group */
    protected static ApplicationThreadGroup m_instance ;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * protected constructor - this object is a singleton and you should
     * use getInstance();
     */ 
    protected ApplicationThreadGroup(String strName)
    {
        super(strName) ;
    }
       
    
    /**
     * get the one and only instance to the application group
     */
    public static ApplicationThreadGroup getInstance()
    {
        if (m_instance == null) {
            m_instance = new ApplicationThreadGroup("apps") ;            
        }           
        return m_instance ;
    }
 
    
    /**
     * This is called whenever an exception is thrown and not caught.  We are
     * looking for ThreadDeath, however, will tell the user about any other
     * uncaught exception (i.e. null pointer, etc) ;
     */
    public void uncaughtException(Thread t, Throwable e)
    {
        if (e instanceof ThreadDeath) {
            // ApplicationManager appManager = ApplicationManager.getInstance() ;
            // appManager.notifyThreadDeath(t) ;
        } else {
            Shell.getInstance().clearFlyOver() ;
            Shell.getInstance().clearHint() ;
            Shell.getInstance().clearStatus(0) ;
                        
            Shell.getInstance().showUnhandledException(e, false) ;
        }
    }
}
