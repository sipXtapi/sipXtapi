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

import java.util.* ;
import java.io.* ;


/**
 * Basic event class.
 *
 * @author Robert J. Andreasen Jr.
 */ 
public class PEvent extends EventObject
{                
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    /** Event Type: TYPE_UNKNOWN */
    public static final int TYPE_UNKNOWN     = -1 ;
    public static final int TYPE_ITEM_EVENT  = 1 ;
    public static final int TYPE_TEXT_EVENT  = 2 ;
    public static final int TYPE_FOCUS_EVENT = 3 ;
      
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    /** Category as set by the EventBroadcaster. */
    protected Object m_objCategory ;
    
    /** event id type */
    protected int    m_iType ;    
    
    /** user provided integer param */
    protected int    m_iParam ;
    
    /** user provided object param */
    protected Object m_objParam ;
    
    /** has this event been consumed or not? */
    protected boolean m_bConsumed ;


//////////////////////////////////////////////////////////////////////////////
// Constructors
////    
    /** 
     * Creates a PEvent with the specified source object.  The type is 
     * set to TYPE_UNKNOWN and all optional params are initialized to either
     * null or zero depending on their data type.
     *
     * @param objSource Source object that is firing the event.
     */
    public PEvent(Object objSource)
    {
        super(objSource) ;
                        
        m_iParam = 0 ; 
        m_objParam = null ;
        m_bConsumed = false ;        
        m_iType = TYPE_UNKNOWN ;
    }
    
    
    /**   
     * Creates a PEvent with the specified source object and specified type
     * identifier.  All optional params are initialized to either null or zero
     * depending on their data type.
     *
     * @param objSource Source object that is firing the event.
     * @param iType TYPE_* constant representing this PEvent
     */
    public PEvent(Object objSource, int iType)
    {
        super(objSource) ;
               
        m_iParam = 0 ;
        m_objParam = null ;
        m_bConsumed = false ;        
        m_iType = iType ;
    }

    
//////////////////////////////////////////////////////////////////////////////
// public methods
////    
        
    /**
     * Set event-specific type identifer accessor method.
     * 
     * @param iID event specific identifer
     */
    public void setType(int iType)
    {
        m_iType = iType ;   
    }
    
    
    /**
     * Get event-specific type identifer accessor method.
     * 
     * @return event specific identifer
     */
    public int getType()
    {
        return m_iType ;   
    }

    
    /**
     * Set object param accessor method. 
     */
    public void setObjectParam(Object objParam)
    {
        m_objParam = objParam ;
    }
    

    /**
     * Get object param accessor method.     
     */
    public Object getObjectParam()
    {
        return m_objParam ;
    }
    
    
    /**
     * Set int param accessor method.     
     */
    public void setIntParam(int iParam)
    {
        m_iParam = iParam ;        
    }
    
    
    /**
     * Get int param accessor method.     
     */
    public int getIntParam()
    {
        return m_iParam ;   
    }
    
    
    /**
     * Consume the event so no one else sees it. <burp>
     */
    public void consume()
    {
        m_bConsumed = true ;   
    }
    
    
    /**
     * Has this event been consumed?
     */
    public boolean isConsumed()
    {
        return m_bConsumed ;
    }
      
    
    /** 
     * @deprecated do not expose
     *
     * Get the string-based representation of this object.
     *
     * @return string based representation of this event
     */
    public String toString()
    {
        return ("PEvent " + 
                    "id=" + Integer.toString(getType()) + " " + 
                    "intParam=" + Integer.toString(getIntParam()) + " " + 
                    "objParam=" + getObjectParam()) ;
    }     
}