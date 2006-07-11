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

package org.sipfoundry.telephony.phone ;

import org.sipfoundry.telephony.* ;

import javax.telephony.* ;
import javax.telephony.phone.* ;
import javax.telephony.phone.capabilities.* ;

import org.sipfoundry.telephony.phone.capabilities.* ;


public class PtComponentGroup extends PtWrappedObject implements ComponentGroup
{
    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
    /** cached copy of our components */
    private Component[] m_components = null ;
    /** cached copy of our type */
    private int         m_iType = -1 ;
    /** cached description of the group */
    private String      m_strDescription = null ;
    
    
	public PtComponentGroup(long lHandle)
	{
		super(lHandle) ;		
	}	


	public int getType()
	{
	    if (m_iType == -1)
	        m_iType = JNI_getType(m_lHandle) ;   
	        
        return m_iType ;
	}
	
	
	public String getDescription()
	{
	    if (m_strDescription == null)
	        m_strDescription = JNI_getDescription(m_lHandle) ;	        
	    
		return m_strDescription ;
	}

		
	public Component[] getComponents()
	{
	    if (m_components == null) {		
		    long lHandles[] = JNI_getComponents(m_lHandle) ;
    		
		    m_components = new PtComponent[lHandles.length] ;
		    for (int i=0;i<lHandles.length; i++) {
			    m_components[i] = PtComponent.createComponent(lHandles[i]) ;
		    }
		}
		return m_components ;	   
	}
	
	
	public boolean activate()
	{
		return JNI_activate(m_lHandle) ;
	}
	
	
	public boolean deactivate()
	{
		return JNI_deactivate(m_lHandle) ;
	}
	
	public boolean activate(Address address)
		throws InvalidArgumentException
	{		 
		PtAddress addr = (PtAddress) address ;
				
		return JNI_activate_addr(m_lHandle, addr.getHandle()) ;
	}
		
	public boolean deactivate(Address address)
		throws InvalidArgumentException
	{
		PtAddress addr = (PtAddress) address ;
				
		return JNI_deactivate_addr(m_lHandle, addr.getHandle()) ;
	}
				   
	public ComponentGroupCapabilities getCapabilities()
	{
		return new PtComponentGroupCapabilities() ;
	}
		
	public void dump()
	{
		Component comps[] = getComponents() ;
		
		System.out.println("") ;
		System.out.println("   Group: " + this) ;
		System.out.println("	Desc: " + getDescription()) ;
		System.out.println("	Type: " + getDescription()) ;				 
		System.out.println(" # Comps: " + Integer.toString(comps.length)) ;
		
		for (int i=0; i<comps.length; i++) {
			System.out.println("   Comp" + i + ": " + comps[i].getName()) ;
		}						 
	}
	
    /**
     * This is called when the VM decides that the object is no longer
     * useful (no more references are found to it) and it is time to
     * delete it.  At this point, we call down to the native world and
     * free the native object
     */
    protected void finalize()
        throws Throwable    
    {
        JNI_finalize(m_lHandle) ;   
        m_lHandle = 0 ;
    }
		
	
	
//////////////////////////////////////////////////////////////////////////////
// Native Methods
////
	public static final native int	   JNI_getType(long lHandle) ;
	public static final native String  JNI_getDescription(long lHandle) ;
	public static final native long[]  JNI_getComponents(long m_lHandle) ;
	public static final native boolean JNI_activate(long m_lHandle) ;
	public static final native boolean JNI_deactivate(long m_lHandle) ;
	public static final native boolean JNI_activate_addr(long m_lHandle, long lAddress) ;
	public static final native boolean JNI_deactivate_addr(long m_lHandle, long lAddress) ;
    protected static native void       JNI_finalize(long lHandle) ;    
}
