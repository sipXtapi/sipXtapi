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

 
package org.sipfoundry.sipxphone.awt.event ;


/**
 * A form listener is told about the varoius GUI state of a form: Initial
 * start up, about to be destroyed, and whenever focus is given or taken
 * away from it. 
 *
 * @author Robert J. Andreasen, Jr. 
 */
public interface PFormListener
{       
    /** 
     * invoked by the framework after the form has been created but before 
     * it is initally shown.
     */
    public void formOpening(PFormEvent event) ;
    
    
    /** 
     * invoked by the framework just before closing the frame.
     */
    public void formClosing(PFormEvent event) ;
    
    
    /**
     * invoked by the framework whenever an application becomes the 
     * foreground application and is given control of the soft buttons
     * and hard buttons.
     */
    public void focusGained(PFormEvent event) ;
    
    
    /**
     * invoked by the framework whenever an application loses focus and is
     * no longer the foreground application.
     */
    public void focusLost(PFormEvent event) ;        
}