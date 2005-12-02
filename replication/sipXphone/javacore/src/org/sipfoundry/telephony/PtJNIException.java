/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/telephony/PtJNIException.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */


package org.sipfoundry.telephony ;


/**
 * This Exception is thrown from JNI methods in the org.sipfoundry.telephony 
 * pacakge and contains the native PTAPI/PtStatus error code along with
 * an optional object parameter.
 * 
 * @author Robert J. Andreasen, Jr.
 */
public class PtJNIException extends Exception
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////    
    public static final int PT_SUCCESS                      = 0 ;
    public static final int PT_AUTH_FAILED                  = 1 ;
    public static final int PT_FAILED                       = 2 ;
    public static final int PT_EXISTS                       = 3 ;
    public static final int PT_HOST_NOT_FOUND               = 4 ;
    public static final int PT_IN_PROGRESS                  = 5 ;
    public static final int PT_INVALID_ARGUMENT             = 6 ;
    public static final int PT_INVALID_PARTY                = 7 ;
    public static final int PT_INVALID_STATE                = 8 ;
    public static final int PT_INVALID_IP_ADDRESS           = 9 ;
    public static final int PT_INVALID_SIP_DIRECTORY_SERVER = 10 ;
    public static final int PT_INVALID_SIP_URL              = 11 ;
    public static final int PT_MORE_DATA                    = 12 ;
    public static final int PT_NO_MORE_DATA                 = 13 ;
    public static final int PT_NOT_FOUND                    = 14 ;
    public static final int PT_PROVIDER_UNAVAILABLE         = 15 ;
    public static final int PT_RESOURCE_UNAVAILABLE         = 16 ;
    public static final int PT_BUSY                         = 17 ;
    
    
//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private int      m_iStatus ;
    private Object   m_objParam ;
    
    
//////////////////////////////////////////////////////////////////////////////
// Construction
////
    /**
     * Guard against nno argument construction
     */
    private PtJNIException()  { }
    
            
    /**
     * Constructor requiring a PtStatus code
     *
     * @param iStatus PtStatus code indicating the failure
     *
     * @signture (I)V
     */
    public PtJNIException(int iStatus) 
    {
        super("PtJNIException status=" + iStatus) ;
        
        m_iStatus = iStatus ;
        m_objParam = null ;
    }           
    
    
//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Set the exception specific object parameter
     */
    public void setObjectParam(Object obj) 
    {
        m_objParam = obj ;        
    }
    
    
    /**
     * Get the exception specific object parameter
     */
    public Object getObjectParam()
    {
        return m_objParam ;        
    }


    /**
     * Get the PtStatus code of this exception
     */
    public int getStatus()
    {
        return m_iStatus ;    
    }
    
      
//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////    


//////////////////////////////////////////////////////////////////////////////
// Nested / Inner Classes
////    

}
