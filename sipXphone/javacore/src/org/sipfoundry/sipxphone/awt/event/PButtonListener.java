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
 * A Button listener is notified whenever a button is pressed down, release,
 * and when held down (repeat).
 *
 * Current, repeat is not implemented.
 *
 * @author Robert J. Andreasen, Jr. 
 */
public interface PButtonListener
{       
    /**
     * The specified button has been press downwards
     */    
    public void buttonDown(PButtonEvent event) ;


    /**
     * The specified button has been released
     */        
    public void buttonUp(PButtonEvent event) ;
    

    /**
     * The specified button is being held down
     */            
    public void buttonRepeat(PButtonEvent event) ;        
}