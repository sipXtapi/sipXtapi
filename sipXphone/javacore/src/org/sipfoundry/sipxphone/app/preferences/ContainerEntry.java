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




package org.sipfoundry.sipxphone.app.preferences ; 

/**
 * Form entry encapsulates the information for a particular form element, 
 * for cases where an element is needed that contains more than one component.
 * This is needed as in the current implementation, containers cannot be added 
 * to containers: only components can. Hence, there is a system of form "entries"
 * that can contain more than one component.
 * 
 */
 
public interface ContainerEntry
{


   /**
    * Get the name of the Entry 
    */
    public String getName() ;
    
   /**
    * Get the value of the Entry 
    */
    public String getValue() ;
    
    
   /**
    * Has the entry changed?
    */ 
    public boolean hasChanged() ;
    
}
